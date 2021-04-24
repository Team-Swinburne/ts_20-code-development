// TEAM SWINBURNE - TS_20
// DISCHARGE MODULE
// PATRICK CURTAIN
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

The Discharge Module is a custom PCB developed for use by Team Swinburne in ts_20. It is intended to complement the functionality
of the Precharge Controller. At its core it is a relay and a resistor that disengages when the negative AIR shuts, and high
voltage becomes active, the state of which controlled by the Precharge Controller. Additionally, the module also integrates a 
resistor divider and additional circuitry for the TSAL (Tractive System Active Light). For the ts_20 Discharge Module, a
microcontroller has been added to relay the state of the discharge to avoid a voltage divider situation (Precharge and discharge
resistors both active).

//-----------------------------------------------

Based on STM32F103T6R8 "Blue Pill".

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
// INTERFACES
//-----------------------------------------------

// UART Interface
Serial pc(PA_2, PA_3);              	   //TX, RX

// I2C Interface
I2C i2c1(PB_7, PB_6);     					//SDA, SCL

// I2C Addressess
//GND 48
//VDD 49
//SDA 4A
//SCL 4B
#define PDOC_ADC_ADDR						0x4B
#define MC_HV_SENSE_ADC_ADDR				0x49

Adafruit_ADS1115 pdoc_adc(&i2c1, PDOC_ADC_ADDR);
Adafruit_ADS1115 mc_hv_sense_adc(&i2c1, MC_HV_SENSE_ADC_ADDR);

// CANBUS Interface
CAN can1(PB_8, PB_9);     						// RXD, TXD

// CANBUS Frequency
#define CANBUS_FREQUENCY 							500000

// CANBUS Addresses
#define PRECHARGE_CONTROLLER_HEARTBEAT_ID 			0x440
#define PRECHARGE_CONTROLLER_ERROR_ID				0x441
#define PRECHARGE_CONTROLLER_ANALOGUE_ID			0x442
#define PRECHARGE_CONTROLLER_PERIPHERAL_ID			0x443

#define DISCHARGE_MODULE_HEARTBEAT_ID			 	0x450
#define DISCHARGE_MODULE_ERROR_ID					0x451
#define DISCHARGE_MODULE_ANALOGUE_ID				0x452
#define DISCHARGE_CONTROLLER_PERIPHERAL_ID			0x453
// Throttle Controller
// Brake Module
// Orion BMS 2
// Orion TEM
// Dash
// Motor Controller

#define TC_CHARGER_STATUS_ID						0x18FF50E5

//-----------------------------------------------
// Calibration Factors
//-----------------------------------------------

const int MC_R_CAL = 5000;

//-----------------------------------------------
// Classes
//-----------------------------------------------

class PCB_Temp {
public:
	PCB_Temp(PinName pin)	: _sensor(pin){
		_temperature = PCB_Temp::read();
	}

	int read(){
		return resistanceToTemperature(voltageToResistance(3.3*_sensor.read()));
	}

private:
	const float r1 = 10000;
	const float vin = 4.35;

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

//-----------------------------------------------
// Globals
//-----------------------------------------------

int8_t  heartbeat_state 			               = 0;
int	    heartbeat_counter 			               = 0;
bool 	error_present							   = 1; // (1 - Default)
int8_t	error_code								   = 0;
bool 	warning_present 						   = 0;
int8_t  warning_code							   = 0;
bool	precharge_button_state					   = 0;
bool 	charge_mode_activated					   = 0; 
int 	discharge_state							   = 2;	// (2 - Default)

int16_t pdoc_temperature						   = 0;
int16_t pdoc_ref_temperature					   = 0;
int16_t mc_voltage								   = 0;
int16_t battery_voltage 						   = 0;

// Program Interval Timer Instances
Ticker ticker_heartbeat;
Ticker ticker_can_transmit;

// Interval Periods
#define	HEARTRATE			                        1
#define CAN_BROADCAST_INTERVAL                  	0.05

//-----------------------------------------------
// GPIO 
//-----------------------------------------------
DigitalOut led1(PC_13);
DigitalOut can1_rx_led(PB_1);
DigitalOut can1_tx_led(PB_0);
PCB_Temp   pcb_temperature(PA_0);

DigitalIn discharge_release(PB_11);
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
	if(can1.write(CANMessage(DISCHARGE_MODULE_HEARTBEAT_ID, &TX_data[0], 3))) 
	{
		can1_tx_led = !can1_tx_led;
       	// pc.printf("Heartbeat Success! State: %d Counter: %d\r\n", heartbeat_state, heartbeat_counter);
    }else
	{
		// pc.printf("Hearts dead :(\r\n");
	}
}

	/*
CAN RECEIVE
	Message box for CAN messages to be interpreted.
	*/
