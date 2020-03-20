// TEAM SWINBURNE - TS_20
// DISCHARGE MODULE
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

//-----------------------------------------------
// INTERFACES
//-----------------------------------------------

// UART Interface
Serial pc(PA_2, PA_3);                 		//TX, RX

// I2C Interface
I2C i2c1(PB_7, PB_6);     					//SDA, SCL

// I2C Addresses
#define PDOC_ADC     						0x4A
#define MC_HV_SENSE_ADC						0x49
#define BATT_HV_SENSE_ADC					0x48

// I2C Config Register
const char adc_initial_config[3] = {0x01, 0b11000000, 0b10000000};

//GND 48
//VDD 49
//SDA 4A
//SCL 4B

// CANBUS Interface
CAN can1(PB_8, PB_9);     						// TXD, RXD

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
AnalogIn   pcb_temperature(PA_0);

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
	char TX_data[2] = {(char)heartbeat_state, (char)heartbeat_counter};
	if(can1.write(CANMessage(DISCHARGE_MODULE_HEARTBEAT_ID, &TX_data[0], 2))) 
	{
       	pc.printf("Heartbeat Success! State: %d Counter: %d\r\n", heartbeat_state, heartbeat_counter);
    }else
	{
		pc.printf("Hearts dead :(\r\n");
	}
}

	/*
CAN RECEIVE
	Message box for CAN messages to be interpreted.
	*/
void CAN1_receive(){
	can1_rx_led = !can1_rx_led;
}

/* CAN TRANSMIT
	Outgoing messages sent periodically (CAN_BROADCAST_INTERVAL).
	*/
void CAN1_transmit(){
    char TX_data[8] = {0};

	TX_data[0] = (char)(pdoc_temperature >> 8);
	TX_data[1] = (char)(pdoc_temperature && 255);
	TX_data[2] = (char)(pdoc_ref_temperature >> 8);
	TX_data[3] = (char)(pdoc_ref_temperature && 255);
	TX_data[4] = (char)(mc_voltage >> 8);
	TX_data[5] = (char)(mc_voltage && 255);

	if(can1.write(CANMessage(DISCHARGE_MODULE_ANALOGUE_ID, &TX_data[0], 8))) {
       can1_tx_led = !can1_tx_led;
    } else {
		printf("MESSAGE FAIL!\r\n");
	}
	
	TX_data[0] = error_present;
	TX_data[1] = error_code;
	TX_data[2] = warning_present;
	TX_data[3] = warning_code; // add stuff for imd period and frequency.
	
	if (can1.write(CANMessage(DISCHARGE_MODULE_ERROR_ID, &TX_data[0], 4))) {
       can1_tx_led = !can1_tx_led;
    } else {
		printf("MESSAGE FAIL!\r\n");
	}

	TX_data[0] = PDOC_ok;
	TX_data[1] = discharge_release;

	if (can1.write(CANMessage(DISCHARGE_CONTROLLER_PERIPHERAL_ID, &TX_data[0], 2))) {
       can1_tx_led = !can1_tx_led;
    } else {
		printf("MESSAGE FAIL!\r\n");
	}
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
		pc.printf("FAULT: PDOC failure detected, please allow to cool and check for\
		short circuit!\r\n");
	}
	if (error_present)
		heartbeat_state = 0;
}

void warnd(){
	warning_present = 0;
	warning_code = 0;
	if (discharge_state > 2 && heartbeat_state != 0){
		warning_present = 1;
		warning_code = warning_code + 0b00000001;
		pc.printf("Discharge reported active, please check wiring to discharge.\r\n");
	}
	if (pcb_temperature > 60){
		warning_present = 1;
		warning_code = warning_code + 0b00010000;
		pc.printf("PCB too hot, you should probably check that, but don't take my word\
		for it, i'm just a hot MCU looking to have some fun! ;) ");
	}
}

void updateanalogd(){
	char reg[3];
	char data[2];
	if(!i2c1.read(PDOC_ADC << 1, data, 2)){
		int16_t output = (int16_t)((data[0] << 8) | data[1]);
		// pc.printf("Reading: %d", output);
		float voltage = ((float)output * 6.144)/32767;
		pdoc_temperature = voltage * 10;
	}

	if(!i2c1.read(MC_HV_SENSE_ADC << 1, data, 2)){
		int16_t output = (int16_t)((data[0] << 8) | data[1]);
		// pc.printf("Reading: %d", output);
		float voltage = ((float)output * 6.144)/32767;
		// pc.printf(" -- Converted to %f\r\n", convert);
		mc_voltage = voltage * 10;
	}
}

void initialiseADC(){
	char cmd[1];
    
	if(!i2c1.write(PDOC_ADC << 1, adc_initial_config, 2)){
		pc.printf("PDOC ADC Write Success!\r\n");
	} else {
		pc.printf("PDOC ADC Write Fail\r\n");
	}

	if(!i2c1.write(MC_HV_SENSE_ADC << 1, adc_initial_config, 2)){
		pc.printf("MC ADC Write Success!\r\n");
	} else {
		pc.printf("MC ADC Write Fail\r\n");
	}

	cmd[0] = 0x00;

	if(!i2c1.write(PDOC_ADC << 1, cmd, 1)){
		pc.printf("PDOC ADC Write Success!\r\n");
	} else {
		pc.printf("PDOC ADC Write Fail\r\n");
	}

	if(!i2c1.write(MC_HV_SENSE_ADC << 1, cmd, 1)){
		pc.printf("MC ADC Write Success!\r\n");
	} else {
		pc.printf("MC ADC Write Fail\r\n");
	}
}

	/*
SETUP
	Initialisation of CANBUS, ADC, and PIT. 
	*/
void setup(){
	can1.frequency(250000);
	can1.attach(&CAN1_receive);
	
	ticker_heartbeat.attach(&heartbeat, HEARTRATE);
	ticker_can_transmit.attach(&CAN1_transmit, CAN_BROADCAST_INTERVAL);

	initialiseADC();
}

//-----------------------------------------------
// Program Loop
//-----------------------------------------------

int main() {
    pc.printf("Starting ts_20 Precharge Controller (STM32F103C8T6 128k) \
	\r\nCOMPILED: %s: %s\r\n",__DATE__, __TIME__);
	setup();

	pc.printf("Finished Startup\r\n");
	wait(1);

    while(1) {
		stated();
		errord();
		if ((heartbeat_counter % 10) == 0)
			warnd();
    }

	printf("Is this a BSOD?");
    return 0;
}
