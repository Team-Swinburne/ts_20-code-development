//    TEAM SWINBURNE - TS20
//    BRAKE MODULE

#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <eXoCAN.h>
#include <Ticker.h>

// UCM Pin Mapping

//Declarations
Adafruit_ADS1115 ads;
#define Serial1_UART_INSTANCE    1 //ex: 2 for Serial2 (USART2)
#define PIN_Serial1_RX           PA10
#define PIN_Serial1_TX           PA9

// Intervals -------------------------------------
#define    HEARTRATE               1000       //ms
#define    CAN_BROADCAST_INTERVAL  50         //ms

int DriveCondition1, DriveRequirment1 = 1, DriveValue1 = 0;
int DriveCondition2, DriveRequirment2 = 1, DriveValue2 = 0;
int DigIN1 =0;
int DigIN2 =0;
int DigIN3 =0;
int DigIN4 =0;
int16_t adc0, adc1, adc2, adc3;

//CANBus Adresses
#define   UCM2_FR_HEARTBEAT_ID    0x521
#define   UCM2_FR_DATA_ID         0x522
#define   UCM2_FR_OVERWRITE       0x523

int rxMsgID = 0x005;   // needed for rx filtering
int fltIdx;
uint8_t rxbytes[8];
uint8_t rxData[8];
uint32_t Heartbeat_Counter = 0;
uint8_t Heartbeat_State = 0;
eXoCAN can;

void heartbeat_tx();
void data_tx();

//Ticker 
Ticker ticker_heartbeat(heartbeat_tx, HEARTRATE); 
Ticker ticker_data_tx(data_tx, CAN_BROADCAST_INTERVAL);

//--------------------------------------------------//
// the setup function runs once when you press reset or power the board
//--------------------------------------------------//
void setup() {
  
  // initialize digital pin PC_13 as an output.
  pinMode(PC13, OUTPUT);
  
  // Configuring the Digital Input Pins. Uncomment when required.
  pinMode(PB10, OUTPUT);
  pinMode(PB11, OUTPUT);
  pinMode(PB12, INPUT);
  pinMode(PB13, INPUT);

  // Configuring Driver Pins. Uncomment when required.
  pinMode(PB15, OUTPUT);
  pinMode(PB14, OUTPUT);

  // Start I2C communication with the ADC
  ads.begin(0x49);
  
  // Initiallising CAN
  can.begin(STD_ID_LEN, BR500K, PORTB_8_9_XCVR);           // 11b IDs, 250k bit rate, Pins 8 and 9 with a transceiver chip
  can.filterMask16Init(0, 0, 0x7ff, 0, 0);                 // filter bank 0, filter 0: don't pass any, flt 1: pass all msgs
  //  pinMode(PB0, OUTPUT);                                   // TX Indicator Pin
  //  pinMode(PB1, OUTPUT);                                   // RX Indicator Pin 
  
  // Initiallising Serial1
  Serial1.begin(9600);

  //Ticker Setup
  heartbeat_timer.start();
  data_tx_timer.start();
}


//--------------------------------------------------//
// Can Message Transmit.
//--------------------------------------------------//
 void CANTransmit(){

// Digital Input Value Fill ------------------------//
  txData[0]=DigIN1;
  txData[1]=DigIN2;
  txData[2]=DigIN3;
  txData[3]=DigIN4;

// Analogue Input Value Fill ----------------------//
  txData[4]=adc0;
  txData[5]=adc1;
  txData[6]=adc2;
  txData[7]=adc3;
  
};
//--------------------------------------------------//
// CAN Message Recieve. 
//--------------------------------------------------//
int id = 0x309;
char cBuff[20];
 void CANRecieve(){
  int len = can.receive(id, fltIdx, rxbytes);
    if (len > -1) // poll for rx
      {
      sprintf(cBuff, "RX @%02x #%d  %d\t", id, len, fltIdx);
      Serial1.print(cBuff);
      for (int j = 0; j < len; j++)
      {
        sprintf(cBuff, "%02x ", rxbytes[j]);
        Serial1.print(cBuff);
      }
      Serial1.println();
    }
 };


//--------------------------------------------------//
// This function reads all digital input values and stores them in variables.
//--------------------------------------------------//
void digitalupdate(){
  //DigIN1 = digitalRead(PB10);  
  //DigIN2 = digitalRead(PB11);
  DigIN3 = digitalRead(PB12);
  DigIN4 = digitalRead(PB13);
}

//--------------------------------------------------//
// This Function Prints all values to Serial1
//--------------------------------------------------//
void Serial1PRINT() {

  //Digital Input 
  Serial1.print("Digital Input 1: ");  Serial1.println(DigIN1); 
  Serial1.print("Digital Input 2: ");  Serial1.println(DigIN2); 
  Serial1.print("Digital Input 3: ");  Serial1.println(DigIN3); 
  Serial1.print("Digital Input 4: ");  Serial1.println(DigIN4); 
  //Analogue Input
  Serial1.print("Analogue Input 0: "); Serial1.println(adc0);
  Serial1.print("Analogue Input 1: "); Serial1.println(adc1);
  Serial1.print("Analogue Input 2: "); Serial1.println(adc2);
  Serial1.print("Analogue Input 3: "); Serial1.println(adc3);
  Serial1.println(" ");
  //Driver Value
  Serial1.print("Driver 1: "); Serial1.println(DriveValue1);
  Serial1.print("Driver 2: "); Serial1.println(DriveValue2);
  delay(300);
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
//Function to Control Drivers. Comment all that is not required.
// Only uncomment 1 option at a time per driver.
//--------------------------------------------------//
void Driver() {

//--------------------------------------------------//
// Set Output Driver
//--------------------------------------------------//
//--------------------------------------------------//
// Conditional Output Driver.
//--------------------------------------------------//
//  if(DriveCondition1 == DriveRequirment1){
//  digitalWrite(PB15, HIGH);
//  DriveValue1 = 1;
//  }
//  else {
//  digitalWrite(PB15, LOW);
//  DriveValue1 = 0;
//  }
//
//    if(DriveCondition2 == DriveRequirment2){
//  digitalWrite(PB14, HIGH);
//  DriveValue2 = 1;
//  }
//  else {
//  digitalWrite(PB14, LOW);
//  DriveValue2 = 0;
//  }

//--------------------------------------------------//
// Conditional Output Driver w/ PWM.
//--------------------------------------------------//
//    if(DriveCondition1 == DriveRequirment1){
//  digitalToggle(PB15);
//  DriveValue1 = 1;
//
//
//    if(DriveCondition2 == DriveRequirment2){
//  digitalWrite(PB14, HIGH);
//  DriveValue2 = 1;
//  }
//  else {S
//}
}
// Transmit Hearbeat ----------------------------------//
void heartbeat_tx(){    
          Heartbeat_Counter = Heartbeat_Counter + 1;
          Heartbeat_Data[0] = Heartbeat_State;
          Heartbeat_Data[1] = Heartbeat_Counter;
          can.transmit(UCM2_FR_HEARTBEAT_ID, Heartbeat_Data, 2);
          digitalToggle(PC13);
          if (Heartbeat_Counter == 256){
          Heartbeat_Counter = 0; 
          };
                    
  };
  // Transmit data ----------------------------------//
 void data_tx(){
          can.transmit(UCM2_FR_DATA_ID, txData, txDataLen);
          //Serial1.println("Transmitting...");          
 }
// Arduinos "Main" function //
 void loop() {
  digitalupdate();
  I2C();
  Driver();
  //Serial1PRINT();
  heartbeat_timer.update();
  data_tx_timer.update();
  CANTransmit();
  CANRecieve();
}
