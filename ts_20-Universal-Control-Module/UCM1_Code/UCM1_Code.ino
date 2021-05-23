//  TEAM SWINBURNE - TS20
//  UNIVERSAL CONTROL MODULE #1

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

// UART Interface
#define Serial_UART_INSTANCE     1 //ex: 2 for Serial2 (USART2)
#define PIN_Serial1_RX           PA10
#define PIN_Serial1_TX           PA9

// I2C Interface
Adafruit_ADS1115 ads;

// CANBus Interface
eXoCAN can(STD_ID_LEN, BR500K, PORTB_8_9_XCVR); //11 Bit Indentifier, 500Kbps

// CANBus Adresses
#define UCM_HEARTBEAT_ID                0x511 //UCM_FL_HEARTBEAT_ID
#define UCM_DATA_ID                     0x512 //UCM_FL_DATA_ID
#define UCM_TEST                        0x309 //Test CAN Receive

// CANBus Intervals
#define    HEARTRATE                    1000       //ms
#define    CAN_BROADCAST_INTERVAL       50         //ms


// UCM Pin Mapping
#define pin_D1                          PB12   // Digital Input 1
#define pin_D2                          PB13   // Digital Input 2
#define pin_Driver1                     PB15  // Driver 1 (24V)
#define pin_Driver2                     PB14  // Driver 2 (24V)
#define pin_PWM_Driver1                 PB0   // PWM Driver 1 (5V)
#define pin_PWM_Driver2                 PB1   // PWM Driver 2 (5V)

//-----------------------------------------------
// Const
//-----------------------------------------------

//-----------------------------------------------
// Globals
//-----------------------------------------------

uint8_t  heartbeat_state                 = 0;
uint8_t	 heartbeat_counter 			         = 0;

int DriveCondition1, DriveRequirment1 = 1, DriveValue1 = 0;
int DriveCondition2, DriveRequirment2 = 1, DriveValue2 = 0;
int DigIN1 =0;
int DigIN2 =0;
int PWM_Drive1 = 0, PWM_Drive2 = 0;
int16_t adc0, adc1, adc2, adc3;


int fltIdx,id;
uint8_t rxData[8];
int len = -1;
char cBuff[50];

void heartbeat_tx();
void data_tx();
void digitalupdate();
void I2C();
void CANRecieve();
void Driver();
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
  
  // Configuring the Digital Input Pins.
  pinMode(pin_D1, INPUT);
  pinMode(pin_D2, INPUT);

  // Configuring Driver Pins.
  pinMode(pin_Driver1, OUTPUT);
  pinMode(pin_Driver2, OUTPUT);
  pinMode(pin_PWM_Driver1, OUTPUT);
  pinMode(pin_PWM_Driver2, OUTPUT);

  // Start I2C communication with the ADC
  ads.begin(0x49);
  
  // Initiallising CAN
  can.begin();           // 11b IDs, 250k bit rate, Pins 8 and 9 with a transceiver chip
  can.filterMask16Init(0, UCM_TEST, 0x7ff);                // filter out every message exept for UCM_TEST
  
  // Initiallising Serial1
  Serial1.begin(250000);
  sprintf(cBuff,"Starting ts_20 Universal Controller Module 1 - Front Left (STM32F103C8T6 32k) \
	\r\nCOMPILED: %s: %s\r\n",__DATE__, __TIME__);
  Serial1.print(cBuff);

  //Ticker Setup
  ticker_heartbeat.start();
  ticker_data_tx.start();

  can.attachInterrupt(canISR);
}

//--------------------------------------------------//
// Arduinos "Main" function
//--------------------------------------------------//
 void loop() {
  digitalupdate();
  I2C();
  CANRecieve();
  ticker_heartbeat.update();
  ticker_data_tx.update();
  Driver();
  SerialPRINT();
}

//--------------------------------------------------//
// This function reads all digital input values and stores them in variables.
//--------------------------------------------------//
void digitalupdate(){
  DigIN1 = digitalRead(pin_D1);  
  DigIN2 = digitalRead(pin_D2);
}

//--------------------------------------------------//
// This function reads all ADC values and stores them in variables.
//--------------------------------------------------//
void I2C() {
  adc0 = ads.readADC_SingleEnded(0);
  adc1 = ads.readADC_SingleEnded(1);
  adc2 = ads.readADC_SingleEnded(2);
  adc3 = ads.readADC_SingleEnded(3);
}

//--------------------------------------------------//
// CAN Message Recieve. 
//--------------------------------------------------//
void canISR(){
  len = can.receive(id, fltIdx, rxData);
  Serial1.print("Recieving...");
  digitalToggle(PC13);
}
void CANRecieve(){
  //canISR;
  if (len > -1){
    if (id == UCM_TEST){
      DriveCondition1 = rxData[0];
      DriveCondition2 = rxData[1];
      PWM_Drive1 = rxData[2];
      PWM_Drive2 = rxData[3];
      len = -1;
    }
  }
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
  txData[0] = PWM_Drive1;
  can.transmit(UCM_DATA_ID, txData, 8);
  //Serial1.println("Transmitting...");          
}


//--------------------------------------------------//
//Function to Control Drivers. Comment all that is not required.
// Only uncomment 1 option at a time per driver.
//--------------------------------------------------//
void Driver() {
// Conditional Output Driver (24V)
  if(DriveCondition1 == DriveRequirment1){
    digitalWrite(PB15, HIGH);
    DriveValue1 = 1;
  }
  else {
    digitalWrite(PB15, LOW);
    DriveValue1 = 0;
  }

  if(DriveCondition2 == DriveRequirment2){
    digitalWrite(PB14, HIGH);
    DriveValue2 = 1;
  }
  else {
    digitalWrite(PB14, LOW);
    DriveValue2 = 0;
  }

// PWM Output Driver.
//--------------------------------------------------//
  analogWrite(pin_PWM_Driver1,PWM_Drive1);
  analogWrite(pin_PWM_Driver2,PWM_Drive2);
//
}

//--------------------------------------------------//
// This Function Prints all values to Serial1
//--------------------------------------------------//
void SerialPRINT() {

  // //Digital Input 
  // Serial1.print("Digital Input 1: ");  Serial1.println(DigIN1); 
  // Serial1.print("Digital Input 2: ");  Serial1.println(DigIN2); 

  // //Analogue Input
  // Serial1.print("Analogue Input 0: "); Serial1.println(adc0);
  // Serial1.print("Analogue Input 1: "); Serial1.println(adc1);
  // Serial1.print("Analogue Input 2: "); Serial1.println(adc2);
  // Serial1.print("Analogue Input 3: "); Serial1.println(adc3);
  // Serial1.println(" ");
  // //Driver Value
  // Serial1.print("Driver 1: "); Serial1.println(DriveValue1);
  // Serial1.print("Driver 2: "); Serial1.println(DriveValue2);
  // delay(300);
  sprintf(cBuff,"Drive Cond 1: %d    Drive Cond 2: %d /r/n", DriveCondition1, DriveCondition2);
  Serial1.print(cBuff);
  sprintf(cBuff,"Drive PWM 1:  %d    Drive PWM 2: %d /r/n", PWM_Drive1, PWM_Drive2);
  Serial1.println(cBuff);
}
