//    TEAM SWINBURNE - TS20
//    UNIVERSAL CONTROL MODULE
//    CAN Test code, put anything you want to transmit on here

#include <Wire.h>
#include <eXoCAN.h>
#include <Ticker.h>

// Serial Declarations
#define Serial1_UART_INSTANCE    1 //ex: 2 for Serial2 (USART2)
#define PIN_Serial1_RX           PA10
#define PIN_Serial1_TX           PA9

// CANBus Adresses
int   UCM_HEARTBEAT_ID  =    0x570;
int   UCM_DATA_ID       =    0x571;

// CANBus Intervals
#define    HEARTRATE               1000       //ms
#define    CAN_BROADCAST_INTERVAL  50         //ms

int rxMsgID = 0x005;   // needed for rx filtering
int fltIdx;
uint8_t rxData[8];
uint32_t Heartbeat_Counter = 0;
uint8_t Heartbeat_State = 0;
eXoCAN can;
int dr = 0, pwm1 = 0, pwm2 = 0;

void heartbeat_tx();
void data_tx();
void CANRecieve();
void SerialPRINT();

//Ticker 
Ticker ticker_heartbeat(heartbeat_tx, HEARTRATE); 
Ticker ticker_data_tx(data_tx, CAN_BROADCAST_INTERVAL);

//--------------------------------------------------//
// the setup function runs once when you press reset or power the board
//--------------------------------------------------//
void setup() {
  
  // Debug LED.
  pinMode(PC13, OUTPUT);
  
  // Initiallising CAN
  can.begin(STD_ID_LEN, BR500K, PORTB_8_9_XCVR);           // 11b IDs, 250k bit rate, Pins 8 and 9 with a transceiver chip
  can.filterMask16Init(0, 0, 0x7ff, 0, 0);                 // filter bank 0, filter 0: don't pass any, flt 1: pass all msgs
  
  // Initiallising Serial1
  Serial1.begin(9600);

  //Ticker Setup
  ticker_heartbeat.start();
  ticker_data_tx.start();
}

//--------------------------------------------------//
// Arduinos "Main" function
//--------------------------------------------------//
 void loop() {
  CANRecieve();
  ticker_heartbeat.update();
  ticker_data_tx.update();
}
//--------------------------------------------------//
// CAN Message Recieve. 
//--------------------------------------------------//
void CANRecieve(){
  //int len = can.receive(UCM2_FR_OVERWRITE, fltIdx, rxData);
  //if (len > -1){
  //}
}

// Transmit Hearbeat ----------------------------------//
void heartbeat_tx(){
  Heartbeat_Counter = Heartbeat_Counter + 1;
  uint8_t txData[2];
  txData[0] = Heartbeat_State;
  txData[1] = Heartbeat_Counter;
  can.transmit(UCM_HEARTBEAT_ID, txData, 2);
  digitalToggle(PC13);
  if (Heartbeat_Counter == 256){
    Heartbeat_Counter = 0; 
  };                 
}
  // Transmit data ----------------------------------//
void data_tx(){
  uint8_t txData[8];
  if (dr == 0) {
    txData[0] = 1;
    txData[1] = 0;
    dr = 1;
  }
  else {
    txData[0] = 0;
    txData[1] = 1;
    dr = 0;  
  }
  pwm1 = pwm1 + 20;
  pwm2 = pwm2 - 20;
  if (pwm1 >= 255) {pwm1 = 0;}
  if (pwm2 <= 0) {pwm2 = 255;}
  txData[2] = 255;
  txData[3] = 255;
  can.transmit(UCM_DATA_ID, txData, 4);
  //Serial1.println("Transmitting...");          
}