// TEAM SWINBURNE - TS_20
// PRECHARGE CONTROLLER
// MICHAEL COCHRANE & PATRICK CURTAIN
// REVISION 0 (29/02/2020)

// TEST MODES
// 0 Normal	
// 1 Test Relays
// 2 Test Max Voltage 592
// 3 Test Max Voltage 250
// 4 Print Current Status
// 5 Print Raw ADC Value
#define TEST_MODE 0

/* 
To correct limited 64k flash issue: https://github.com/platformio/platform-ststm32/issues/195. Ensure device is 128k model. 
Use the following platformIO initialisation:
	[env:genericSTM32F103C8]
	platform = ststm32@4.6.0
	board = genericSTM32F103C8
	framework = mbed
	board_upload.maximum_size = 120000

//-----------------------------------------------

The precharge controller is a PCB designed by Team Swinburne to control the precharge sequences of 
a Formula SAE vehicle, mointor the health of the accumulator based on various inputs and react accordingly. This PCB 
integrates the precharge resistor array, precharge relay, over-temperature safety control (PDOc), safety interlock for the AMS, 
an interface for the isolation monitoring device (IMD), and the ability to detect the voltage of the battery and motor
controller.

//-----------------------------------------------

Based on STM32F103C8 "Blue Pill".

PA_0/ADC0/CTS2/T2C1E/WKUP (PWM) (3.3V)
PA_1/ADC/RT52/T2C2 (PWM) (3.3V)
PA_2/ADC2/TX2/T2C3 (PWM) (3.3V)
PA_3/ADC3/RX2/T2C4 (PWM) (3.3V)
PA_4/ADC4/NSS1/CK2 (3.3V)
PA_5/ADC5/SCK1 (3.3V)
PA_6/ADC6/MISO1/T3C1/T1BKIN (PWM) (3.3V)
PA_7/ADC7/MOSI1/T3C2/T1C1N (PWM) (3.3V)

PA_8/CK1/T1C2/MCO (PWM)
PA_9/TX1/T1C2 (PWM)
PA_10/RX1/T1C3 (PWM)
PA_11/USB-/CTS1/T1C4/CANRX
PA_12/USB+/RTS1/T1ETR/CANTX
PA_13/JTMS/SWDIO
PA_14/JTMS/SWCLK
PA_15/JTD1/NSS/T2C1E

PB_0/ADC8/T3C3/T1C2N (PWM) (3.3V)
PB_1/ADC9/T3C4/T1C3N (PWM) (3.3V)
PB_2/BOOT1
PB_3/JTD0/SCK1/T2C2
PB_4/JTRST/MISO/T3C1
PB_5/SMBAI1/MOSI1/T2C2 (3.3V)

PB_8/T4C3/SCL1/CANRX (PWM)
PB_9/T4C4/SDA1/CANTX
PB_10/SCL2/TX3/T2C3N
PB_11/SDA2/RX3/T2C3N
PB_12/NSS2/T1BKIN/CK3
PB_13/SCK2/T1C1N/CTS3
PB_14/MISO2/T1C2N/RTS3
PB_15/MOSI2/T1C3N

PC_13/TAMPER/LED1 (3.3V)*
PC_14/OSC32IN (3.3V)*
PC_15/OSC32OUT (3.3V)*

*/

//-----------------------------------------------
// Initialisation
//-----------------------------------------------

#include <mbed.h>
#include <CAN.h>
#include "Adafruit_ADS1015.h"

//-----------------------------------------------
// Interfaces
//-----------------------------------------------

// UART Interface
Serial pc(PA_2, PA_3);                 		//TX, RX

// I2C Interface
I2C i2c1(PB_7, PB_6);     					//SDA, SCL

// I2C Addressess
// ADS1115 ADDR PINS: GND 48 - VDD 49 - SDA 4A - SCL 4B
#define PDOC_ADC_ADDR								0x4B
#define MC_HV_SENSE_ADC_ADDR						0x49
#define BATT_HV_SENSE_ADC_ADDR						0x48

// ADC Interfaces
Adafruit_ADS1115 pdoc_adc(&i2c1, PDOC_ADC_ADDR);
Adafruit_ADS1115 mc_hv_sense_adc(&i2c1, MC_HV_SENSE_ADC_ADDR);
Adafruit_ADS1115 batt_hv_sense_adc(&i2c1, BATT_HV_SENSE_ADC_ADDR);

