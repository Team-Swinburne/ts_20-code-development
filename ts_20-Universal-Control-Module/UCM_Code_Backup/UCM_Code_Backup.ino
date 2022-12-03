// TEAM SWINBURNE - UNIVERSAL CONTROL MODULE - HARDWARE REVISION 0
// BEN MCINNES, NAM TRAN, BRADLEY REED, PATRICK CURTAIN
// REVISION 2 (24/06/2021)

/***************************************************************************
    UCM1.cpp

    INTRO
    This is the base Arduino code for the UCM
	  DO NOT change this if you want to implement it on one of the UCMs. Instead, create another copy
	  with name UCM{number}, e.g. UCM1

	  UCM 1 & 2 has 4 digital input instead of 2 like the other fan. Drive 1 is purely on off while Driver 2
	  can do PWM (low side).

    Revision     Date          Comments
    --------   ----------     ------------
    0.0        No clue        Initial coding
    1.0        20/06/2021     Updated with ticker class and pcb_temp
    2.0        24/06/2021     Rewrite Ticker as sperated header TickerInterrupt



 /*
 * Upload settings (for USB): 
 * U(S)ART Support: Enabled (generic) serial
 * USB support: CDC (generic "Serial" supersedes U(S)ART
 * Upload method: HID Bootloader 2.2
 * 
 * 
 *
****************************************************************************/

/*--------------------------------------------------------------------------- 
`								LIBRARIES 
---------------------------------------------------------------------------*/

#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <eXoCAN.h>
#include "TickerInterrupt.h"
#include "can_addresses.h"
#include "FlowSensor.h"
#include "UCM3_info.h"

/*--------------------------------------------------------------------------- 
`								INTERFACES 
---------------------------------------------------------------------------*/

// I2C Interface
Adafruit_ADS1115 ads;
#define Serial1_UART_INSTANCE    1 //ex: 2 for Serial2 (USART2)
#define PIN_SERIAL1_RX           PA10
#define PIN_SERIAL1_TX           PA9

// CANBus Interface
eXoCAN can;

//Used to determine which functions need to be uploaded to this particular board
#define UCM_NUMBER 5



#if UCM_NUMBER == 1
  #define UCM_ADDRESS CAN_UCM1_BASE_ADDRESS
#elif UCM_NUMBER == 2
  #define UCM_ADDRESS CAN_UCM2_BASE_ADDRESS
#elif UCM_NUMBER == 3
  #define UCM_ADDRESS CAN_UCM3_BASE_ADDRESS
#elif UCM_NUMBER == 4
  #define UCM_ADDRESS CAN_UCM4_BASE_ADDRESS
#elif UCM_NUMBER == 5
  #define UCM_ADDRESS CAN_UCM5_BASE_ADDRESS
#endif



#define CAN_TEST 0x600
/*--------------------------------------------------------------------------- 
`								GPIOs 
---------------------------------------------------------------------------*/

#define pin_DigIn1               PB10  // Digital Input 1
#define pin_DigIn2               PB11  // Digital Input 2
#define pin_DigIn3				       PB12  // Digital Input 3
#define pin_DigIn4				       PB13  // Digital Input 4
#define pin_Driver1              PB15  // Driver 1 (24V)
#define pin_Driver2              PB1   // Driver 2 (PWM)

/*--------------------------------------------------------------------------- 
`								GLOBALS 
---------------------------------------------------------------------------*/





struct msgFrame
{
  uint8_t len = 8;
  uint8_t bytes[8] = {0};
};

float motorMaxTemp;
float inverterMaxTemp;
float accumulatorTemp;
//Used for storing AIR status, for green loop activation in pump and fan control.
bool canHvActive;

static msgFrame	heartFrame, //{.len = 6},
               	errorFrame,// {.len = 2}, 
				digitalFrame1, //{.len = 1},
               	analogFrame1;

uint8_t rxData[8];

//Modify these two values to change when the sidepod fans turn on/throttle to.
int minTemp = 20;
int maxTemp = 100;

uint8_t motorLHSTemp;
uint8_t motorRHSTemp;

//ADC varible storage
int16_t adc0, adc1, adc2, adc3;

//Flowrate Sensor variables
volatile int FS1_Pulses = 0;
volatile int FS2_Pulses = 0;
volatile int FS3_Pulses = 0;
volatile int FS4_Pulses = 0;

float FS1_FlowRate = 0.0;
float FS2_FlowRate = 0.0;
float FS3_FlowRate = 0.0;
float FS4_FlowRate = 0.0;

/*--------------------------------------------------------------------------- 
`								FUNCTIONS 
---------------------------------------------------------------------------*/
//Function for read the ADC variables
void i2C() {
  adc0 = ads.readADC_SingleEnded(0);
  adc1 = ads.readADC_SingleEnded(1);
  adc2 = ads.readADC_SingleEnded(2);
  adc3 = ads.readADC_SingleEnded(3);  
  
}



