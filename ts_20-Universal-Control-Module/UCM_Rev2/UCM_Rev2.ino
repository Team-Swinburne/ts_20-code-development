/*  TEAM SWINBURNE - TS20
    UNIVERSAL CONTROL MODULE
    NAM TRAN - Revision 2
*/

//-----------------------------------------------
// Libraries
//-----------------------------------------------
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <eXoCAN.h>

//-----------------------------------------------
// INTERFACES
//-----------------------------------------------

// I2C Interface
Adafruit_ADS1115 ads;
#define Serial1_UART_INSTANCE    1 //ex: 2 for Serial2 (USART2)
#define PIN_Serial1_RX           PA10
#define PIN_Serial1_TX           PA9

// CANBus Interface
eXoCAN can;

// CANBus Adresses
#define UCM_HEARTBEAT_ID             0x511        //UCM_FL_HEARTBEAT_ID
#define UCM_DATA_ID                  0x512        //UCM_FL_DATA_ID
#define UCM_TEST                     0x571        //Test CAN Receive

// CANBus Intervals (ms)
#define HEARTRATE                    1000         
#define CAN_BROADCAST_INTERVAL       50           

//-----------------------------------------------
// GPIO
//-----------------------------------------------
#define pin_D1                          PB12   // Digital Input 1
#define pin_D2                          PB13   // Digital Input 2
#define pin_Driver1                     PB15  // Driver 1 (24V)
#define pin_Driver2                     PB14  // Driver 2 (24V)
#define pin_PWM_Driver1                 PB0   // PWM Driver 1 (5V)
#define pin_PWM_Driver2                 PB1   // PWM Driver 2 (5V)

//-----------------------------------------------
// Structs, typedef and enum
//-----------------------------------------------
typedef void (*fpointer)();
struct txFrame
{
  uint8_t len = 8;
  uint8_t bytes[8] = {0};
};

//-----------------------------------------------
// Globals
//-----------------------------------------------
int cnt = 0;
txFrame heartbeatFrame {.len = 6},
        errorFrame,
        digitalFrame1,
        analog1Frame1;


//-----------------------------------------------
// Classes
//-----------------------------------------------
// Ticker class, only support 5 objects at most
class Ticker {
private:
  fpointer callback;
  uint32_t freq_us;
  uint8_t timerIdx;

public:
  static uint8_t objectCount;
  Ticker (fpointer f, uint16_t freq_ms) {
    timerIdx = objectCount;
    callback = f;
    freq_us = freq_ms*1000;
    objectCount++;
  }
  void start();
};

// initialize static attribute objectCount
uint8_t Ticker::objectCount = 0; 

// Ticker.start method definition
void Ticker::start() {
  cnt = timerIdx;
  switch (timerIdx) {
    case 0: {
      HardwareTimer *Timer1 = new HardwareTimer(TIM1);
      Timer1->setOverflow(freq_us,MICROSEC_FORMAT); // 10 Hz
      Timer1->attachInterrupt(callback);
      Timer1->resume();
      break;
    }
    case 1: {
      HardwareTimer *Timer2 = new HardwareTimer(TIM2);
      Timer2->setOverflow(freq_us,MICROSEC_FORMAT); // 10 Hz
      Timer2->attachInterrupt(callback);
      Timer2->resume();
      break;
    }
    case 2: {
      HardwareTimer *Timer3 = new HardwareTimer(TIM3);
      Timer3->setOverflow(freq_us,MICROSEC_FORMAT); // 10 Hz
      Timer3->attachInterrupt(callback);
      Timer3->resume();
      break;
    }
    case 3: {
      HardwareTimer *Timer4 = new HardwareTimer(TIM4);
      Timer4->setOverflow(freq_us,MICROSEC_FORMAT); // 10 Hz
      Timer4->attachInterrupt(callback);
      Timer4->resume();
      break;
    }
  }
}

void heartbeat()
{
  static uint8_t heartbeat_counter = 0;
  uint8_t txData[8];
  txData[0] = cnt;
  txData[1] = heartbeat_counter;
  can.transmit(UCM_HEARTBEAT_ID, txData, 2);
  //Serial.println(heartbeat_counter);
  heartbeat_counter++;
  digitalToggle(PC13);
  
}

void testTX()
{
  uint8_t txData[2];
  txData[0] = cnt;
  txData[1] = 50;
  can.transmit(UCM_DATA_ID, txData, 2);

  delayMicroseconds(125);

  txData[0] = 100;
  txData[1] = 100;
  can.transmit(0x500, txData, 2);

}

Ticker hearbeat_ticker(heartbeat,1000);
Ticker test_ticker(testTX,100);

void setup()
{
  Serial1.begin(9600);
  Serial1.print("Start");

  pinMode(PC13, OUTPUT);

  // Start I2C communication with the ADC
  ads.begin(0x49);

  // Initiallising CAN
  can.begin(STD_ID_LEN, BR500K, PORTB_8_9_XCVR);   //11 Bit Id, 500Kbps
  can.filterMask16Init(0, UCM_TEST, 0x7ff);

  //test_callback(heartbeat);
  
  hearbeat_ticker.start();
  test_ticker.start();
}


void loop()
{
  /* Nothing to do all is done by hardware. Even no interrupt required. */
}