// CANBUS Interface
CAN can1(PB_8, PB_9);     						// RXD, TXD

// CANBUS Frequency
#define CANBUS_FREQUENCY							500000

// CANBUS Message Format
CANMessage can1_msg;

// CANBUS Addresses
#define PRECHARGE_CONTROLLER_HEARTBEAT_ID 			0x440
#define PRECHARGE_CONTROLLER_ERROR_ID				0x441
#define PRECHARGE_CONTROLLER_ANALOGUE_ID			0x442
#define PRECHARGE_CONTROLLER_PERIPHERAL_ID			0x443

#define DISCHARGE_MODULE_HEARTBEAT_ID			 	0x450
#define DISCHARGE_MODULE_ERROR_ID					0x451
#define DISCHARGE_MODULE_ANALOGUE_ID				0x452
#define DISCHARGE_MODULE_PERIPHERAL_ID				0x453

// Throttle Controller
#define THROTTLE_CONTROLLER_HEARTBEAT_ID			0x340
#define THROTTLE_CONTROLLER_ERROR_ID				0x341
#define THROTTLE_CONTROLLER_ANALOGUE_ID				0x342
#define THROTTLE_CONTROLLER_PERIPERAL_ID			0x343

// Brake Module
// Orion BMS 2
#define ORION_BMS_STATUS_ID							0x180
#define ORION_BMS_VOLTAGE_ID						0x181
#define ORION_BMS_TEMPERATURE_ID					0x182

#define ORION_BMS_RINEHART_LIMITS					0x202
// Orion TEM
// Dash
// Motor Controllers

#define TC_CHARGER_STATUS_ID						0x18FF50E5

//-----------------------------------------------
// Calibration Factors
//-----------------------------------------------

// HV Voltage Bridge Offset Resistors
const int MC_R_CAL = 5000;
const int BATT_R_CAL = 5000;

// Voltage Limits
const int MINIMUM_CELL_VOLTAGE = 27;
const int MAXIMUM_CELL_VOLTAGE = 43;
const int MAXIMUM_CELL_TEMPERATURE = 65;

//-----------------------------------------------
// Classes
//-----------------------------------------------

class PCB_Temp_Sensor {
public:
	PCB_Temp_Sensor(PinName pin) : _sensor(pin){
		_temperature = _sensor.read();
	}

	int read(){
		return resistanceToTemperature(voltageToResistance(3.3*_sensor.read()));
	}

private:
	const float r1 = 10000;
	const float vin = 5;

	const float BETA = 3430;
	const float R2 = 10000;
	const float T2 = 25 + 270;

	float _resistance;
	int _temperature;
	AnalogIn _sensor;

	float voltageToResistance(float vout){
		return r1/((vin/vout)-1);
	}

	int resistanceToTemperature(float R1){
		return ((BETA * T2)/(T2 * log(R1/R2) + BETA))-270;
	}
};

class IMD_Data {
public: 
	IMD_Data(PinName imd_data_interrupt_pin) : imd_data_interrupt(imd_data_interrupt_pin) {}

	void start(){
		imd_data_interrupt.rise(callback(this, &IMD_Data::set_period));
		imd_data_interrupt.fall(callback(this, &IMD_Data::set_duty_cycle));
		imd_timer.start();
	}

	int get_period(){return period;}
	int get_frequency(){return frequency;}
	int get_duty_cycle(){return duty_cycle;}

private:
	InterruptIn imd_data_interrupt;

	float active_end;
	float duty_cycle;
	float period;

	int	frequency;

	Timer imd_timer;

	void set_period(){
		period = imd_timer.read_us();
		frequency = 1000000.0f/period;
        imd_timer.reset();
	}

	void set_duty_cycle(){
		active_end = imd_timer.read_us();
		duty_cycle = (active_end/period)*100.0;
	}
};


//-----------------------------------------------
// Globals
//-----------------------------------------------

static int8_t  	heartbeat_state 			= 0;
static int	    heartbeat_counter 			= 0;

static int8_t	error_code					= 0xFF;
static int8_t  	warning_code				= 0xFF;

static bool		precharge_button_state		= 0;
static bool 	charge_mode_activated		= 0; 
static int 		discharge_state				= 2;	// (2 - Default)
static bool 	orion_connected	 			= 0;	// (0 - Default)
static bool		orion_safety_ok				= 0xFF;

