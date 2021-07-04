//    TEAM SWINBURNE - TS20
//    UNIVERSAL CONTROL MODULE

#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <eXoCAN.h>
#include <Ticker.h>

// Serial Declarations
Adafruit_ADS1115 ads;
#define Serial1_UART_INSTANCE    1 //ex: 2 for Serial2 (USART2)
#define PIN_Serial1_RX           PA10
#define PIN_Serial1_TX           PA9

// CANBus Adresses
int   UCM2_FR_HEARTBEAT_ID  =    0x521;
int   UCM2_FR_DATA_ID       =    0x522;
int   UCM_TEST              =    0x571;

// CANBus Intervals
#define    HEARTRATE               1000       //ms
#define    CAN_BROADCAST_INTERVAL  50         //ms


// UCM Pin Mapping
#define pin_D1              PB12   // Digital Input 1
#define pin_D2              PB13   // Digital Input 2
#define pin_Driver1         PB15  // Driver 1 (24V)
#define pin_Driver2         PB14  // Driver 2 (24V)
#define pin_PWM_Driver1     PB0   // PWM Driver 1 (5V)
#define pin_PWM_Driver2     PB1   // PWM Driver 2 (5V)


int DriveCondition1, DriveRequirment1 = 1, DriveValue1 = 0;
int DriveCondition2, DriveRequirment2 = 1, DriveValue2 = 0;
int DigIN1 =0;
int DigIN2 =0;
int PWM_Drive1 = 0, PWM_Drive2 = 0;
int16_t adc0, adc1, adc2, adc3;


int rxMsgID = 0x005;   // needed for rx filtering
int fltIdx,id;
uint8_t rxData[8];
uint32_t Heartbeat_Counter = 0;
uint8_t Heartbeat_State = 0;
int len = -1;
eXoCAN can;

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
  can.begin(STD_ID_LEN, BR500K, PORTB_8_9_XCVR);           // 11b IDs, 250k bit rate, Pins 8 and 9 with a transceiver chip
  can.filterMask16Init(0, UCM_TEST, 0x7ff);                // filter out every message exept for UCM_TEST
  can.attachInterrupt(canISR);
  
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
}
void CANRecieve(){
  if (len > -1){
    Serial1.print("Recieving...");
    Serial1.println(rxData[0]);
    DriveCondition1 = rxData[0];
    DriveCondition2 = rxData[1];
    PWM_Drive1 = rxData[2];
    PWM_Drive2 = rxData[3];
    len = -1;
  }
}

// Transmit Hearbeat ----------------------------------//
void heartbeat_tx(){
  Heartbeat_Counter = Heartbeat_Counter + 1;
  uint8_t txData[2];
  txData[0] = Heartbeat_State;
  txData[1] = Heartbeat_Counter;
  can.transmit(UCM2_FR_HEARTBEAT_ID, txData, 2);
  digitalToggle(PC13);
  if (Heartbeat_Counter == 256){
    Heartbeat_Counter = 0; 
  };                 
}
  // Transmit data ----------------------------------//
void data_tx(){
  uint8_t txData[8];
  txData[0] = PWM_Drive1;
  txData[1] = PWM_Drive2;
  can.transmit(UCM2_FR_DATA_ID, txData, 8);
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
  Serial1.print("Digital Input 1: ");  Serial1.println(DigIN1); 
  Serial1.print("Digital Input 2: ");  Serial1.println(DigIN2); 

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
  Serial1.print("Drive Cond 1: ");  Serial1.print(DriveCondition1,DEC);
  Serial1.print("    Drive Cond 2: ");  Serial1.println(DriveCondition2,DEC);
  Serial1.println(" ");
  Serial1.print("Drive PWM 1: ");  Serial1.print(PWM_Drive1,DEC);
  Serial1.print("    Drive PWM 2: ");  Serial1.println(PWM_Drive2,DEC);
  Serial1.println(" ");
}
