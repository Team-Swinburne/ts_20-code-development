// TEAM SWINBURNE - UNIVERSAL CONTROL MODULE - HARDWARE REVISION 0
// NAM TRAN
// REVISION 0 - MOCK INVERTER (24/06/2021)

/***************************************************************************
    inverter1.cpp

    INTRO
    This is a code built to mock the CAN signal of the motor controllers, this case it is 1 and 3 on CAN 2.

    NOTE!: CAN signal format shown in here is Little endidan

    Revision     Date          Comments
    --------   ----------     ------------
    0.0        20/10/2021     Initial coding

****************************************************************************/

/*---------------------------------------------------------------------------
  `                LIBRARIES
  ---------------------------------------------------------------------------*/

#include <eXoCAN.h>
#include "TickerInterrupt.h"
#include "can_addresses.h"

/*---------------------------------------------------------------------------
  `               INTERFACES
  ---------------------------------------------------------------------------*/

// I2C Interface
#define Serial1_UART_INSTANCE    1 //ex: 2 for Serial2 (USART2)
#define PIN_SERIAL1_RX           PA10
#define PIN_SERIAL1_TX           PA9


// CANBus Interface
eXoCAN can;

#define MOTEC_SET_POINT1 0x213
#define MOTEC_SET_POINT2 0x233

#define fanControlPin PB1

/*---------------------------------------------------------------------------
  `               GLOBALS
  ---------------------------------------------------------------------------*/
struct msgFrame
{
  uint8_t len = 8;
  uint8_t bytes[8] = {0};
};
float tArray[4];

static msgFrame actualValue1, actualValue2, debug;

uint8_t rxData[8];
bool bInverterOn, bDcOn, bEnable = false;

uint8_t msb = 0, lsb = 0;
volatile static int motor_highest_temp = 0;
volatile static int rineheart_highest_temp = 0;
volatile static float max_accum_temp = 0;
volatile static int rineheart_voltage = 0;
volatile static int brake = 0;
volatile static uint8_t telemHeartbeat = 0;

struct PayLoadInfo
{
   uint8_t brakePosition;
   uint8_t throttlePosition;
   uint8_t motorControllerTemperature;
   uint8_t motorTemperature;
   uint8_t accumulatorTemperature;
   uint8_t accumulatorVoltage[2];  
};
struct SerialFields
{
  uint8_t startFlag[2];
  uint8_t packetLength;
  PayLoadInfo payload;
};
union SerialData
{
  SerialFields fields;
  uint8_t data[10];
};

SerialData txBuffer;

/*---------------------------------------------------------------------------
  `               FUNCTIONS
  ---------------------------------------------------------------------------*/
// Can receive interupt service routine
void canISR() {
  can.rxMsgLen = can.receive(can.id, can.fltIdx, can.rxData.bytes);
}

/*---------------------------------------------------------------------------
  `               DAEMONS
  ---------------------------------------------------------------------------*/

