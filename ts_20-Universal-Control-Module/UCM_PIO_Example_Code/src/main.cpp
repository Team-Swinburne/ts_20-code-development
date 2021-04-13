// TEAM SWINBURNE - TS_21
// UCM PIO Example
// BEN MCINNES & PATRICK CURTAIN
// REVISION 0 (13/04/2021)

/* 
To correct limited 64k flash issue: https://github.com/platformio/platform-ststm32/issues/195. Ensure device is 128k model. 
Use the following platformIO initialisation:
	[env:genericSTM32F103C8]
	platform = ststm32@4.6.0
	board = genericSTM32F103C8
	framework = arduino
	board_upload.maximum_size = 120000

//-----------------------------------------------

The UCM (Universal Control Module) is a smaller generic PCB designed by Team Swinburne to fill minor functions, 
and to be used as a teaching tool. 

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

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_ADS1x15.h>
#include <SPI.h>
#include <eXoCAN.h>
#include "STM32TimerInterrupt.h"

//-----------------------------------------------
// Interfaces
//-----------------------------------------------

// UART Interface
#define Serial1_UART_INSTANCE


// I2C Interface


// I2C Addressess
// ADS1115 ADDR PINS: GND 48 - VDD 49 - SDA 4A - SCL 4B
#define DEFAULT_ADC_ADDR								        0x49

// ADC Interfaces
Adafruit_ADS1X15 adc(DEFAULT_ADC_ADDR);

// CANBUS Interface
eXoCAN can;

// CANBUS Addresses
#define DEFAULT_UCM_HEARTBEAT_ID                0x070
#define DEFAULT_UCM_ANALOGUE_ID                 0x071

#define PRECHARGE_CONTROLLER_HEARTBEAT_ID 			0x440
#define PRECHARGE_CONTROLLER_ERROR_ID				    0x441
#define PRECHARGE_CONTROLLER_ANALOGUE_ID			  0x442
#define PRECHARGE_CONTROLLER_PERIPHERAL_ID			0x443

#define DISCHARGE_MODULE_HEARTBEAT_ID			 	    0x450
#define DISCHARGE_MODULE_ERROR_ID					      0x451
#define DISCHARGE_MODULE_ANALOGUE_ID				    0x452
#define DISCHARGE_MODULE_PERIPHERAL_ID				  0x453

// Throttle Controller
#define THROTTLE_CONTROLLER_HEARTBEAT_ID			  0x340
#define THROTTLE_CONTROLLER_ERROR_ID			    	0x341
#define THROTTLE_CONTROLLER_ANALOGUE_ID			  	0x342
#define THROTTLE_CONTROLLER_PERIPERAL_ID	    	0x343

// Brake Module
// Orion BMS 2
#define ORION_BMS_STATUS_ID						        	0x180
#define ORION_BMS_VOLTAGE_ID						        0x181
#define ORION_BMS_TEMPERATURE_ID					      0x182

#define ORION_BMS_RINEHART_LIMITS					      0x202
// Orion TEM
// Dash
// Motor Controllers

#define TC_CHARGER_STATUS_ID						        0x18FF50E5

//-----------------------------------------------
// Calibration Factors
//-----------------------------------------------



//-----------------------------------------------
// Classes
//-----------------------------------------------

class PCB_Temp {
public:
	// PCB_Temp(PinName pin)	: _sensor(pin){
	// 	_temperature = PCB_Temp::_sensor.read();
	// }

	int read(){
		// return resistanceToTemperature(voltageToResistance(3.3*_sensor.read()));
	}

private:
	const float r1 = 10000;
	const float vin = 5;

	const float BETA = 3430;
	const float R2 = 10000;
	const float T2 = 25 + 270;

	float _resistance;
	int _temperature;
	// AnalogIn _sensor;

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

static int8_t  	heartbeat_state 			  = 0;
static int	    heartbeat_counter 			= 0;

int adc_input_values[4];

// Program Interrupt Timer Instances


// Interval Periods
#define	HEARTRATE			 				              1
#define CAN_BROADCAST_INTERVAL              0.05

//-----------------------------------------------
// GPIO 
//-----------------------------------------------

#define led1                                PC13
#define can1_rx_led                         PB0
#define can1_tx_led                         PB1

// Digital Input Pins
#define digital_input_1                     PB10
#define digital_input_2                     PB11
#define digital_input_3                     PB12
#define digital_input_4                     PB13

// Driver Output Pins
#define driver_output_1                     PB15
#define driver_output_2                     PB14




//-----------------------------------------------
// Functions
//-----------------------------------------------

	/*
HEARTBEAT
	The heartbeat is used to relay the state of the device and time since device epoch to the CANBUS, allowing 
	for the logging of faulure events. 
	*/
