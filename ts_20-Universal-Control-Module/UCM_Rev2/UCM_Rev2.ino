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
#include <Ticker.h>

//-----------------------------------------------
// INTERFACES
//-----------------------------------------------

// I2C Interface
Adafruit_ADS1115 ads;
#define Serial1_UART_INSTANCE    1 //ex: 2 for Serial2 (USART2)
#define PIN_Serial1_RX           PA10
#define PIN_Serial1_TX           PA9

// CANBus Interface
eXoCAN can;   //11 Bit Id, 500Kbps

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
// Structs and enum
//-----------------------------------------------
struct msgFrame
{
  uint8_t len = 0x08;
  MSG txMsg; // MSG is a union defined in exoCAN.h
};

enum msgName : uint8_t
{
  // Uncomment as needed
  Heartbeat,
  Error,
  Digital_1,
  //Digital_2,
  Analog_1,
  // Analog_2,
};
//-----------------------------------------------
// Globals
//-----------------------------------------------
msgFrame frames[4];

//-----------------------------------------------
// Classes
//-----------------------------------------------
class Heart {
public:
  uint8_t state = 0;
  static uint8_t counter;
  
  void start() {
    HardwareTimer *MyTim = new HardwareTimer(TIM1);

    MyTim->setOverflow(100000,MICROSEC_FORMAT); // 10 Hz
    MyTim->attachInterrupt(beat);
    MyTim->resume();  
  }

private:
  static void beat() {
    uint8_t txData[2];
    txData[0] = 0;
    txData[1] = counter;
    can.transmit(UCM_HEARTBEAT_ID, txData, 2);
    counter++;
    digitalToggle(PC13);
  }
};
// void tickerInit()
// {
//   HardwareTimer *MyTim = new HardwareTimer(TIM1);

//   MyTim->setOverflow(100000,MICROSEC_FORMAT); // 10 Hz
//   MyTim->attachInterrupt(heartbeat);
//   MyTim->resume();

// }

// void heartbeat()
// {
//   static uint8_t heartbeat_counter = 0;
//   uint8_t txData[2];
//   txData[0] = 0;
//   txData[1] = heartbeat_counter;
//   can.transmit(UCM_HEARTBEAT_ID, txData, 2);
//   //Serial.println(heartbeat_counter);
//   heartbeat_counter++;
//   digitalToggle(PC13);
// }
void setup()
{
  Serial1.begin(9600);
  Serial1.print("Start");

  pinMode(PC13, OUTPUT);

  // Start I2C communication with the ADC
  ads.begin(0x49);

  // Initiallising CAN
  can.begin(STD_ID_LEN, BR500K, PORTB_8_9_XCVR);
  can.filterMask16Init(0, UCM_TEST, 0x7ff);

  Heart hearbeat;
  hearbeat.start();
  //tickerInit();
}


void loop()
{
  /* Nothing to do all is done by hardware. Even no interrupt required. */
}