// CAN Receive function, use map if neccessary
void canRX() {
  //digitalToggle(PC13);
  //Serial1.println("CAN RX Function");
  if (can.rxMsgLen > -1) {
    //Serial1.println("Should be receiving");
    /*if (can.id == CAN_INV_ACTUAL_VALUE1) {
        rxData[0] = can.rxData.bytes[0];
        rxData[1] = can.rxData.bytes[1];
        rxData[2] = can.rxData.bytes[2];
        rxData[3] = can.rxData.bytes[3];
        rxData[4] = can.rxData.bytes[4];
        rxData[5] = can.rxData.bytes[5];
        rxData[6] = can.rxData.bytes[6];
        rxData[7] = can.rxData.bytes[7];
        can.rxMsgLen = -1;
        digitalToggle(PC13);

        Serial1.println("Frame Start");
        for (int i = 0; i < 8; i++){
          Serial1.print(rxData[i]);
        }
        Serial1.println("Frame End");

        //bDcOn = 1
        if (rxData[6] == 0x40) {
          // bQuitDcOn
          actualValue1.bytes[STATUS2] = 0b10011000 ; //bit 12 (offset 11) to 1
          bDcOn = true;
        }
        else {
          bDcOn = false;
        }

        if (rxData[6] == 0xE0) {
            bInverterOn = true;
            actualValue1.bytes[STATUS2] = 0b10011110;
          }
          else {
            bInverterOn = false;
         }
      }*/

    switch (can.id)
    {
      case RMS_TEMPERATURE_SET_2:
        {
          //Motor Controller Temperature:
          rxData[0] = can.rxData.bytes[0];
          rxData[1] = can.rxData.bytes[1]; 
          rxData[2] = can.rxData.bytes[2];
          rxData[3] = can.rxData.bytes[3];
          rxData[4] = can.rxData.bytes[4];
          rxData[5] = can.rxData.bytes[5];
          rxData[6] = can.rxData.bytes[6];
          rxData[7] = can.rxData.bytes[7];
          can.rxMsgLen = -1;
          
          rineheart_highest_temp = (rxData[0] | (rxData[1] << 8))/10;
          txBuffer.fields.payload.motorControllerTemperature = rineheart_highest_temp;
          //Serial1.print("Motor Controller Temperature: ");
          //Serial1.println(rineheart_highest_temp);
          
        break;
        }
      case RMS_TEMPERATURE_SET_3:
        {
          //Motor Temperature:
          rxData[0] = can.rxData.bytes[0];
          rxData[1] = can.rxData.bytes[1]; 
          rxData[2] = can.rxData.bytes[2];
          rxData[3] = can.rxData.bytes[3];
          rxData[4] = can.rxData.bytes[4];
          rxData[5] = can.rxData.bytes[5];
          rxData[6] = can.rxData.bytes[6];
          rxData[7] = can.rxData.bytes[7];
          can.rxMsgLen = -1;

          motor_highest_temp = (rxData[4] | (rxData[5] << 8 )/10);
          txBuffer.fields.payload.motorTemperature = motor_highest_temp;
          //Serial1.print("Motor Temperature: ");
          //Serial1.println(motor_highest_temp);
        break;
        }
        //accumulator VOLTAGE
      case ACCUMULATOR_VOLTAGE:
        {
          rxData[0] = can.rxData.bytes[0];
          rxData[1] = can.rxData.bytes[1]; 
          rxData[2] = can.rxData.bytes[2];
          rxData[3] = can.rxData.bytes[3];
          rxData[4] = can.rxData.bytes[4];
          rxData[5] = can.rxData.bytes[5];
          rxData[6] = can.rxData.bytes[6];
          rxData[7] = can.rxData.bytes[7];
          can.rxMsgLen = -1;
          
          //rineheart_voltage = (rxData[0] | (rxData[1] << 8))/10;
          txBuffer.fields.payload.accumulatorVoltage[0] = rxData[0];
          txBuffer.fields.payload.accumulatorVoltage[1] = rxData[1];
          //
          //Serial1.print("Motor Controller Voltage: ");
          //Serial1.println(rineheart_voltage);
        break;
        }
      case ACCUMULATOR_TEMP:
        {
          rxData[0] = can.rxData.bytes[0];
          rxData[1] = can.rxData.bytes[1]; 
          rxData[2] = can.rxData.bytes[2];
          rxData[3] = can.rxData.bytes[3];
          rxData[4] = can.rxData.bytes[4];
          rxData[5] = can.rxData.bytes[5];
          rxData[6] = can.rxData.bytes[6];
          rxData[7] = can.rxData.bytes[7];
          can.rxMsgLen = -1;
          txBuffer.fields.payload.accumulatorTemperature = rxData[2];
          //max_accum_temp = (float)rxData[1];
          //Serial1.print("Accumulator Temperature: ");
          //Serial1.println(max_accum_temp);
      break;
      }
      case BRAKE_POSITION:
        {
          rxData[0] = can.rxData.bytes[0];
          rxData[1] = can.rxData.bytes[1]; 
          rxData[2] = can.rxData.bytes[2];
          rxData[3] = can.rxData.bytes[3];
          rxData[4] = can.rxData.bytes[4];
          rxData[5] = can.rxData.bytes[5];
          rxData[6] = can.rxData.bytes[6];
          rxData[7] = can.rxData.bytes[7];
          can.rxMsgLen = -1;
          txBuffer.fields.payload.brakePosition = rxData[2];
      break;
      }
      case THROTTLE_POSITION:
      {
          rxData[0] = can.rxData.bytes[0];
          rxData[1] = can.rxData.bytes[1]; 
          rxData[2] = can.rxData.bytes[2];
          rxData[3] = can.rxData.bytes[3];
          rxData[4] = can.rxData.bytes[4];
          rxData[5] = can.rxData.bytes[5];
          rxData[6] = can.rxData.bytes[6];
          rxData[7] = can.rxData.bytes[7];
          can.rxMsgLen = -1;
          txBuffer.fields.payload.throttlePosition = rxData[2];
          break;
      }
    }
  }
}