// Init all GPIO pins
void GPIO_Init() {
	// Debug LED.
	pinMode(PC13, OUTPUT);
  
	// Configuring the Digital Input Pins.
  pinMode(pin_DigIn1, INPUT);
  pinMode(pin_DigIn2, INPUT);
	pinMode(pin_DigIn3, INPUT);
	pinMode(pin_DigIn4, INPUT);

  //Enable flowrate sensors.
  //for the line below the number corresponds to the aissgned number of flow rate sensors attached to the UCM 
  //if using two, 1 will need to be changed to 2 and pin_DigIn1 will need to be changed to pin_DigIn2 and another init function will need to be created.
  FlowSensor_init(1, pin_DigIn1);

	// Configuring Driver Pins.
  pinMode(pin_Driver1, OUTPUT);
  pinMode(pin_Driver2, OUTPUT);
}

// Transmit hearbeat, letting the other pals know you're alive
void heartbeat() {
	heartFrame.bytes[HEART_COUNTER]++;
	can.transmit(UCM_ADDRESS+TS_HEARTBEAT_ID, 
				heartFrame.bytes, 
				heartFrame.len);
	digitalToggle(PC13);
}

// Transmit digital message
void canTX_Digital1() {
  	can.transmit(UCM_ADDRESS+TS_DIGITAL_1_ID, 
  				digitalFrame1.bytes, 
				digitalFrame1.len);
}
// Transmit analog message
void canTX_Analog1() {
  	can.transmit(UCM_ADDRESS+TS_ANALOGUE_1_ID, 
  				analogFrame1.bytes, 
				analogFrame1.len);
}

// Transmit critical error/warning message
void canTX_criticalError() {
	can.transmit(UCM_ADDRESS+TS_ERROR_WARNING_ID, 
  				errorFrame.bytes, 
				errorFrame.len);
}

void canTransmitPwmData()
{
    can.transmit(UCM_ADDRESS+TS_ANALOGUE_1_ID, 
        analogFrame1.bytes, 
        analogFrame1.len);
}

// Can receive interupt service routine
void canISR() 
{
	can.rxMsgLen = can.receive(can.id, can.fltIdx, can.rxData.bytes);
}

/*--------------------------------------------------------------------------- 
`								DAEMONS 
---------------------------------------------------------------------------*/

// CAN Receive function, use map if neccessary
void canRX() {
	if (can.rxMsgLen > -1) {
		if (can.id == CAN_TEST) {
      		rxData[0] = can.rxData.bytes[0];
      		rxData[1] = can.rxData.bytes[1];
			rxData[2] = can.rxData.bytes[2];
			rxData[3] = can.rxData.bytes[3];
      		can.rxMsgLen = -1;
		}
	}
}

// digital update daemon
void updateDigital() {

  digitalFrame1.bytes[0] = (byte)(digitalRead(pin_DigIn1) << 3);
  digitalFrame1.bytes[0] |= (byte)(digitalRead(pin_DigIn2) << 2);
  digitalFrame1.bytes[0] |= (byte)(digitalRead(pin_DigIn3) << 1);
  digitalFrame1.bytes[0] |= (byte)(digitalRead(pin_DigIn4));

  digitalFrame1.bytes[0] = 1;
}

// analog update daemon
void updateAnalog() {
  // use bitwise operation to split the data
	int16_t adc[4] = {0};
	int16_t mapFrom[4][2]  = {{0,65536}, {0,65536}, {0,1023}, {0,1023}};
	int16_t mapTo[4][2] = {{0,100}, {0,100}, {0,500}, {0, 500}};
  
  	// read ADC values
	for (int i = 0; i <= 3; i++) {
    	adc[i] = ads.readADC_SingleEnded(i);
  	}
	
	// map them as we need, map(value, fromLow, fromHigh, toLow, toHigh)
  	for (int i =0; i <= 3; i++) {
    	adc[i] = map(adc[i], mapFrom[i][0], mapFrom[i][1], mapTo[i][0], mapTo[i][1]);
	}

  	// use bitwise to split the data into 2 bytes as needed
	// &0xFF masks the first 8bits, >>8 shit it 8 bits to the right
	analogFrame1.bytes[0] = adc[0];
	analogFrame1.bytes[1] = adc[1];
	analogFrame1.bytes[2] = (byte)(adc[2] & 0xFF);
	analogFrame1.bytes[3] = (byte)(adc[2] >> 8);
  analogFrame1.bytes[4] = (byte)(adc[3] & 0xFF);
  analogFrame1.bytes[5] = (byte)(adc[3] >> 8);
}



