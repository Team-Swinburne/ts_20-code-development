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
  `								LIBRARIES
  ---------------------------------------------------------------------------*/

#include <eXoCAN.h>
#include "TickerInterrupt.h"
#include "can_addresses.h"

/*---------------------------------------------------------------------------
  `								INTERFACES
  ---------------------------------------------------------------------------*/

// I2C Interface
#define Serial1_UART_INSTANCE    1 //ex: 2 for Serial2 (USART2)
#define PIN_SERIAL1_RX           PA10
#define PIN_SERIAL1_TX           PA9

// CANBus Interface
eXoCAN can;

#define MOTEC_SET_POINT1 0x213
#define MOTEC_SET_POINT2 0x233

/*---------------------------------------------------------------------------
  `								GLOBALS
  ---------------------------------------------------------------------------*/
struct msgFrame
{
  uint8_t len = 8;
  uint8_t bytes[8] = {0};
};

static msgFrame	actualValue1, actualValue2, debug;

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
  `								FUNCTIONS
  ---------------------------------------------------------------------------*/
// Can receive interupt service routine
void canISR() {
  can.rxMsgLen = can.receive(can.id, can.fltIdx, can.rxData.bytes);
}

/*---------------------------------------------------------------------------
  `								DAEMONS
  ---------------------------------------------------------------------------*/

// CAN Receive function, use map if neccessary
void canRX() {
  if (can.rxMsgLen > -1) {
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
          txBuffer.fields.payload.motorControllerTemperature = (uint8_t)rineheart_highest_temp;
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
          txBuffer.fields.payload.motorTemperature = (uint8_t)motor_highest_temp;
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
          txBuffer.fields.payload.accumulatorVoltage[0] = (uint8_t)rxData[0];
          txBuffer.fields.payload.accumulatorVoltage[1] = (uint8_t)rxData[1];
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
          txBuffer.fields.payload.accumulatorTemperature = (uint8_t)rxData[2];
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
          txBuffer.fields.payload.brakePosition = (uint8_t)rxData[2];
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
          txBuffer.fields.payload.throttlePosition = (uint8_t)rxData[2];
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

void TX_Serial(){
  for (int i=0; i<10; i++) {
    Serial1.write(txBuffer.data[i]);
  }
}

/*---------------------------------------------------------------------------
  `								SETUP
  ---------------------------------------------------------------------------*/

void setup()
{
  Serial1.begin(57600);
  //delay(2000);
  pinMode(PC13, OUTPUT);
  // Initiallising CAN
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
 
  Ticker.attach(TX_Serial, 20); 
  SetData();
}

/*---------------------------------------------------------------------------
  `								MAIN LOOP
  ---------------------------------------------------------------------------*/

void loop()
{
  canRX();
  //Serial1.println("Hello");
  //telemTransmitHeartbeat();
}