static int 		precharge_start_time 		= 0;
static int 		orion_last_connection		= 0;

static int 		orion_low_voltage			= 0;
static int 		orion_high_voltage			= 0;
static int 		orion_high_temperature		= 0;

static int16_t  pdoc_temperature			= 0;
static int16_t  pdoc_ref_temperature		= 0;
static int16_t  mc_voltage					= 0;
static int16_t  battery_voltage 			= 0;

// Program Interrupt Timer Instances
Ticker ticker_heartbeat;
Ticker ticker_can_transmit;
// Ticker ticker_orionwd;

// Interval & Periods
#define	HEARTRATE			 				1
#define CAN_BROADCAST_INTERVAL              0.2
#define ORION_TIMEOUT_INTERVAL				0.25

//-----------------------------------------------
// GPIO 
//-----------------------------------------------

DigitalOut led1(PC_13);
DigitalOut can1_rx_led(PB_1);
DigitalOut can1_tx_led(PB_0);
PCB_Temp_Sensor pcb_temperature(PA_0);

DigitalOut AIR_neg_relay(PA_8);
DigitalOut precharge_relay(PA_9); 
DigitalOut AIR_pos_relay(PA_10);
DigitalOut AMS_ok(PB_13);

IMD_Data  IMD_interface(PA_15);
DigitalIn AIR_power(PB_3);
DigitalIn AIR_neg_feedback(PB_10);
DigitalIn AIR_pos_feedback(PB_11);
DigitalIn IMD_ok(PB_12);
DigitalIn PDOC_ok(PB_15);

//-----------------------------------------------
// Functions
//-----------------------------------------------

/* 
ORION WATCHDOG CALLBACK
	Callback function used to check whether the orion is functioning correctly.
*/
void orionwd_cb(){
	if (heartbeat_counter - orion_last_connection > 3){
		orion_connected = false;
	}
}

	/*
HEARTBEAT
	The heartbeat is used to relay the state of the device and time since device epoch to the CANBUS, allowing 
	for the logging of faulure events. 
	*/
void heartbeat(){
	heartbeat_counter++;
	led1 = !led1;
	char TX_data[3] = {(char)heartbeat_state, (char)heartbeat_counter, (char)pcb_temperature.read()};
	if(can1.write(CANMessage(PRECHARGE_CONTROLLER_HEARTBEAT_ID, &TX_data[0], 3))) {
		can1_tx_led = !can1_tx_led;
       	// pc.printf("Heartbeat Success! State: %d Counter: %d\r\n", heartbeat_state, heartbeat_counter);
    } else {
		// pc.printf("Hearts dead :(\r\n");
	}

	orionwd_cb();
}

	/*
CAN RECEIVE
	Message box for CANBUS.
	*/
void CAN1_receive(){
	can1_rx_led = !can1_rx_led;

	if (can1.read(can1_msg)){
		switch(can1_msg.id){
			case DISCHARGE_MODULE_HEARTBEAT_ID:
				discharge_state = can1_msg.data[0];
				break;

			case THROTTLE_CONTROLLER_PERIPERAL_ID:
				precharge_button_state = can1_msg.data[0];
				break;

			case ORION_BMS_STATUS_ID:
				// Big endian & MSB
				// 0b000000, 0000001	Discharge Relay Enabled
				// 0b000000, 0000010	Charge Relay Enabled
				// 0b000000, 0000100	Charge Safety Enabled
				// Use bitwise operator to mask out all except relevent statuses.
				orion_connected = true;
				orion_last_connection = heartbeat_counter;

				if ((can1_msg.data[0] & 0b00000111) > 0){
					orion_safety_ok = true;	
				} else {
					orion_safety_ok = false;
				}
				break;
			
			case ORION_BMS_VOLTAGE_ID:
				orion_low_voltage = can1_msg.data[0];
				orion_high_voltage = can1_msg.data[2];
				break;

			case ORION_BMS_TEMPERATURE_ID:
				orion_high_temperature = can1_msg.data[2];
				break;
			
			//Need to add temperature check as well, but wait until that is sorted!

			case TC_CHARGER_STATUS_ID:
				charge_mode_activated = 1;
				printf("Entering charge mode. Cycle power to exit charge mode\r\n");
				break;
		}
	}
}