// update the heart message data
void updateHeartdata() {
	heartFrame.bytes[HEART_HARDWARE_REV] = 1;
	heartFrame.bytes[HEART_PCB_TEMP] = 0;
	heartFrame.bytes[4] = rxData[0];
	heartFrame.bytes[5] = rxData[1];
}

// update the drivers value, both PWM and 24V drivers
void updateDrivers() {
	digitalWrite(pin_Driver1,rxData[2]);
	digitalWrite(pin_Driver2,rxData[1]);
}

// print values on to serial
void SerialPrint () {
	//Digital Input 
  	Serial1.print("Digital Input 1: ");  Serial1.println(digitalRead(pin_DigIn1)); 
  	Serial1.print("Digital Input 2: ");  Serial1.println(digitalRead(pin_DigIn2)); 
}



//UCM 1 Functions 
void FlowSensorProcess()
{
  int FS1_FlowRateInt = (int) (FS1_FlowRate * 10);

  (FS1_FlowRateInt > 255) ? (FS1_FlowRateInt = 255) : (FS1_FlowRateInt = FS1_FlowRateInt);
  
  analogFrame1.bytes[0] = FS1_FlowRateInt;

  (FS1_FlowRateInt < 100) ?  errorFrame.bytes[0] = 1 : errorFrame.bytes[0] = 0;
}



int pumpLHS, fanLHS;
void ucm3PwmControl()
{
   if (can.rxMsgLen > -1) 
   {
    switch (can.id)
    {
      case UCM3_SP_CONTROL:
      {
        rxData[0] = can.rxData.bytes[0];
        rxData[1] = can.rxData.bytes[1];
        can.rxMsgLen = -1;
        pumpLHS = rxData[0];
        fanLHS = rxData[1];
        break;
      }
    }
   }
    analogWrite(PB0, pumpLHS);
    analogWrite(PB1, fanLHS);
}

void ucm4PwmControl()
{
  if (can.rxMsgLen > -1) 
  {
    if(can.id == UCM4_RL_CONTROL)
    {
      int pumpRHS = 0, fanRHS = 0;
      pumpRHS = can.rxData.bytes[0];
      fanRHS  = can.rxData.bytes[1];
      can.rxMsgLen = -1;

       analogWrite(PB0, pumpRHS);
       analogWrite(PB1, fanRHS);
    }
  }
}


void ucm5PwmControl()
{
  if (can.rxMsgLen > -1) 
  {
    if(can.id == UCM5_RR_CONTROL)
    {
      int rearFans, brakeLight = 0;
      rearFans      = can.rxData.bytes[0];
      brakeLight  = can.rxData.bytes[1];
      can.rxMsgLen = -1;

      analogWrite(PB0, rearFans);
      analogWrite(PB1, brakeLight);
    }
  }
}





/*--------------------------------------------------------------------------- 
`								SETUP 
---------------------------------------------------------------------------*/

void setup()
{
  Serial1.begin(57600);
  //Serial1.print("Start");

  // Initiallising CAN
  can.begin(STD_ID_LEN, CANBUS_FREQUENCY, PORTB_8_9_XCVR);   //11 Bit Id, 500Kbps
  //can.filterMask16Init(0, CAN_TEST, 0x7ff);
  can.attachInterrupt(canISR);

  //Motor Inverter Fan Control Pins
  //pinMode(PA1, OUTPUT);
  //pinMode(inverterFans, OUTPUT);
  //pinMode(accumulatorFans, OUTPUT);
  //This is necessary to control the fans on the car. See UCM Fan Control in the TS-22 manufacturing
  //page on notion.
  analogWriteFrequency(25000);
    
  GPIO_Init();

  // Start I2C communication with the ADC
  ads.begin(0x49);

  // Start ticker and attach callback to it
  TickerInterrupt Ticker(TIM2,1);
  Ticker.start();
  Ticker.attach(heartbeat,INTERVAL_HEARTBEAT);
  Ticker.attach(canTX_criticalError,INTERVAL_ERROR_WARNING_CRITICAL);
  Ticker.attach(canTX_Digital1,INTERVAL_ERROR_WARNING_LOW_PRIORITY);
  Ticker.attach(canTX_Analog1,INTERVAL_ERROR_WARNING_LOW_PRIORITY);

  pinMode(PB0, OUTPUT);
  pinMode(PB1, OUTPUT);
}

/*--------------------------------------------------------------------------- 
`								MAIN LOOP 
---------------------------------------------------------------------------*/
void loop()
{
  switch (UCM_NUMBER)
  {
    case 1:
      break;
    case 2:
      break;
    case 3:
        ucm3PwmControl();
      break;
    case 4:
        ucm4PwmControl();
      break; 
    case 5:
        ucm5PwmControl();
      break;
    }
}