void SetData()
{
  txBuffer.fields.startFlag[0] = 0x19;
  txBuffer.fields.startFlag[1] = 0x94;
  txBuffer.fields.packetLength = 7;
}
/*
void SendData()
{
  Serial.write(txBuffer.fields.startFlag[0]);
  Serial.write(txBuffer.fields.startFlag[1]);
  Serial.write(txBuffer.fields.packetLength);
  Serial.write(txBuffer.fields.payload.brakePosition);
  Serial.write(txBuffer.fields.payload.throttlePosition);
  Serial.write(txBuffer.fields.payload.motorControllerTemperature);
  Serial.write(txBuffer.fields.payload.motorTemperature);
  Serial.write(txBuffer.fields.payload.accumulatorTemperature);
  Serial.write(txBuffer.fields.payload.accumulatorVoltage[0]);
  Serial.write(txBuffer.fields.payload.accumulatorVoltage[1]);
}
*/

void telemTransmitHeartbeat() {
  Serial1.println("Hello World ");
}

void TX_Can(){
  debug.bytes[0] = 1;
    debug.bytes[1] = 1;
    debug.bytes[2] = 1;
    
    can.transmit(0x350, 
        debug.bytes, 
        8);
  }
//  Serial1.print(txBuffer.fields.startFlag[0]);
//  Serial1.print(txBuffer.fields.startFlag[1]);
//  Serial1.print(txBuffer.fields.packetLength);
//  Serial1.print(txBuffer.fields.payload.brakePosition);
//  Serial1.print(txBuffer.fields.payload.throttlePosition);
//  Serial1.print(txBuffer.fields.payload.motorControllerTemperature);
//  Serial1.print(txBuffer.fields.payload.motorTemperature);
//  Serial1.print(txBuffer.fields.payload.accumulatorTemperature);
//  Serial1.print(txBuffer.fields.payload.accumulatorVoltage[0]);
//  Serial1.print(txBuffer.fields.payload.accumulatorVoltage[1]);
  /*
  //Heartbeat
  Serial1.print("HB: ");
  if (telemHeartbeat > 255){
    telemHeartbeat = 0;
  }
  Serial1.print(telemHeartbeat);
  telemHeartbeat++;
  Serial1.print("        ");
  
  //Motor Controller Temp
  Serial1.print("RH Temp: ");
  Serial1.print(rineheart_highest_temp);
  Serial1.print("        "); 
  //Motor Temp
  Serial1.print("Emrax Temp: ");
  Serial1.print(motor_highest_temp);
  Serial1.print("        ");
  //Motor Controller Voltage
  Serial1.print("RH Volts: ");
  Serial1.print(rineheart_voltage);
  Serial1.print("        "); //Motor Temp
  //Accumulator Temperature
  Serial1.print("Eddie Temp: ");
  Serial1.println(max_accum_temp);
  */
  //Serial1.println(brake);

