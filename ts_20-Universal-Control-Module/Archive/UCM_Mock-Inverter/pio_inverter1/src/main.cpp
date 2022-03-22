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

****************************************************************************/

//LITTLE ENDIDAN
/*--------------------------------------------------------------------------- 
`								LIBRARIES 
---------------------------------------------------------------------------*/

#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <eXoCAN.h>
#include "TickerInterrupt.h"
#include "can_addresses.h"

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

#define MOTEC_SET_POINT 0x213

/*--------------------------------------------------------------------------- 
`								GLOBALS 
---------------------------------------------------------------------------*/
struct msgFrame
{
  uint8_t len = 8;
  uint8_t bytes[8] = {0};
};

static msgFrame	actualValue1, actualValue2,debug;

uint8_t rxData[8];
bool bInverterOn, bDcOn = false;
uint64_t bDcOn_time = 0;
/*--------------------------------------------------------------------------- 
`								FUNCTIONS 
---------------------------------------------------------------------------*/
void TX_Debug() {
    debug.bytes[0] = bDcOn;
    can.transmit(0x100, 
				debug.bytes, 
				debug.len);
	digitalToggle(PC13);
    
}
// Transmit hearbeat, letting the other pals know you're alive
void TX_actualValue1() {
	can.transmit(CAN_INV_ACTUAL_VALUE1, 
				actualValue1.bytes, 
				actualValue1.len);
	digitalToggle(PC13);
}

// Transmit digital message
// void TX_actualValue2() {
//   	can.transmit(CAN_UCM_BASE_ADDRESS+TS_DIGITAL_1_ID, 
//   				digitalFrame1.bytes, 
// 				digitalFrame1.len);
// }

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
		if (can.id == MOTEC_SET_POINT) {
      		rxData[0] = can.rxData.bytes[0];
      		rxData[1] = can.rxData.bytes[1];
			rxData[2] = can.rxData.bytes[2];
			rxData[3] = can.rxData.bytes[3];
      		can.rxMsgLen = -1;
		}
        if (rxData[1] & 0b01000000) {
					actualValue1.bytes[STATUS2] = 0b10010000; //bit 12 (offset 11) to 1

					//if bInverterOn = 1
					if (rxData[1] & 0b10000000) {
							actualValue1.bytes[STATUS2] = 0b10010010;
					}
			}
        //if bDcOn = 1
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
  	//can.attachInterrupt(canISR);
	
	  actualValue1.bytes[STATUS1] = 0; //reserve
    actualValue1.bytes[STATUS2] = 0b10000000; //bit 9 (offset 8), bSystemReady = 1
	// Start ticker and attach callback to it
	  TickerInterrupt Ticker(TIM2,1);
  	Ticker.start();
  	Ticker.attach(TX_actualValue1,500);
}

/*--------------------------------------------------------------------------- 
`								MAIN LOOP 
---------------------------------------------------------------------------*/

void loop()
{
  canRX();
  if (bDcOn && !bDcOn_time) {
      bDcOn_time = millis();
  }
}