void CAN1_receive(){
	can1_rx_led = !can1_rx_led;

	CANMessage can1_msg;
}

/* CAN TRANSMIT
	Outgoing messages sent periodically (CAN_BROADCAST_INTERVAL).
	*/
void CAN1_transmit(){
    char TX_data[8] = {0};

	TX_data[0] = error_present;
	TX_data[1] = error_code;
	TX_data[2] = warning_present;
	TX_data[3] = warning_code; // add stuff for imd period and frequency.
	
	if (can1.write(CANMessage(DISCHARGE_MODULE_ERROR_ID, &TX_data[0], 4))) {
       can1_tx_led = !can1_tx_led;
	   // pc.printf("MESSAGE SUCCESS!\r\n");
    } else {
		// pc.printf("MESSAGE FAIL!\r\n");
	}

	wait(0.0001);

	TX_data[0] = (char)(pdoc_temperature >> 8);
	TX_data[1] = (char)(pdoc_temperature && 255);
	TX_data[2] = (char)(pdoc_ref_temperature >> 8);
	TX_data[3] = (char)(pdoc_ref_temperature && 255);
	TX_data[4] = (char)(mc_voltage >> 8);
	TX_data[5] = (char)(mc_voltage && 255);

	if(can1.write(CANMessage(DISCHARGE_MODULE_ANALOGUE_ID, &TX_data[0], 6))) {
       can1_tx_led = !can1_tx_led;
	   // pc.printf("MESSAGE SUCCESS!\r\n");
    } else {
		// printf("MESSAGE FAIL!\r\n");
	}

	wait(0.0001);

	TX_data[0] = discharge_release;

	if (can1.write(CANMessage(DISCHARGE_CONTROLLER_PERIPHERAL_ID, &TX_data[0], 2))) {
       can1_tx_led = !can1_tx_led;
	   // pc.printf("MESSAGE SUCCESS!\r\n");
    } else {
		// pc.printf("MESSAGE FAIL!\r\n");
	}

	wait(0.0001);
}



//-----------------------------------------------
// Daemons
//-----------------------------------------------

void stated(){
	if (error_present > 0){
		heartbeat_state = discharge_release + 1; 
	}
}

void errord(){
	error_present = 0;
	error_code = 0;
	if (PDOC_ok == 0){
		error_present = 1;
		error_code = error_code + 0b00000010;
		// pc.printf("FAULT: PDOC failure detected, please allow to cool and check for\
		short circuit!\r\n");
	}
	if (error_present)
		heartbeat_state = 0;
}

void warnd(){
	warning_present = 0;
	warning_code = 0;
	if (pcb_temperature.PCB_Temp::read() > 60){
		warning_present = 1;
		warning_code = warning_code + 0b0000001;
		// PCB Overtemperature
		// pc.printf("PCB too hot, you should probably check that, but don't take my word\
		// for it, i'm just a hot MCU looking to have some fun! ;) ");
	}
	if (discharge_state > 2 && heartbeat_state != 0){
		warning_present = 1;
		warning_code = warning_code + 0b00000010;
		// Discharge/Precharge Mismatch
		// pc.printf("Discharge reported active, please check wiring to discharge.\r\n");
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
	pc.printf("MC_VOLTAGE: %d \r\n", mc_voltage);
}
	/*
SETUP
	Initialisation of CANBUS, ADC, and PIT. 
	*/
void setup(){
	can1.frequency(CANBUS_FREQUENCY);
	can1.attach(&CAN1_receive);
	
	ticker_heartbeat.attach(&heartbeat, HEARTRATE);
	ticker_can_transmit.attach(&CAN1_transmit, CAN_BROADCAST_INTERVAL);

}

//-----------------------------------------------
// Program Loop
//-----------------------------------------------

int main() {
	__disable_irq();
    pc.printf("Starting ts_20 Discharge Controller (STM32F103C8T6 128k) \
	\r\nCOMPILED: %s: %s\r\n",__DATE__, __TIME__);
	setup();

	pc.printf("Finished Startup\r\n");
	wait(1);
	__enable_irq();

    while(1) {
		updateanalogd();

		stated();
		errord();
		if ((heartbeat_counter % 10) == 0)
			warnd();
		wait(0.5);
    }

	pc.printf("Is this a BSOD?");
    return 0;
}
