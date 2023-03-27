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
#define UCM_NUMBER 4

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

#define ADC_VREF 6.144f
#define ADC_BIT_RESOLUTION 32767

#define rightFanControlPin PB0
#define leftFanControlPin PB1
#define inverterFans PB1
#define accumulatorFans PB0
#define motorCoolingPumpLeft PB0
#define motorCoolingPumpRight PB1

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

int inverterMaxTemp;
//Used for storing AIR status, for green loop activation in pump and fan control.
bool canHvActive;

static msgFrame	heartFrame, //{.len = 6},
               	errorFrame,// {.len = 2}, 
				digitalFrame1, //{.len = 1},
               	analogFrame1,
               	analogFrame2;

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
void i2C() 
{
  adc0 = ads.readADC_SingleEnded(0);
  adc1 = ads.readADC_SingleEnded(1);
  adc2 = ads.readADC_SingleEnded(2);
  adc3 = ads.readADC_SingleEnded(3);  
  
}

//Functions for modifying the fan curve
float gradientValue()
{
  // Increasing this value steepens the curve.
  int multiplier = 1;
  return multiplier * (70.0/(pow(maxTemp,3)-pow(minTemp,3)));
}

float cValue()
{
  return 30 - (gradientValue()*pow(minTemp,3));
}

