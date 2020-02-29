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
*/

//-----------------------------------------------

/*	
The precharge controller is a PCB designed by Team Swinburne to control the precharge sequences of 
a Formula SAE vehicle, and safely manage the health of the accumulator. This PCB integrates the
precharge resistor array, precharge relay, overtemperature safety control, safety interlock for the AMS, 
interface for isolation monitoring device (IMD), and the ability to detect the voltage of the battery and motor
controller.
*/

//-----------------------------------------------

/* Based on STM32F103T6R8 "Blue Pill".

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
Serial pc(PA_9, PA_10);                 //TX, RX

// I2C Interface
I2C 			      	i2c1(PB_7, PB_6);     	//SDA, SCL

// I2C Addresses
#define PDOC_ADC     0x04A

//GND 48
//VDD 49
//SDA 4A
//SCL 4B

// CANBUS Interface
CAN 			    	can1(PB_8, PB_9);     // TXD, RXD
DigitalOut				can1_rx_led(PB_1);
DigitalOut 				can1_tx_led(PB_0);
CANMessage 				can1_msg;

// CANBUS Addresses
#define PRECHARGE_CONTROLLER_HEARTBEAT_ID 			0x440
#define PRECHARGE_CONTROLLER_ANALOGUE_ID			0x441
#define PRECHARGE_CONTROLLER_DIGITAL_ID				0x442

#define DISCHARGE_MODULE_HEARTBEAT_ID			 	0x450
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
bool 	warning_present 						   = 0;
bool	precharge_button_active					   = 0;
bool 	charge_mode_activated					   = 0; 
int 	discharge_state							   = 2;	// (2 - Default)

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
DigitalOut CAN_tx_indicator(PB_0);
DigitalOut CAN_rx_indicator(PB_1);
AnalogIn   pcb_temperature(PA_0);

DigitalOut AIR_neg_relay(PA_8);
DigitalOut precharge_relay(PA_9);
DigitalOut AIR_pos_relay(PA_10);
DigitalOut AMS_ok(PB_13);

DigitalIn IMD_data(PA_15);
DigitalIn AIR_power(PB_3);
DigitalIn discharge_enable(PB_4);
DigitalIn charge_enable(PB_5);
DigitalIn AIR_neg_feedback(PB_10);
DigitalIn AIR_pos_feedback(PB_11);
DigitalIn IMD_ok(PB_12);
DigitalIn TS_ok(PB_14);
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
	char TX_data[2] = {(char)heartbeat_state, (char)heartbeat_counter};
	if(can1.write(CANMessage(PRECHARGE_CONTROLLER_HEARTBEAT_ID, &TX_data[0], 2))) 
	{
       pc.printf("Heartbeat Success! State: %d Counter: %d\n", heartbeat_state, heartbeat_counter);
    }else
	{
		printf("Hearts dead :(\r\n");
	}
}

	/*
CAN RECEIVE
	Message box for CAN messages to be interpreted.
	*/
void CAN1_receive(){
	can1_rx_led = !can1_rx_led;
	if (can1.read(can1_msg)){
		switch(can1_msg.id){
			case DISCHARGE_MODULE_HEARTBEAT_ID:
				discharge_state = can1_msg.data[0];
				break;
			case TC_CHARGER_STATUS_ID:
				charge_mode_activated = 1;
				printf("Entering charge mode. Cycle power to exit charge mode\r\n");
		}
	}
}

	/*
CAN TRANSMIT
	Outgoing messages sent periodically (CAN_BROADCAST_INTERVAL).
	*/
void CAN1_transmit(){
    char TX_data[8] = {0};
}

	/*
READ ADC
	Updates relevant Globals with analogue readings from ADS1115
	*/
void read_adcs(){
	pc.printf("Reading ADC\r\n");
    char cmd[1];
    char data[2];
    
    // Address ADC and Read from Register
	cmd[0] = 0xC2;
	if(!i2c1.write(0x4A << 1, cmd, 1)){
		pc.printf("APPS1 Write Success!\r\n");
	}else
	{
		pc.printf("APPS1 Write Fail\r\n");
	}

	if(!i2c1.read(0x4A << 1, data, 2)){
		int16_t apps_output_1 = (int16_t)((data[0] << 8) | data[1]);
		pc.printf("%d", apps_output_1);
	}
	wait(0.5);
}

//-----------------------------------------------
// Daemons
//-----------------------------------------------

void errord(){
	error_present = 0;
	if (IMD_ok == 0){
		error_present = 1;
		pc.printf("FAULT: Isolation fault detected, please check wiring!\r\n");
	}
	if (PDOC_ok == 0){
		error_present = 1;
		pc.printf("FAULT: PDOC failure detected, please allow to cool and check for\
		short circuit!\r\n");
	}
	if (discharge_enable == 0 && charge_enable == 0)
		error_present = 1;
		pc.printf("FAULT: Orion charge && discharge not available, check Orion status\r\n");
	// ADD ADDITIONAL BATTERY CHECKS
	if (error_present)
		AMS_ok = 0;
	else
		AMS_ok = 1;
}

void warnd(){
	warning_present = 0;
	if (discharge_state > 2 && heartbeat_state != 0){
		warning_present = 1;
		pc.printf("Discharge reported active, please check wiring to discharge.\r\n");
	}
	if (TS_ok == 0){
		warning_present = 1;
		pc.printf("Tractive loop not completed, check all MSDs, HV Plugs, or additional connectors.\r\n");
	}
	if (AIR_neg_feedback != AIR_neg_relay){
		warning_present = 1;
		pc.printf("Negative AIR mismatch, check for welding or wiring failure\r\n");
	}
	if (AIR_pos_feedback != AIR_pos_relay){
		warning_present = 1;
		pc.printf("Positive AIR mismatch, check for welding or wiring failure\r\n");
	}
	if (pcb_temperature > 60){
		warning_present = 1;
		pc.printf("PCB too hot, you should probably check that, but don't take my word\
		for it, i'm just a hot MCU looking to have some fun!");
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
			if (precharge_button_active || charge_mode_activated){
				pc.printf("Beginging precharge sequence\r\n");
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
				pc.printf("Precharge within 95%, safe to close postive contactor\r\n");
				wait(0.1);
				heartbeat_state = 3;
			}
			if (heartbeat_counter > precharge_timeout + 5){
				pc.printf("Precharge timed out, check for discharge relay failure,\
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


	/*
SETUP
	Initialisation of CANBUS, ADC, and PIT. 
	*/
void setup(){
	can1.frequency(250000);
	can1.attach(&CAN1_receive);
	
	ticker_heartbeat.attach(&heartbeat, HEARTRATE);
	ticker_can_transmit.attach(&CAN1_transmit, CAN_BROADCAST_INTERVAL);
}

//-----------------------------------------------
// Program Loop
//-----------------------------------------------

int main() {
    setup();
    pc.printf("Starting\r\n");
    while(1) {
		stated();
		errord();
		if ((heartbeat_counter % 10) == 0)
			warnd();	
        read_adcs(); 
    }

	printf("Is this a BSOD?");
    return 0;
}
