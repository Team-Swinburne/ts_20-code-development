// TEAM SWINBURNE - UNIVERSAL CONTROL MODULE - HARDWARE REVISION 0
// NAM TRAN
// REVISION 0 - MOCK INVERTER (24/06/2021)

/***************************************************************************
    inverter1.cpp

    INTRO
    This is a code built to mock the CAN signal of the motor controllers, this case it is 2 and 4 on CAN 3.

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

#define MOTEC_SET_POINT1 0x223
#define MOTEC_SET_POINT2 0x243

/*--------------------------------------------------------------------------- 
`								GLOBALS 
---------------------------------------------------------------------------*/
struct msgFrame
{
  uint8_t len = 8;
  uint8_t bytes[8] = {0};
};

static msgFrame	heartbeat;

uint8_t rxData[8];
/*--------------------------------------------------------------------------- 
`								FUNCTIONS 
---------------------------------------------------------------------------*/
// Actual value 1
void TX_Heartbeat() {
  heartbeat.len = 3;
	can.transmit(CAN_PRECHARGE_CONTROLLER_BASE_ADDRESS+TS_HEARTBEAT_ID, 
				heartbeat.bytes, 
				heartbeat.len);
	digitalToggle(PC13);
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
	if (can.rxMsgLen > -1) {
		if (can.id == CAN_MOTEC_THROTTLE_CONTROLLER_BASE_ADDRESS + TS_DIGITAL_1_ID) { 
      if (can.rxData.bytes[0] == 1) {
        heartbeat.bytes[0] = 4;
      }

      if (can.rxData.bytes[1] == 1 && heartbeat.bytes[0] == 4) {
        heartbeat.bytes[0] = 5;
      }
      
      can.rxMsgLen = -1;
	}
}
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
	
	// Start ticker and attach callback to it
	  TickerInterrupt Ticker(TIM2,1);
  	Ticker.start();
  	Ticker.attach(TX_Heartbeat,500);
}

/*--------------------------------------------------------------------------- 
`								MAIN LOOP 
---------------------------------------------------------------------------*/

void loop()
{
  canRX();
}