/* 
CAN TRANSMIT
	Outgoing messages sent periodically (CAN_BROADCAST_INTERVAL).
*/
void CAN1_transmit(){
    char TX_data[8] = {0};

	TX_data[0] = 0;
	TX_data[1] = error_code;
	TX_data[2] = 0;
	TX_data[3] = warning_code; // add stuff for imd period and frequency.
	
	if (can1.write(CANMessage(PRECHARGE_CONTROLLER_ERROR_ID, &TX_data[0], 4))) {
    	can1_tx_led = !can1_tx_led;
    } else {
	}

	TX_data[0] = (char)(pdoc_temperature >> 8);
	TX_data[1] = (char)(pdoc_temperature & 0xFF);
	TX_data[2] = (char)(pdoc_ref_temperature >> 8);
	TX_data[3] = (char)(pdoc_ref_temperature & 0xFF);
	TX_data[4] = (char)(mc_voltage*10 >> 8);
	TX_data[5] = (char)(mc_voltage*10 & 0xFF);
	TX_data[6] = (char)(battery_voltage*10 >> 8);
	TX_data[7] = (char)(battery_voltage*10 & 0xFF);

	if(can1.write(CANMessage(PRECHARGE_CONTROLLER_ANALOGUE_ID, &TX_data[0], 8))) {
       can1_tx_led = !can1_tx_led;
	   // pc.printf("MESSAGE SUCCESS!\r\n");
    } else {
		// printf("MESSAGE FAIL!\r\n");
	}

	TX_data[0] = precharge_relay;
	TX_data[1] = IMD_interface.get_frequency();
	TX_data[2] = IMD_interface.get_duty_cycle();
	TX_data[3] = AIR_power;
	TX_data[4] = AIR_neg_feedback;
	TX_data[5] = AIR_pos_feedback;

	if (can1.write(CANMessage(PRECHARGE_CONTROLLER_PERIPHERAL_ID, &TX_data[0], 6))) {
       can1_tx_led = !can1_tx_led;
	   // pc.printf("MESSAGE SUCCESS!\r\n");
    } else {
		// printf("MESSAGE FAIL!\r\n");
	}
}

//-----------------------------------------------
// Daemons
//-----------------------------------------------

bool errord(){
	int _error_code = 0;
	if (IMD_ok == 0){
		_error_code = _error_code + 0b00000001;
		// pc.printf("FAULT: Isolation fault detected, please check wiring!\r\n");
	}
	if (PDOC_ok == 0){
		_error_code = _error_code + 0b00000010;
		// pc.printf("FAULT: PDOC failure detected, please allow to cool and check for\
		short circuit!\r\n");
	}
	if (orion_connected == false){
		_error_code = _error_code + 0b00000100;
		// pc.printf("FAULT: Orion BMS not attached, please check CAN is functioning, and\
		// Orion is attached!\r\n");
	}
	if (orion_safety_ok == false){
		_error_code = _error_code + 0b00001000;
	}

	// if (orion_low_voltage < MINIMUM_CELL_VOLTAGE){
	// 	_error_code = _error_code + 0b00001000;
	// 	// pc.printf("FAULT: Orion reports undervoltage fault!\r\n");
	// }
	// if (orion_high_voltage > MAXIMUM_CELL_VOLTAGE){
	// 	_error_code = _error_code + 0b00010000;
	// 	// pc.printf("FAULT: Orion reports overvoltage fault!\r\n");
	// }
	// if (orion_high_temperature > MAXIMUM_CELL_TEMPERATURE){
	// 	_error_code = _error_code + 0b00100000;
	// 	// pc.printf("FAULT: Orion reports overtemperature fault!\r\n");
	// }
	
	if (_error_code > 1){
		heartbeat_state = 0;
		AMS_ok = 0;
	} else {
		AMS_ok = 1;
	}
	error_code = _error_code;
	return _error_code; 
}

int warnd(){
	int _warning_code = 0;
	if (pcb_temperature.read() > 60){
		_warning_code = _warning_code + 0b00000001;
		// PCB Overtemperature
		// pc.printf("PCB too hot, you should probably check that, but don't take my word\
		for it, i'm just a hot MCU looking to have some fun! ;) ");
	}
	if (discharge_state > 2 && heartbeat_state != 0){
		_warning_code = _warning_code + 0b00000010;
		// Discharge/Precharge Mismatch
		// pc.printf("Discharge reported active during drive, please check wiring to discharge.\r\n");
	}
	if (AIR_neg_feedback != AIR_neg_relay){
		_warning_code = _warning_code + 0b0000100;
		// Negative AIR Mismatch
		// pc.printf("Negative AIR mismatch, check for welding or wiring failure\r\n");
	}
	if (AIR_pos_feedback != AIR_pos_relay){
		_warning_code = _warning_code + 0b00001000;
		// Positive AIR Mismatch
		// pc.printf("Positive AIR mismatch, check for welding or wiring failure\r\n");
	}
	warning_code = _warning_code;
	return _warning_code;
}