// void heartbeat(){
// 	heartbeat_counter++;
// 	led1 = !led1;
// 	char TX_data[3] = {(char)heartbeat_state, (char)heartbeat_counter, (char)pcb_temperature.PCB_Temp::read()};
// 	if(can1.write(CANMessage(PRECHARGE_CONTROLLER_HEARTBEAT_ID, &TX_data[0], 3))) {
//        	// pc.printf("Heartbeat Success! State: %d Counter: %d\r\n", heartbeat_state, heartbeat_counter);
//     } else {
// 		// pc.printf("Hearts dead :(\r\n");
// 	}
// }

	/*
CAN RECEIVE
	Message box for CANBUS.
	*/
void CAN1_receive(){


}


/* 
CAN TRANSMIT
	Outgoing messages sent periodically (CAN_BROADCAST_INTERVAL).
*/
void CAN1_transmit(){
  char TX_data[8] = {0};

  // Fill Analogue Input Value 
  TX_data[0] = adc_input_values[0];
  TX_data[1] = adc_input_values[1];
  TX_data[2] = adc_input_values[2];
  TX_data[3] = adc_input_values[3];

  // Fill Digital Input Value
  TX_data[4] = digitalRead(digital_input_1);
  TX_data[5] = digitalRead(digital_input_2);
  TX_data[6] = digitalRead(digital_input_3);
  TX_data[7] = digitalRead(digital_input_4);

  can.transmit(DEFAULT_UCM_ANALOGUE_ID, TX_data, 8);

}

void updateanalogd(){
  for (int i = 0; i < sizeof(adc_input_values); i++){
    adc_input_values[i] = adc.readADC_SingleEnded(i);
  }
}

//-----------------------------------------------
// Initialisations
//-----------------------------------------------
  /*
INITIALISE_PINS
  Initialises all pins, based on previous mappings.
  */
void initialise_pins(){
  pinMode(led1, OUTPUT);
  pinMode(can1_rx_led, OUTPUT);        
  pinMode(can1_tx_led, OUTPUT);

  pinMode(digital_input_1, INPUT);
  pinMode(digital_input_2, INPUT);
  pinMode(digital_input_3, INPUT);
  pinMode(digital_input_4, INPUT);

  pinMode(driver_output_1, OUTPUT);
  pinMode(driver_output_2, OUTPUT);
}


	/*
SETUP
	Initialisation of CANBUS, ADC, and HEARTBEAT. 
	*/
void setup() {
  initialise_pins();

  can.begin(STD_ID_LEN, BR500K, PORTB_8_9_XCVR);           // 11b IDs, 500k bit rate, Pins 8 and 9 with a transceiver chip
  can.filterMask16Init(0, 0, 0x7ff, 0, 0); 

  adc.begin();

  Serial1.begin(9600);
}

//-----------------------------------------------
// Program Loop
//-----------------------------------------------

	/*
LOOP
	Arduino semantic loop because why have one thing, when you can for no reason at all... have two? 
	*/
void loop() {
  updateanalogd();
}