//Motor Inverter Messages off CAN
void canInverter() {
  if (can.rxMsgLen > -1) {
  switch (can.id)
    {
      case CAN_INV_ACTUAL_VALUE2:
        {
          //Motor Inverter Temperature:
          rxData[0] = can.rxData.bytes[0];
          rxData[1] = can.rxData.bytes[1]; 
          rxData[2] = can.rxData.bytes[2];
          rxData[3] = can.rxData.bytes[3];
          rxData[4] = can.rxData.bytes[4];
          rxData[5] = can.rxData.bytes[5];
          rxData[6] = can.rxData.bytes[6];
          rxData[7] = can.rxData.bytes[7];
          can.rxMsgLen = -1;
          //Serial1.print("rxData[2]: ");
          //Serial1.println(rxData[2]);
          tArray[0] = (256*rxData[3]+rxData[2])/10;
        break;
        }
        /*
      case inverterTemp2:
        {
          rxData[2] = can.rxData.bytes[2];
          rxData[3] = can.rxData.bytes[3];
          can.rxMsgLen = -1;
          tArray[1] = (256*rxData[3] + rxData[2])/10;
          break;
        }
      case inverterTemp3:
        {
          rxData[2] = can.rxData.bytes[2];
          rxData[3] = can.rxData.bytes[3];
          can.rxMsgLen = -1;
          tArray[2] = (256*rxData[3] + rxData[2])/10;
          break;
        }
      case inverterTemp4:
      {
        rxData[2] = can.rxData.bytes[2];
        rxData[3] = can.rxData.bytes[3];
        can.rxMsgLen = -1;
        tArray[3] = (256*rxData[3] + rxData[2])/10;
        break;
      }
      */
    }
  }
}

void highestTemp()
{
  int minTempId;
  int one, two;
  for(int i =0; i<3;i++)
  {
    minTempId = i;
    for(int j = i+1; j<4;j++)
    {
      if(tArray[j] < tArray[minTempId])
      {
        minTempId = j;
      }
      one = tArray[minTempId];
      two = tArray[i];
      tArray[minTempId] = two;
      tArray[i] = one;
    }
  }
}

void fanControl()
{
  float pwmtemp;
  Serial1.print("tArray[0]: ");
  Serial1.println(tArray[0]);
  //this line maps the temperature of the motor controller onto a value from 0-256 to be outputted to the
  //fans. The 
  float pwmPercentage = 0.000336538*(pow((tArray[0]),3))+27.31;
  Serial1.print("pwmPercentage: ");
  Serial1.print(pwmPercentage);
  pwmtemp = (pwmPercentage/100)*255;
  int pwmSignal = (int) pwmtemp;

  Serial1.print("pwmSignal: ");
  Serial1.println(pwmSignal);
  
  if (tArray[0] <39) 
  {
    analogWrite(fanControlPin, 120);
  }
  else if ( tArray[0] > 60 && 120 >= tArray[0])
  {
    analogWrite(fanControlPin, 255);
  }
  else if(tArray[0] > 120)
  {
    analogWrite(fanControlPin, 179);
  }
  else
  {
    analogWrite(fanControlPin, pwmSignal);
  }
}

/*---------------------------------------------------------------------------
  `               SETUP
  ---------------------------------------------------------------------------*/

void setup()
{
  Serial1.begin(57600);
  //delay(2000);
  pinMode(PC13, OUTPUT);
  // Initiallising CAN
  pinMode(fanControlPin, OUTPUT);
  analogWriteFrequency(25000);
  can.begin(STD_ID_LEN, CANBUS_FREQUENCY, PORTB_8_9_XCVR);   //11 Bit Id, 500Kbps
  //can.filterMask16Init(0, MOTEC_SET_POINT, 0x7ff);
  can.attachInterrupt(canISR);

  actualValue1.bytes[STATUS1] = 0b00000000; //reserve
  actualValue1.bytes[STATUS2] = 0b10000000; //bit 9 (offset 8), bSystemReady = 1
  // Start ticker and attach callback to it
  TickerInterrupt Ticker(TIM2, 1);
  Ticker.start();
  //Ticker.attach(TX_actualValue1, 500);
  //Ticker.attach(TX_Debug, 500);
 
  //Ticker.attach(TX_Can, 50); 
  SetData();
}

/*---------------------------------------------------------------------------
  `               MAIN LOOP
  ---------------------------------------------------------------------------*/

void loop()
{
  Serial1.println(tArray[3]);
  canInverter();
  fanControl();
  //canRX();
  //Serial1.println("Hello");
  //telemTransmitHeartbeat();
}
