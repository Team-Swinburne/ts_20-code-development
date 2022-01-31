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

uint16_t motor_highest_temp = 0;
uint16_t rineheart_highest_temp = 0;
float max_accum_temp = 0;
uint16_t rineheart_voltage = 0;

/*---------------------------------------------------------------------------
  `								FUNCTIONS
  ---------------------------------------------------------------------------*/
void TX_Debug() {
  debug.bytes[0] = bDcOn;
  debug.bytes[1] = bInverterOn;
  debug.bytes[2] = bEnable;

  can.transmit(0x100,
               debug.bytes,
               debug.len);
}

// Actual value 1
void TX_actualValue1() {
  /*can.transmit(CAN_INV_ACTUAL_VALUE1,
  			actualValue1.bytes,
  			actualValue1.len);
    digitalToggle(PC13);
    delay(5);
    can.transmit(CAN_INV1_ACTUAL_VALUE1,
       actualValue1.bytes,
       actualValue1.len);*/
}

// Can receive interupt service routine
void canISR() {
  can.rxMsgLen = can.receive(can.id, can.fltIdx, can.rxData.bytes);
}

/*---------------------------------------------------------------------------
  `								DAEMONS
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

          rineheart_highest_temp = rxData[0] | (rxData[1] << 8);
          //Serial1.print("Motor Controller Temperature: ");
          //Serial1.println(rineheart_highest_temp);

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

          motor_highest_temp = rxData[4] | (rxData[5] << 8);
          //Serial1.print("Motor Temperature: ");
          //Serial1.println(motor_highest_temp);
        }
      case RMS_VOLTAGE_INFO:
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

          rineheart_voltage = (rxData[0] | (rxData[1] << 8))/10;
          //Serial1.print("Motor Controller Voltage: ");
          //Serial1.println(rineheart_voltage);
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

          max_accum_temp = (float)rxData[1];
          //Serial1.print("Accumulator Temperature: ");
          //Serial1.println(max_accum_temp);
      }
    }
  }
}

void telemTransmitHeartbeat() {
  Serial1.println("Hello World ");
}

void TX_Serial(){
  //Motor Controller Temp
  Serial1.print("Motor Controller Temperature: ");
  Serial1.println(rineheart_highest_temp);
  //Motor Temp
  Serial1.print("Motor Temperature: ");
  Serial1.println(motor_highest_temp);
  //Motor Controller Voltage
  Serial1.print("Motor Controller Voltage: ");
  Serial1.println(rineheart_voltage);
  //Accumulator Temperature
  Serial1.print("Accumulator Temperature: ");
  Serial1.println(max_accum_temp);
}

/*---------------------------------------------------------------------------
  `								SETUP
  ---------------------------------------------------------------------------*/

void setup()
{
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
  Serial1.begin(57600);
  Ticker.attach(TX_Serial, 500);
}

/*---------------------------------------------------------------------------
  `								MAIN LOOP
  ---------------------------------------------------------------------------*/

void loop()
{
  canRX();
  //telemTransmitHeartbeat();
}
