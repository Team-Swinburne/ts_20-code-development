// TEAM SWINBURNE - TS_20
// PRECHARGE CONTROLLER
// MICHAEL COCHRANE & PATRICK CURTAIN
// REVISION 0 (29/02/2020)

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

class PCB_Temp {
public:
	PCB_Temp(PinName pin)	: _sensor(pin){
		_temperature = PCB_Temp::_sensor.read();
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
	IMD_Data(PinName pin) 	: _interrupt(pin) {}

	void start(){
		_t1 = 0;
		_interrupt.rise(this, &IMD_Data::setPeriod);
		_interrupt.fall(this, &IMD_Data::setDutyCycle);
		_t.start();
	}

	void setPeriod(){
		_t3 = _t.read();
		_T1 = _t1 - _t3;
		_F1 = 1/_T1;
	}

	void setDutyCycle(){
		_t2 = _t.read();
		_T2 = _t2 - _t1;
		_dutyCycle = _T2/_T1;
	}

	int readPeriod(){
		// Return ms * 100
		return _T2 * 100;
	}

	int readDutyCycle(){
		return _dutyCycle;
	}

private:
	InterruptIn _interrupt;

	volatile float	_t1;
	volatile float 	_t2;
	volatile float 	_t3;
	
	volatile float 	_T1;
	volatile float 	_T2;

	volatile int	_F1;
	volatile int	_dutyCycle;

	Timer _t;
};

//-----------------------------------------------
// Globals
//-----------------------------------------------

static int8_t  	heartbeat_state 			= 0;
static int	    heartbeat_counter 			= 0;
static bool 	error_present				= 1; 	// (1 - Default)
static int8_t	error_code					= 0;
static bool 	warning_present 			= 0;
static int8_t  	warning_code				= 0;
static bool		precharge_button_state		= 0;
static bool 	charge_mode_activated		= 0; 
static int 		discharge_state				= 2;	// (2 - Default)
static int 		orion_timeout_counter		= 0;
static bool 	orion_connected	 			= 0;	// (0 - Default)

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
Ticker ticker_orionwd;

// Interval Periods
#define	HEARTRATE			 				1
#define CAN_BROADCAST_INTERVAL              0.05
#define ORION_TIMEOUT						0.1

//-----------------------------------------------
// GPIO 
//-----------------------------------------------

DigitalOut led1(PC_13);
DigitalOut can1_rx_led(PB_1);
DigitalOut can1_tx_led(PB_0);
PCB_Temp   pcb_temperature(PA_0);

DigitalOut AIR_neg_relay(PA_8);
DigitalOut precharge_relay(PA_9); //pa_
DigitalOut AIR_pos_relay(PA_10); //pa_10
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
HEARTBEAT
	The heartbeat is used to relay the state of the device and time since device epoch to the CANBUS, allowing 
	for the logging of faulure events. 
	*/
void heartbeat(){
	heartbeat_counter++;
	led1 = !led1;
	char TX_data[3] = {(char)heartbeat_state, (char)heartbeat_counter, (char)pcb_temperature.PCB_Temp::read()};
	if(can1.write(CANMessage(PRECHARGE_CONTROLLER_HEARTBEAT_ID, &TX_data[0], 3))) {
       	// pc.printf("Heartbeat Success! State: %d Counter: %d\r\n", heartbeat_state, heartbeat_counter);
    } else {
		// pc.printf("Hearts dead :(\r\n");
	}
}

