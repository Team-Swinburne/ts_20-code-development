#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <eXoCAN.h>

//Declarations
Adafruit_ADS1115 ads;
#define SERIAL_UART_INSTANCE    1 //ex: 2 for Serial2 (USART2)
#define PIN_SERIAL_RX           PA10
#define PIN_SERIAL_TX           PA9
#define TXLED PB0
#define RXLED PB1


int DriveCondition1, DriveRequirment1 = 1, DriveValue1 = 0;
int DriveCondition2, DriveRequirment2 = 1, DriveValue2 = 0;
int DigIN1 =0;
int DigIN2 =0;
int DigIN3 =0;
int DigIN4 =0;
int16_t adc0, adc1, adc2, adc3;


//CAN configuration constants

// tx frame setup #1
int UCMID = 0x069;
int rxMsgID = 0x005;   // needed for rx filtering
uint8_t txData[8];
uint8_t txDataLen = 8;
uint32_t txDly = 5000; //Message Transmit Frequency in Milliseconds (Transmits every 5 seconds atm)
uint32_t last = 0;
int id, fltIdx;
uint8_t rxbytes[8];
uint8_t rxData[8];
eXoCAN can;

// the setup function runs once when you press reset or power the board
void setup() {
  
  // initialize digital pin PC_13 as an output.
  // for troubleshooting to ensure the board is comunicating andissuing comands
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

  //ads.begin();
  
//--------------------------------------------------//
// Initiallising CAN
//--------------------------------------------------//
 can.begin(STD_ID_LEN, BR250K, PORTB_8_9_XCVR);           // 11b IDs, 250k bit rate, Pins 8 and 9 with a transceiver chip
 can.filterMask16Init(0, 0, 0x7ff, 0, 0);                 // filter bank 0, filter 0: don't pass any, flt 1: pass all msgs
  pinMode(PB0, OUTPUT);                                   // TX Indicator Pin
  pinMode(PB1, OUTPUT);                                   // RX Indicator Pin 
//--------------------------------------------------//
// Initiallising Serial
//--------------------------------------------------//
  Serial.begin(9600);
}


//--------------------------------------------------//
// Can Message Transmit.
//--------------------------------------------------//
 void CANTransmit(){

//--------------------------------------------------//
// Digital Input Value Fill
//--------------------------------------------------//
  txData[0]=DigIN1;
  txData[1]=DigIN2;
  txData[2]=DigIN3;
  txData[3]=DigIN4;


//--------------------------------------------------//
// Analogue Input Value Fill
//--------------------------------------------------//
  txData[4]=adc0;
  txData[5]=adc1;
  txData[6]=adc2;
  txData[7]=adc3;


//--------------------------------------------------//
// Transmit data
//--------------------------------------------------//
    if (millis() / txDly != last)             // tx every txDly
       {
          digitalToggle(TXLED);
          last = millis() / txDly;
          can.transmit(txMsgID, txData, txDataLen);
          digitalToggle(TXLED);
          Serial.println("Transmitting...");
  }
    };

//--------------------------------------------------//
// CAN Message Recieve. 
//--------------------------------------------------//
 void CANRecieve(){
    if (can.receive(id, fltIdx, rxbytes) > -1) // poll for rx
      {
       digitalToggle(RXLED);
       rxData[]=rxbytes;
       Serial.println("Receiving...");
       digitalToggle(RXLED);
      }
   };

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
// This Function Prints all values to Serial
//--------------------------------------------------//
void SERIALPRINT() {

  //Digital Input 
  Serial.print("Digital Input 1: ");  Serial.println(DigIN1); 
  Serial.print("Digital Input 2: ");  Serial.println(DigIN2); 
  Serial.print("Digital Input 3: ");  Serial.println(DigIN3); 
  Serial.print("Digital Input 4: ");  Serial.println(DigIN4); 
  //Analogue Input
  Serial.print("Analogue Input 0: "); Serial.println(adc0);
  Serial.print("Analogue Input 1: "); Serial.println(adc1);
  Serial.print("Analogue Input 2: "); Serial.println(adc2);
  Serial.print("Analogue Input 3: "); Serial.println(adc3);
  Serial.println(" ");
  //Driver Value
  Serial.print("Driver 1: "); Serial.println(DriveValue1);
  Serial.print("Driver 2: "); Serial.println(DriveValue2);
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
digitalWrite(PB15, HIGH);
digitalWrite(PB14, HIGH);
//--------------------------------------------------//
// Conditional Output Driver.
//--------------------------------------------------//
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
}

// Arduinos "Main" function //
void loop() {
  digitalupdate();
  I2C();
  Driver();
  CANTransmit();
  CANRecieve();
  digitalToggle(PC13);
  delay(100);   
}