void stated(){
	switch(heartbeat_state){
		case 0:	// Fail State (Default){
			AIR_neg_relay = 0;
			precharge_relay = 0;
			AIR_pos_relay = 0;
			AMS_ok = 0;

			if (error_code == 0)
				heartbeat_state = 1;
			break;

		case 1: // Idle State
			AIR_neg_relay = 0;
			precharge_relay = 0;
			AIR_pos_relay = 0;

			if (precharge_button_state || charge_mode_activated){
				// pc.printf("Beginging precharge sequence\r\n");
				precharge_start_time = heartbeat_counter;
				heartbeat_state = 2;
				wait(0.5);	// Safeguard for discharge relay
			}
			break;

		case 2: // Precharging State
			AIR_neg_relay = 1;
			precharge_relay = 1;
			AIR_pos_relay = 0;

			if (!AIR_power){
				heartbeat_state = 1;
			}

			if (mc_voltage > battery_voltage*0.95){
				AIR_neg_relay = 1;
				precharge_relay = 1;
				AIR_pos_relay = 1;

				// pc.printf("Precharge within 95%, safe to close postive contactor\r\n");
				wait(0.5);
				heartbeat_state = 3;
			}

			if (heartbeat_counter > precharge_start_time + 5){
				// pc.printf("Precharge timed out, check for discharge relay failure,\
				or higher than expected resistance before continuing\r\n");
				heartbeat_state = 0;
			}
			break;
			
		case 3: // Precharged State
			if (!AIR_power){
				heartbeat_state = 1;
			}

			AIR_neg_relay = 1;
			precharge_relay = 0;
			AIR_pos_relay = 1;
			break;
	}
}

// Suspect issue present due to multiple inputs connected to same net.
// There are more elegant solutions, however this shall suffice.
int validate_adc_input(int raw_adc){
	if (raw_adc > 32768){
		return 0;
	} else {
		return raw_adc;
	}
}

int NTC_voltageToTemperature(float voltage, float BETA=3380){
	float r1 = 2000;
	float vin = 5;
	
	float R2 = 10000;
	float T2 = 25 + 270;

	float resistance;
	int temperature;
	
	resistance = r1/((vin/voltage)-1);
	temperature = ((BETA * T2)/(T2 * log(resistance/R2) + BETA))-270;
	
	return temperature;
}

int HV_voltageScaling(float input, float R_CAL){
	float R1 = 330000 * 4;
	float R2 = 10000 + R_CAL;

	float R1_R2 = R1 + R2;
	float scaling_factor = (R2 / R1_R2);
	float output = input / scaling_factor;
	
	int _output = output;

	return _output;
}

float adc_to_voltage(int adc_value, int adc_resolution, float voltage_range){
	float voltage = adc_value * voltage_range / adc_resolution;
	return voltage;
}

void updateanalogd(){
	pdoc_temperature = NTC_voltageToTemperature(adc_to_voltage(pdoc_adc.readADC_SingleEnded(0), 32768, 6.144), 3380) + 50;
	// pc.printf("PDOC_TEMPERAUTRE: %d \r\n", pdoc_temperature);
	pdoc_ref_temperature = NTC_voltageToTemperature(adc_to_voltage(pdoc_adc.readADC_SingleEnded(1), 32768, 6.144), 3380);
	// pc.printf("PDOC_REF_TEMPERAUTRE: %d \r\n", pdoc_ref_temperature);
	mc_voltage = HV_voltageScaling(adc_to_voltage(validate_adc_input(mc_hv_sense_adc.readADC_SingleEnded(0)), 32768, 6.144), MC_R_CAL);
	// pc.printf("MC_VOLTAGE: %d \r\n", mc_voltage);
	battery_voltage = HV_voltageScaling(adc_to_voltage(validate_adc_input(batt_hv_sense_adc.readADC_SingleEnded(0)), 32768, 6.144), BATT_R_CAL);
	// pc.printf("BATTERY_VOLTAGE: %d \r\n", battery_voltage);
}