	/*
CAN RECEIVE
	Message box for CANBUS.
	*/
void CAN1_receive(){
	can1_rx_led = !can1_rx_led;

	CANMessage can1_msg;
	
	if (can1.read(can1_msg)){
		switch(can1_msg.id){
			case DISCHARGE_MODULE_HEARTBEAT_ID:
				discharge_state = can1_msg.data[0];
				break;

			case THROTTLE_CONTROLLER_PERIPERAL_ID:
				precharge_button_state = can1_msg.data[0];

			case ORION_BMS_STATUS_ID:
				// Big endian & MSB
				// 0b000000, 0000001	Discharge Relay Enabled
				// 0b000000, 0000010	Charge Relay Enabled
				// 0b000000, 0000100	Charge Safety Enabled
				// Use bitwise operator to mask out all except relevent statuses.
				if (can1_msg.data[1] & 0b0000000000000111 > 0)
					orion_connected = true;	
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
		
	TX_data[0] = error_present;
	TX_data[1] = error_code;
	TX_data[2] = warning_present;
	TX_data[3] = warning_code; // add stuff for imd period and frequency.
	TX_data[4] = IMD_interface.IMD_Data::readPeriod();
	TX_data[5] = IMD_interface.IMD_Data::readDutyCycle();
	
	if (can1.write(CANMessage(PRECHARGE_CONTROLLER_ERROR_ID, &TX_data[0], 4))) {
       can1_tx_led = !can1_tx_led;
    } else {
		// printf("MESSAGE FAIL!\r\n");
	}

	TX_data[0] = (char)(pdoc_temperature >> 8);
	TX_data[1] = (char)(pdoc_temperature && 255);
	TX_data[2] = (char)(pdoc_ref_temperature >> 8);
	TX_data[3] = (char)(pdoc_ref_temperature && 255);
	TX_data[4] = (char)(mc_voltage >> 8);
	TX_data[5] = (char)(mc_voltage && 255);
	TX_data[6] = (char)(battery_voltage >> 8);
	TX_data[7] = (char)(battery_voltage && 255);

	if(can1.write(CANMessage(PRECHARGE_CONTROLLER_ANALOGUE_ID, &TX_data[0], 8))) {
       can1_tx_led = !can1_tx_led;
    } else {
		// printf("MESSAGE FAIL!\r\n");
	}

	TX_data[0] = precharge_relay;
	TX_data[1] = IMD_interface.readPeriod();
	TX_data[2] = IMD_interface.readDutyCycle();
	TX_data[3] = AIR_power;
	TX_data[4] = AIR_neg_feedback;
	TX_data[5] = AIR_pos_feedback;

	if (can1.write(CANMessage(PRECHARGE_CONTROLLER_PERIPHERAL_ID, &TX_data[0], 5))) {
       can1_tx_led = !can1_tx_led;
    } else {
		// printf("MESSAGE FAIL!\r\n");
	}
}

//-----------------------------------------------
// Daemons
//-----------------------------------------------

void orionwd(){
	if (heartbeat_state > 0){
		if ((orion_timeout_counter/10) >= ORION_TIMEOUT){
			orion_connected = false;
		}
		orion_timeout_counter += 1;
	}
}

void errord(){
	error_present = 0;
	error_code = 0;
	if (IMD_ok == 0){
		error_present = 1;
		error_code = error_code + 0b00000001;
		// pc.printf("FAULT: Isolation fault detected, please check wiring!\r\n");
	}
	if (PDOC_ok == 0){
		error_present = 1;
		error_code = error_code + 0b00000010;
		// pc.printf("FAULT: PDOC failure detected, please allow to cool and check for\
		short circuit!\r\n");
	}
	if (orion_connected != 1){
		error_present = 1;
		error_code = error_code + 0b00000100;
		// pc.printf("FAULT: Orion BMS not attached, please check CAN is functioning, and\
		Orion is attached!\r\n");
	}
	if (orion_low_voltage < MINIMUM_CELL_VOLTAGE){
		error_present = 1;
		error_code = error_code + 0b00001000;
		// pc.printf("FAULT: Orion reports undervoltage fault!\r\n");
	}
	if (orion_high_voltage > MAXIMUM_CELL_VOLTAGE){
		error_present = 1;
		error_code = error_code + 0b000010000;
		// pc.printf("FAULT: Orion reports overvoltage fault!\r\n");
	}
	if (orion_high_temperature > MAXIMUM_CELL_TEMPERATURE){
		error_present = 1;
		error_code = error_code + 0b000100000;
		// pc.printf("FAULT: Orion reports overtemperature fault!\r\n");
	}
	if (error_present)
		heartbeat_state = 0;
}

void warnd(){
	warning_present = 0;
	warning_code = 0;
	if (pcb_temperature.PCB_Temp::read() > 60){
		warning_present = 1;
		warning_code = warning_code + 0b00000001;
		// PCB Overtemperature
		// pc.printf("PCB too hot, you should probably check that, but don't take my word\
		for it, i'm just a hot MCU looking to have some fun! ;) ");
	}
	if (discharge_state > 2 && heartbeat_state != 0){
		warning_present = 1;
		warning_code = warning_code + 0b00000010;
		// Discharge/Precharge Mismatch
		// pc.printf("Discharge reported active during drive, please check wiring to discharge.\r\n");
	}
	if (AIR_neg_feedback != AIR_neg_relay){
		warning_present = 1;
		warning_code = warning_code + 0b0000100;
		// Negative AIR Mismatch
		// pc.printf("Negative AIR mismatch, check for welding or wiring failure\r\n");
	}
	if (AIR_pos_feedback != AIR_pos_relay){
		warning_present = 1;
		warning_code = warning_code + 0b00001000;
		// Positive AIR Mismatch
		// pc.printf("Positive AIR mismatch, check for welding or wiring failure\r\n");
	}
}

void stated(){
	int precharge_timeout = 0;

	switch(heartbeat_state){
		case 0:	// Fail State (Default){
			AIR_neg_relay = 0;
			precharge_relay = 0;
			AIR_pos_relay = 0;
			AMS_ok = 0;
			break;
		case 1: // Idle State
			if (precharge_button_state || charge_mode_activated){
				// pc.printf("Beginging precharge sequence\r\n");
				heartbeat_state = 2;
			}
			break;
		case 2: // Precharging State
			AIR_neg_relay = 1;
			precharge_relay = 0;
			AIR_pos_relay = 0;

			wait(0.5);	// Safeguard for discharge relay
			precharge_timeout = heartbeat_counter;
			
			precharge_relay = 1;
			if (battery_voltage*0.95 > mc_voltage){
				// pc.printf("Precharge within 95%, safe to close postive contactor\r\n");
				wait(0.1);
				heartbeat_state = 3;
			}
			if (heartbeat_counter > precharge_timeout + 5){
				// pc.printf("Precharge timed out, check for discharge relay failure,\
				or higher than expected resistance before continuing\r\n");
				heartbeat_state = 0;
			}
			break;
		case 3: // Precharged State
			AIR_neg_relay = 1;
			precharge_relay = 1;
			AIR_pos_relay = 1;
			break;
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

float HV_voltageScaling(float input, int R_CAL){
	int R1 = 330000 * 4;
	int R2 = 10000 + R_CAL;

	float scaling_factor = ((R1 + R2) / R1);
	float output = input * scaling_factor;

	return output;
}

float adc_to_voltage(int adc_value, int adc_resolution, float voltage_range){
	float voltage = adc_value * voltage_range / adc_resolution;
	return voltage;
}

void updateanalogd(){
	pdoc_temperature = NTC_voltageToTemperature(adc_to_voltage(pdoc_adc.readADC_SingleEnded(0), 32768, 6.144), 3380);
	// pc.printf("PDOC_TEMPERAUTRE: %d \r\n", pdoc_temperature);
	pdoc_ref_temperature = NTC_voltageToTemperature(adc_to_voltage(pdoc_adc.readADC_SingleEnded(1), 32768, 6.144), 3380);
	// pc.printf("PDOC_REF_TEMPERAUTRE: %d \r\n", pdoc_ref_temperature);
	mc_voltage = HV_voltageScaling(adc_to_voltage(mc_hv_sense_adc.readADC_SingleEnded(0), 32768, 6.144), MC_R_CAL);
	// pc.printf("MC_VOLTAGE: %d \r\n", mc_voltage);
	battery_voltage = HV_voltageScaling(adc_to_voltage(batt_hv_sense_adc.readADC_SingleEnded(0), 32768, 6.144), BATT_R_CAL);
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
	can1.frequency(500000);
	can1.attach(&CAN1_receive);
	
	ticker_heartbeat.attach(&heartbeat, HEARTRATE);
	ticker_can_transmit.attach(&CAN1_transmit, CAN_BROADCAST_INTERVAL);

	ticker_orionwd.attach(&orionwd, ORION_TIMEOUT);

	IMD_interface.IMD_Data::start();
}

//-----------------------------------------------
// Program Loop
//-----------------------------------------------

int main() {
	__disable_irq();
    pc.printf("Starting ts_20 Precharge Controller (STM32F103C8T6 128k) \
	\r\nCOMPILED: %s: %s\r\n",__DATE__, __TIME__);
	setup();

	pc.printf("Finished Startup\r\n");
	wait(1);
	__enable_irq();

    while(1) {
		stated();
		errord();
		updateanalogd();
		if ((heartbeat_counter % 10) == 0)
			warnd();
		wait(0.1);
    }

	pc.printf("Is this a BSOD?");
    return 0;
}