// Init all GPIO pins
void GPIO_Init() 
{
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
void heartbeat() 
{
	heartFrame.bytes[HEART_COUNTER]++;
	can.transmit(UCM_ADDRESS+TS_HEARTBEAT_ID, 
				heartFrame.bytes, 
				heartFrame.len);
	digitalToggle(PC13);
}

// Transmit digital message
void canTX_Digital1() 
{
  	can.transmit(UCM_ADDRESS+TS_DIGITAL_1_ID, 
  				digitalFrame1.bytes, 
				digitalFrame1.len);
}

// Transmit analog 1 message
void canTX_Analog1() 
{
  	can.transmit(UCM_ADDRESS+TS_ANALOGUE_1_ID, 
  				analogFrame1.bytes, 
				analogFrame1.len);
}

// Transmit analog 2 message
void canTX_Analog2()
{
  can.transmit(UCM_ADDRESS+TS_ANALOGUE_2_ID, 
          analogFrame2.bytes, 
        analogFrame2.len);
}

// Transmit critical error/warning message
void canTX_criticalError() 
{
	can.transmit(UCM_ADDRESS+TS_ERROR_WARNING_ID, 
  				errorFrame.bytes, 
				errorFrame.len);
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
void canRX() 
{
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
void updateDigital() 
{

  digitalFrame1.bytes[0] = (byte)(digitalRead(pin_DigIn1) << 3);
  digitalFrame1.bytes[0] |= (byte)(digitalRead(pin_DigIn2) << 2);
  digitalFrame1.bytes[0] |= (byte)(digitalRead(pin_DigIn3) << 1);
  digitalFrame1.bytes[0] |= (byte)(digitalRead(pin_DigIn4));

  digitalFrame1.bytes[0] = 1;
}

// analog update daemon
void updateAnalog() 
{
  float Voltage[4] = { 0.0 };
  int VoltageInt[4] = { 0 };

  for(int i = 0; i < 4; i++)
  {
    Voltage[i] = (ADC_VREF/ADC_BIT_RESOLUTION)*(ads.readADC_SingleEnded(i));

    //Converts float to int with mV resolution
    VoltageInt[i] = (1000*Voltage[i]);
  }

  //Transmit voltages read
  analogFrame1.bytes[0] = (VoltageInt[0] >> 8) & 0xFF;
  analogFrame1.bytes[1] = (VoltageInt[0]) & 0xFF;
  analogFrame1.bytes[2] = (VoltageInt[1] >> 8) & 0xFF;
  analogFrame1.bytes[3] = (VoltageInt[1]) & 0xFF;
  analogFrame1.bytes[4] = (VoltageInt[1] >> 8) & 0xFF;
  analogFrame1.bytes[5] = (VoltageInt[1]) & 0xFF;
  analogFrame1.bytes[6] = (VoltageInt[1] >> 8) & 0xFF;
  analogFrame1.bytes[7] = (VoltageInt[1]) & 0xFF;
  
  //Convert voltages read from NTC thermistors into temperature
  int Thermistor_Resistance[2] = { 0 };
  float Temperature[2] = { 0 };
  int TemperatureInt[2] = { 0 };

  //Thermistor calibration factors
  double A = 0.0008235388853;
  double B = 0.0002632202645;
  double C = 0.0000001349156215;

  for(int i = 0; i < 2; i++)
  {
    Thermistor_Resistance[i] = (((Voltage[i]/5) - 1)*1000 + ((Voltage[i]/5)*10000))/(1 - (Voltage[i]/5));
  }

  for(int i = 0; i < 2; i++)
  {
    Temperature[i] = (1.0/(A + B*log(Thermistor_Resistance[i]) + C*(log(Thermistor_Resistance[i])*log(Thermistor_Resistance[i])*log(Thermistor_Resistance[i])))) - 273.15;

    //Convert float to int with 0.01 degree C resolution
    TemperatureInt[i] = 100*Temperature[i];
  }

  analogFrame2.bytes[0] = (TemperatureInt[0] >> 8) & 0xFF;
  analogFrame2.bytes[1] = (TemperatureInt[0]) & 0xFF;
  analogFrame2.bytes[2] = (TemperatureInt[1] >> 8) & 0xFF;
  analogFrame2.bytes[3] = (TemperatureInt[1]) & 0xFF;
}

// update the heart message data
void updateHeartdata() 
{
	heartFrame.bytes[HEART_HARDWARE_REV] = 1;
	heartFrame.bytes[HEART_PCB_TEMP] = 0;
	heartFrame.bytes[4] = rxData[0];
	heartFrame.bytes[5] = rxData[1];
}

// update the drivers value, both PWM and 24V drivers
void updateDrivers() 
{
	digitalWrite(pin_Driver1,rxData[2]);
	digitalWrite(pin_Driver2,rxData[1]);
}

// print values on to serial
void SerialPrint () 
{
	//Digital Input 
  	Serial1.print("Digital Input 1: ");  Serial1.println(digitalRead(pin_DigIn1)); 
  	Serial1.print("Digital Input 2: ");  Serial1.println(digitalRead(pin_DigIn2)); 
}

//Green loop activation function
void greenLoopActive() 
{
  if (can.rxMsgLen > -1) 
  {
    if(can.id == CAN_PRECHARGE_CONTROLLER_DIGITAL_1_ID)
    {
      //Motor Temperature:
          rxData[0] = can.rxData.bytes[0];
          can.rxMsgLen = -1;
          canHvActive = rxData[0];
    }
  }
}


//UCM 1 Functions 
void FlowSensorProcess()
{
  int FS1_FlowRateInt = (int) (FS1_FlowRate * 10);

  (FS1_FlowRateInt > 255) ? (FS1_FlowRateInt = 255) : (FS1_FlowRateInt = FS1_FlowRateInt);
  
  analogFrame1.bytes[0] = FS1_FlowRateInt;

  (FS1_FlowRateInt < 100) ?  errorFrame.bytes[0] = 1 : errorFrame.bytes[0] = 0;
}




//UCM 3 Functions
//Motor Temps (L+R) off CAN

//Gets the temperature of the Left and Right motors off the CAN bus.
void canMotorTemp() 
{
  if (can.rxMsgLen > -1) 
  {
    if(can.id == CAN_INVERTER_PASSTHROUGH)
    {
      //Motor Temperature:
          rxData[0] = can.rxData.bytes[2];
          rxData[1] = can.rxData.bytes[3];
          can.rxMsgLen = -1;
          motorLHSTemp = rxData[0];
          motorRHSTemp = rxData[1];
    }
  }
}



void sidepodFanControlLHS()
{
  float gradient, c;

  gradient = gradientValue();
  c = cValue();
  
  float pwmTempL, pwmTempR;
  //this line maps the temperature of the motor controller onto a value from 0-256 to be outputted to the
  //fans. 

  //LHS
  float pwmPercentageL = gradient*(pow((motorLHSTemp),3))+c;
  pwmTempL = (pwmPercentageL/100)*255;
  int pwmSignalL = (int) pwmTempL;


  /*
  //RHS 
  float pwmPercentageR = gradient*(pow((motorRHSTemp),3))+c;
  pwmTempR = (pwmPercentageR/100)*255;
  int pwmSignalR = (int) pwmTempR;
  */

  //LHS PWM output
  if (motorLHSTemp <39) 
  {
    analogWrite(leftFanControlPin, 120);
  }
  else if (motorLHSTemp > 60 && 120 >= motorLHSTemp)
  {
    analogWrite(leftFanControlPin, 255);
  }
  else if(motorLHSTemp > 120)
    {
    analogWrite(leftFanControlPin, 179);
  }
  else
  {
    analogWrite(leftFanControlPin, pwmSignalL);
  }

  /*
  //RHS PWM output 
  if (motorRHSTemp <39) 
  {
    analogWrite(rightFanControlPin, 120);
  }
  else if (motorRHSTemp > 60 && 120 >= motorRHSTemp)
  {
    analogWrite(rightFanControlPin, 255);
  }
  else if(motorRHSTemp > 120)
    {
    analogWrite(rightFanControlPin, 179);
  }
  else
  {
    analogWrite(rightFanControlPin, pwmSignalR);
  }
  */
}


void motorPumpControlLHS()
{
  float gradient, c;

  gradient = gradientValue();
  c = cValue();
  
  float pwmTempL, pwmTempR;
  //this line maps the temperature of the motor controller onto a value from 0-256 to be outputted to the
  //fans. 

  //LHS
  float pwmPercentageL = gradient*(pow((motorLHSTemp),3))+c;
  pwmTempL = (pwmPercentageL/100)*255;
  int pwmSignalL = (int) pwmTempL;

  /*
  //RHS 
  float pwmPercentageR = gradient*(pow((motorRHSTemp),3))+c;
  pwmTempR = (pwmPercentageR/100)*255;
  int pwmSignalR = (int) pwmTempR;
  */

  //LHS PWM output
  if (motorLHSTemp <39) 
  {
    analogWrite(motorCoolingPumpLeft, 120);
  }
  else if (motorLHSTemp > 60 && 120 >= motorLHSTemp)
  {
    analogWrite(motorCoolingPumpLeft, 255);
  }
  else if(motorLHSTemp > 120)
    {
    analogWrite(motorCoolingPumpLeft, 179);
  }
  else
  {
    analogWrite(motorCoolingPumpLeft, pwmSignalL);
  }

  
  /*
  //RHS PWM output 
    if (motorRHSTemp <39) 
  {
    analogWrite(motorCoolingPumpRight, 120);
  }
  else if (motorRHSTemp > 60 && 120 >= motorRHSTemp)
  {
    analogWrite(motorCoolingPumpRight, 255);
  }
  else if(motorRHSTemp > 120)
    {
    analogWrite(motorCoolingPumpRight, 179);
  }
  else
  {
    analogWrite(motorCoolingPumpRight, pwmSignalR);
  }
  */
}





//UCM 4 functions


void sidepodFanControlRHS()
{
  float gradient, c;

  gradient = gradientValue();
  c = cValue();
  
  float pwmTempL, pwmTempR;
  //this line maps the temperature of the motor controller onto a value from 0-256 to be outputted to the
  //fans. 

  /*
  //LHS
  float pwmPercentageL = gradient*(pow((motorLHSTemp),3))+c;
  pwmTempL = (pwmPercentageL/100)*255;
  int pwmSignalL = (int) pwmTempL;
  */

  
  //RHS 
  float pwmPercentageR = gradient*(pow((motorRHSTemp),3))+c;
  pwmTempR = (pwmPercentageR/100)*255;
  int pwmSignalR = (int) pwmTempR;
  
  /*
  //LHS PWM output
  if (motorLHSTemp <39) 
  {
    analogWrite(leftFanControlPin, 120);
  }
  else if (motorLHSTemp > 60 && 120 >= motorLHSTemp)
  {
    analogWrite(leftFanControlPin, 255);
  }
  else if(motorLHSTemp > 120)
    {
    analogWrite(leftFanControlPin, 179);
  }
  else
  {
    analogWrite(leftFanControlPin, pwmSignalL);
  }
  */

  //RHS PWM output 
  if (motorRHSTemp <39) 
  {
    analogWrite(rightFanControlPin, 120);
  }
  else if (motorRHSTemp > 60 && 120 >= motorRHSTemp)
  {
    analogWrite(rightFanControlPin, 255);
  }
  else if(motorRHSTemp > 120)
    {
    analogWrite(rightFanControlPin, 179);
  }
  else
  {
    analogWrite(rightFanControlPin, pwmSignalR);
  }
}


void motorPumpControlRHS()
{
  float gradient, c;

  gradient = gradientValue();
  c = cValue();
  
  float pwmTempL, pwmTempR;
  //this line maps the temperature of the motor controller onto a value from 0-256 to be outputted to the
  //fans. 

  /*
  //LHS
  float pwmPercentageL = gradient*(pow((motorLHSTemp),3))+c;
  pwmTempL = (pwmPercentageL/100)*255;
  int pwmSignalL = (int) pwmTempL;
  */
  
  //RHS 
  float pwmPercentageR = gradient*(pow((motorRHSTemp),3))+c;
  pwmTempR = (pwmPercentageR/100)*255;
  int pwmSignalR = (int) pwmTempR;
  
  /*
  //LHS PWM output
  if (motorLHSTemp <39) 
  {
    analogWrite(motorCoolingPumpLeft, 120);
  }
  else if (motorLHSTemp > 60 && 120 >= motorLHSTemp)
  {
    analogWrite(motorCoolingPumpLeft, 255);
  }
  else if(motorLHSTemp > 120)
    {
    analogWrite(motorCoolingPumpLeft, 179);
  }
  else
  {
    analogWrite(motorCoolingPumpLeft, pwmSignalL);
  }
  */
  
  
  //RHS PWM output 
    if (motorRHSTemp <39) 
  {
    analogWrite(motorCoolingPumpRight, 120);
  }
  else if (motorRHSTemp > 60 && 120 >= motorRHSTemp)
  {
    analogWrite(motorCoolingPumpRight, 255);
  }
  else if(motorRHSTemp > 120)
  {
    analogWrite(motorCoolingPumpRight, 179);
  }
  else
  {
    analogWrite(motorCoolingPumpRight, pwmSignalR);
  }
}


//UCM 5
//Accumulator Fan Control
void canAccumulator() 
{
  if (can.rxMsgLen > -1) {
  switch (can.id)
    {
      case CAN_INVERTER_PASSTHROUGH:
        {
          //Motor Inverter Temperature:
          rxData[0] = can.rxData.bytes[0];
          can.rxMsgLen = -1;
          inverterMaxTemp = rxData[0];
        break;
        }
    }
  }
}

void fanControlAccumulator()
{
  float pwmtemp;
  //this line maps the temperature of the motor controller onto a value from 0-256 to be outputted to the
  //fans. 
  float pwmPercentage = 0.000336538*(pow((inverterMaxTemp),3))+27.31;
  pwmtemp = (pwmPercentage/100)*255;
  int pwmSignal = (int) pwmtemp;

  if (inverterMaxTemp <39) 
  {
    analogWrite(inverterFans, 120);
  }
  else if (inverterMaxTemp > 60 && 120 >= inverterMaxTemp)
  {
    analogWrite(inverterFans, 255);
  }
  else if(inverterMaxTemp > 120)
  {
    analogWrite(inverterFans, 179);
  }
  else
  {
    analogWrite(inverterFans, pwmSignal);
  }
}

void canInverter() 
{
  if (can.rxMsgLen > -1) 
  {
  switch (can.id)
    {
      case CAN_INVERTER_PASSTHROUGH:
        {
          //Motor Inverter Temperature:
          rxData[0] = can.rxData.bytes[0];
          can.rxMsgLen = -1;
          inverterMaxTemp = rxData[0];
        break;
        }
    }
  }
}

void fanControlInverter()
{
  float pwmtemp;
  //this line maps the temperature of the motor controller onto a value from 0-256 to be outputted to the
  //fans. 
  float pwmPercentage = 0.000336538*(pow((inverterMaxTemp),3))+27.31;
  pwmtemp = (pwmPercentage/100)*255;
  int pwmSignal = (int) pwmtemp;

  Serial1.print("the pwmsignal is: ");
  Serial1.println(pwmSignal);
  if (inverterMaxTemp <39) 
  {
    Serial1.println("Inverter below 39");
    analogWrite(inverterFans, 120);
  }
  else if (inverterMaxTemp > 60 && 120 >= inverterMaxTemp)
  {
    Serial1.println("Inverter between 60-120");
    analogWrite(inverterFans, 255);
  }
  else if(inverterMaxTemp > 120)
  {
    
    analogWrite(inverterFans, 179);
  }
  else
  {
    analogWrite(inverterFans, pwmSignal);
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
  pinMode(PA1, OUTPUT);
  //pinMode(leftFanControlPin, OUTPUT);
  //pinMode(rightFanControlPin, OUTPUT);
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
  Ticker.attach(canTX_Analog2,INTERVAL_ERROR_WARNING_LOW_PRIORITY);

  switch (UCM_NUMBER)
  {
      case 1:
        Ticker.attach(FlowSensor_Measure, 1000);
        break;
      case 2:
        Ticker.attach(FlowSensor_Measure, 1000);
        break;
      case 3:
        pinMode(leftFanControlPin, OUTPUT);
        pinMode(rightFanControlPin, OUTPUT);
        break;
      case 4:
        //Ticker.attach(FlowSensor_Measure, 1000);
        pinMode(inverterFans, OUTPUT);
        pinMode(accumulatorFans, OUTPUT);
        break;
      case 5:
        // 
        break;
   }
}

/*--------------------------------------------------------------------------- 
`								MAIN LOOP 
---------------------------------------------------------------------------*/

void loop()
{
  //needs to be called before the switch case statement so that it is always checked whether or not the HV is active.
  greenLoopActive();
  
  switch (UCM_NUMBER)
  {
    case 1:
      break;
      
    case 2:
      break;
      
    case 3:
      updateAnalog();
      //FlowSensorProcess();
      
      if(canHvActive == 1)
      {
        canMotorTemp();
        sidepodFanControlLHS();
        canMotorTemp();
        motorPumpControlLHS();
      }
      break;
      
    case 4:
      updateAnalog();
      //FlowSensorProcess();
      
      if(canHvActive == 1)
      {
        canMotorTemp();
        sidepodFanControlRHS();
        canMotorTemp();
        motorPumpControlRHS();
      }
      break; 
      
    case 5:
      updateAnalog();
      
      if(canHvActive == 1)
      {
        canAccumulator();
        fanControlAccumulator();
        canInverter();
        fanControlInverter();
      }
    break;
    
  }
}