//-----------------------------------------------
// Initialisations
//-----------------------------------------------

	/*
SETUP
	Initialisation of CANBUS, ADC, and HEARTBEAT. 
	*/
void setup(){
	can1.frequency(CANBUS_FREQUENCY);
	can1.attach(&CAN1_receive);
	
	ticker_heartbeat.attach(&heartbeat, HEARTRATE);
	ticker_can_transmit.attach(&CAN1_transmit, CAN_BROADCAST_INTERVAL);

	IMD_interface.start();

	orion_connected = false;
}

#if TEST_MODE > 0

// TEST PRECHARGE RELAY SEQUENCING
void test_relays(float time, int delay){
	while(delay){
		AIR_neg_relay = !AIR_neg_relay;
		AIR_pos_relay = !AIR_pos_relay;
		precharge_relay = !precharge_relay;

		pc.printf("AIR_POWER = %d :::: AIR_neg_feedback %f:%f :::: AIR_pos_feedback = %f:%f\r\n", \
		AIR_power, AIR_neg_relay, AIR_neg_feedback, AIR_pos_relay, AIR_pos_feedback);

		wait(time);
	}
}

void test_precharge_sequence(float highest_voltage){
	__disable_irq();
	setup();
	__enable_irq();

	heartbeat_state = 1;

	float R = 4000;
	float C = 0.000270;
	float t = 0;

	battery_voltage = highest_voltage;
	mc_voltage = 0;

	bool latch_test_voltage = false;

	while(1){
		if (precharge_button_state)
			latch_test_voltage = true;

		if (latch_test_voltage == true){
			t = heartbeat_counter-precharge_start_time;

			mc_voltage = highest_voltage*(1-exp(-(t/(R*C))));
		}

		wait(0.01);
		pc.printf("TIME :: %d RC :: %f THEORETICAL :: %f INT_SIMULATED MC VOLTAGE :: %d \r\n", t, R*C, mc_voltage);

		// Manually check analogue values.
		pdoc_temperature = NTC_voltageToTemperature(adc_to_voltage(pdoc_adc.readADC_SingleEnded(0), 32768, 6.144), 3380);
		// pc.printf("PDOC_TEMPERAUTRE: %d \r\n", pdoc_temperature);
		pdoc_ref_temperature = NTC_voltageToTemperature(adc_to_voltage(pdoc_adc.readADC_SingleEnded(1), 32768, 6.144), 3380);
		// pc.printf("PDOC_REF_TEMPERAUTRE: %d \r\n", pdoc_ref_temperature);
		
		stated();
		errord();
		// warnd();
		wait(0.001);
	}
}

void print_current_status(float delay){

	// setup();
	wait(1);

	while(1){
		pc.printf("ts_20 Precharge Controller (STM32F103C8T6 128k) \
		\r\nCOMPILED: %s: %s\r\n",__DATE__, __TIME__);

		pc.printf("\r\n \r\n -- DEVICE STATUS --\r\n");
		pc.printf("HEARTBEAT_STATE: %d \r\n", heartbeat_state);
		pc.printf("HEARTBEAT_COUNTER: %d \r\n", heartbeat_counter);

		// pc.printf("\r\n \r\n -- ERROR SEMIPHORES -- \r\n");
		// pc.printf("ERROR_CODE: %u \r\n", error_code);
		// pc.printf("WARNING_CODE: %u \r\n", warning_code);

		pc.printf("\r\n \r\n -- PERIPHERY STATUSES -- \r\n");
		pc.printf("PRECHARGE_BUTTON_STATE: %d\r\n", precharge_button_state);
		pc.printf("CHARGE_MODE_ACTIVATED: %d \r\n", charge_mode_activated);
		pc.printf("DISCHARGE_STATE: %d \r\n", discharge_state);
		pc.printf("ORION_CONNECTED: %d \r\n", orion_connected);
		pc.printf("ORION_SAFETY_OK %d \r\n", orion_safety_ok);

		pc.printf("\r\n \r\n -- COUNTERS --  \r\n");
		pc.printf("PRECHARGE_START_TIME: %d \r\n", precharge_start_time);
		pc.printf("ORION_LAST_CONNECTION: %d \r\n", orion_last_connection);
		
		pc.printf("\r\n \r\n -- ORION ANALOGUE -- \r\n");
		pc.printf("ORION_LOW_VOLTAGE: %d \r\n", orion_low_voltage);
		pc.printf("ORION_HIGH_VOLTAGE: %d \r\n", orion_high_voltage);
		pc.printf("ORION_HIGH_TEMPERATURE: %d \r\n", orion_high_temperature);

		pc.printf("\r\n \r\n -- INTERNAL ANALOGUE -- \r\n");
		pc.printf("PDOC_TEMPERAUTRE: %d \r\n", pdoc_temperature);
		pc.printf("PDOC_REF_TEMPERAUTRE: %d \r\n", pdoc_ref_temperature);
		pc.printf("MC_VOLTAGE: %d \r\n", mc_voltage);
		pc.printf("BATTERY_VOLTAGE: %d \r\n", battery_voltage);

		pc.printf("\r\n \r\n \r\n \r\n");

		stated();
		// errord();
		updateanalogd();
		// warnd();
		wait(delay);
	}
}

