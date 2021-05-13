#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <eXoCAN.h>
#include <Ticker.h>

//Declarations
Adafruit_ADS1115 ads(0x49);
#define Serial1_UART_INSTANCE    1 //ex: 2 for Serial12 (USART2)
#define PIN_Serial1_RX           PA10
#define PIN_Serial1_TX           PA9
#define TXLED PB0
#define RXLED PB1


int DriveCondition1, DriveRequirment1 = 1, DriveValue1 = 0;
int DriveCondition2, DriveRequirment2 = 1, DriveValue2 = 0;
int DigIN1 =0;
int DigIN2 =0;
int DigIN3 =0;
int DigIN4 =0;
int16_t adc0, adc1, adc2, adc3;

int avBrake = 0;

//Ticker 
void heartbeat_tx();
Ticker heartbeat_timer(heartbeat_tx, 1000); 

void Transmit_data();
Ticker Transmit_data_timer(Transmit_data, 100);


//CAN configuration constants

// tx frame setup #1
int UCMID = 0x069;
int UCM_Heartbeat = 0x068;
int rxMsgID = 0x005;   // needed for rx filtering
uint8_t txData[8];
uint8_t txDataLen = 8;
uint32_t txDly = 100; //Message Transmit Frequency in Milliseconds (Transmits every 5 seconds atm)
uint32_t last = 0;
int fltIdx;
uint8_t rxbytes[8];
uint8_t rxData[8];
uint32_t last_counter = 0;
uint32_t Heartbeat_Counter = 0;
uint8_t Heartbeat_Data[2];
uint8_t Heartbeat_State = 0;
eXoCAN can;

// the setup function runs once when you press reset or power the board
void setup() {
  
  // initialize digital pin PC_13 as an output.
  // for troubleshooting to ensure the board is comunicating and issuing comands
  pinMode(PC13, OUTPUT);
//--------------------------------------------------//
//Configuring the Digital Input Pins. Uncomment when required.
//--------------------------------------------------//
  pinMode(PB10, INPUT);
  pinMode(PB11, INPUT);
  pinMode(PB12, INPUT);
  pinMode(PB13, INPUT);

//--------------------------------------------------//
// Configuring Driver Pins. Uncomment when required.
//--------------------------------------------------//
  pinMode(PB15, OUTPUT);
  pinMode(PB14, OUTPUT);
//--------------------------------------------------//
// ADC Setup for I2C. Uncomment when required.
//--------------------------------------------------// 
  //Wire.begin();
 
  // //The ADC input range (or gain) can be changed via the following
  // //functions, but be careful never to exceed VDD +0.3V max, or to
  // //exceed the upper and lower limits if you adjust the input range!
  // //Setting these values incorrectly may destroy your ADC!
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // //ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  // //ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // //ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // //ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // //ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // //ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV

  ads.begin();
  
//--------------------------------------------------//
// Initiallising CAN
//--------------------------------------------------//
 can.begin(STD_ID_LEN, BR500K, PORTB_8_9_XCVR);           // 11b IDs, 250k bit rate, Pins 8 and 9 with a transceiver chip
 can.filterMask16Init(0, 0, 0x7ff, 0, 0);                 // filter bank 0, filter 0: don't pass any, flt 1: pass all msgs
  pinMode(PB0, OUTPUT);                                   // TX Indicator Pin
  pinMode(PB1, OUTPUT);                                   // RX Indicator Pin 
//--------------------------------------------------//
// Initiallising Serial1
//--------------------------------------------------//
  Serial1.begin(9600);
//-----------------------//
//Ticker Setup
//-----------------------//
heartbeat_timer.start();
Transmit_data_timer.start();
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

void brakePercentage(){
  avBrake = rxbytes[2];
  Serial1.println(avBrake, DEC);
}

//--------------------------------------------------//
// This function reads all digital input values and stores them in variables.
//--------------------------------------------------//
void digitalupdate(){
  DigIN1 = digitalRead(PB10);  
  DigIN2 = digitalRead(PB11);
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
if (avBrake >= 50) {
  digitalWrite(PB15, HIGH);
  digitalWrite(PB14, HIGH);
}
else {
  digitalWrite(PB15, LOW);
  digitalWrite(PB14, LOW);
}
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
//  else {
//  digitalWrite(PB14, LOW);
//  DriveValue2 = 0;
//  }
//}
}
// Transmit Hearbeat ----------------------------------//
void heartbeat_tx(){    
          Heartbeat_Counter = Heartbeat_Counter + 1;
          digitalToggle(TXLED);
          Heartbeat_Data[0] = Heartbeat_State;
          Heartbeat_Data[1] = Heartbeat_Counter;
          can.transmit(UCM_Heartbeat, Heartbeat_Data, 2);
          digitalToggle(TXLED);
          digitalToggle(PC13);
          if (Heartbeat_Counter == 256){
          Heartbeat_Counter = 0; 
          };
                    
  };
  // Transmit data ----------------------------------//
 void Transmit_data(){
          digitalToggle(TXLED);
          //last = millis() / txDly;
          can.transmit(UCMID, txData, txDataLen);
          digitalToggle(TXLED);
          //Serial1.println("Transmitting...");          
 }
// Arduinos "Main" function //
 void loop() {
  digitalupdate();
  I2C();
  Driver();
  //Serial1PRINT();
  heartbeat_timer.update();
  Transmit_data_timer.update();
  CANTransmit();
  CANRecieve();
  brakePercentage();
}