void print_raw_adc(float delay){
	// setup();
	// wait(1);

	while(delay){
		updateanalogd();
		
		pc.printf("\r\n \r\n-- STATUS -- \r\n");

		pc.printf("\r\n-- PDOC_ADC -- \r\n");
		pc.printf("PDOC_RAW 0: %d \r\n", pdoc_adc.readADC_SingleEnded(0));
		pc.printf("PDOC_RAW 1: %d \r\n", pdoc_adc.readADC_SingleEnded(1));
		pc.printf("PDOC_RAW 2: %d \r\n", pdoc_adc.readADC_SingleEnded(0));
		pc.printf("PDOC_RAW 3: %d \r\n", pdoc_adc.readADC_SingleEnded(1));

		pc.printf("\r\n-- MC_VOLTAGE_ADC -- \r\n");
		pc.printf("MC_VOLTAGE_RAW 0: %d \r\n", mc_hv_sense_adc.readADC_SingleEnded(0));
		pc.printf("MC_VOLTAGE_RAW 1: %d \r\n", mc_hv_sense_adc.readADC_SingleEnded(1));
		pc.printf("MC_VOLTAGE_RAW 2: %d \r\n", mc_hv_sense_adc.readADC_SingleEnded(2));
		pc.printf("MC_VOLTAGE_RAW 3: %d \r\n", mc_hv_sense_adc.readADC_SingleEnded(3));
		
		pc.printf("\r\n-- BATTERY_VOLTAGE_ADC -- \r\n");
		pc.printf("BATTERY_VOLTAGE_RAW 0: %d \r\n", batt_hv_sense_adc.readADC_SingleEnded(0));
		pc.printf("BATTERY_VOLTAGE_RAW 1: %d \r\n", batt_hv_sense_adc.readADC_SingleEnded(1));
		pc.printf("BATTERY_VOLTAGE_RAW 2: %d \r\n", batt_hv_sense_adc.readADC_SingleEnded(2));
		pc.printf("BATTERY_VOLTAGE_RAW 3: %d \r\n", batt_hv_sense_adc.readADC_SingleEnded(3));		
		
		pc.printf("\r\n \r\n \r\n \r\n");

		wait(delay);
	}
}

#endif

//-----------------------------------------------
// Program Loop
//-----------------------------------------------

int main() {
	#if TEST_MODE == 1
		test_relays(2, 1);
	#elif TEST_MODE == 2
		test_precharge_sequence(592);
	#elif TEST_MODE == 3
		test_precharge_sequence(250);
	#elif TEST_MODE == 4
		print_current_status(1);
	#elif TEST_MODE == 5
		print_raw_adc(2);
	#else

	__disable_irq();
    pc.printf("Starting ts_20 Precharge Controller (STM32F103C8T6 128k) \
	\r\nCOMPILED: %s: %s\r\n",__DATE__, __TIME__);
	setup();

	// pc.printf("Finished Startup\r\n");
	wait(1);
	__enable_irq();

    while(1) {
		stated();
		errord();
		updateanalogd();
		warnd();
		wait(0.0001);	// Don't stress mcu.
		pc.printf("IMD_FREQUENCY = %d, DUTY_CYCLE = %d\r\n", IMD_interface.get_frequency(), IMD_interface.get_duty_cycle());
    }

	// pc.printf("Is this a BSOD?");
    return 0;

	#endif
}
