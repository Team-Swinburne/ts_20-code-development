//    TEAM SWINBURNE - TS20
//    UCM Code for TS_19

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
#define   UCM_HEARTBEAT_ID      0x511 //UCM_FL_HEARTBEAT_ID
#define  UCM_DATA_ID            0x512 //UCM_FL_DATA_ID
int   UCM_TEST              =    0x309;

// CANBus Intervals
#define    HEARTRATE               1       //ms
#define    CAN_BROADCAST_INTERVAL  50         //ms


// UCM Pin Mapping
#define pin_D1              PB12   // Digital Input 1
#define pin_D2              PB13   // Digital Input 2
#define pin_Driver1         PB15  // Driver 1 (24V)
#define pin_Driver2         PB14  // Driver 2 (24V)
#define pin_PWM_Driver1     PB0   // PWM Driver 1 (5V)
#define pin_PWM_Driver2     PB1   // PWM Driver 2 (5V)

// Constants
const uint16_t shock_sensor_max = 23600;
const uint16_t shock_sensor_min = 0;

// Globals

static uint8_t DriveCondition1, DriveRequirment1 = 1, DriveValue1 = 0;
static uint8_t DriveCondition2, DriveRequirment2 = 1, DriveValue2 = 0;
static uint8_t DigIN1 =0;
static uint8_t DigIN2 =0;
static uint8_t PWM_Drive1 = 0, PWM_Drive2 = 0;
static uint16_t adc0, adc1, adc2, adc3;

static int fltIdx,id;
static uint8_t rxData[8];
static uint32_t Heartbeat_Counter = 0;
static uint8_t Heartbeat_State = 0;
int len = -1;
char cBuff[50];
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

uint8_t calculate_shock_percent(uint16_t raw_adc){
  float shock_pot_percent_float = 255 - (adc0*255.0)/shock_sensor_max;
  return shock_pot_percent_float;
}

void initialise_GPIO(){
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
}

//--------------------------------------------------//
// the setup function runs once when you press reset or power the board
//--------------------------------------------------//
void setup() {
    // Start I2C communication with the ADC
  initialise_GPIO();
  
  ads.begin(0x49);
  
  // Initiallising CAN
  can.begin(STD_ID_LEN, BR500K, PORTB_8_9_XCVR);           // 11b IDs, 250k bit rate, Pins 8 and 9 with a transceiver chip
  //can.filterMask16Init(0, UCM_TEST, 0x7ff);                // filter out every message exept for UCM_TEST
  
  // Initiallising Serial1
  Serial1.begin(250000);
  sprintf(cBuff,"Starting ts_20 Universal Controller Module 1 - Front Left (STM32F103C8T6 32k) \
	\r\nCOMPILED: %s: %s\r\n",__DATE__, __TIME__);
  Serial1.print(cBuff);
  
  //Ticker Setup
  ticker_heartbeat.start();
  delay(100);
  ticker_data_tx.start();
  delay(100);
  
  
  
  Serial1.println("STARTED DATA TICKER!");

  //can.attachInterrupt(canISR);
}

//--------------------------------------------------//
// Arduinos "Main" function
//--------------------------------------------------//
 void loop() {
 // digitalupdate();
  I2C();
//  CANRecieve();
  ticker_heartbeat.update();
//  ticker_data_tx.update();
  //Driver();
 // SerialPRINT();
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
  //adc1 = ads.readADC_SingleEnded(1);
  //adc2 = ads.readADC_SingleEnded(2);
  //adc3 = ads.readADC_SingleEnded(3);

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
  int dlc = 3;
  Heartbeat_Counter = Heartbeat_Counter + 1;
  
  uint8_t txData[dlc];
  txData[0] = Heartbeat_State;
  txData[1] = Heartbeat_Counter;
  txData[2] = calculate_shock_percent(adc0);
  can.transmit(UCM_HEARTBEAT_ID, txData, dlc);
  digitalToggle(PC13);
//  if (Heartbeat_Counter == 256){
//    Heartbeat_Counter = 0; 
//  };                 
}

// Transmit data ----------------------------------//
void data_tx(){
  Serial1.println("IF I GET INTO HERE, I AM IN THE DAMN INTERRUPT!!");
  int dlc = 1;
  uint8_t txData[dlc];
  
  txData[0] = calculate_shock_percent(adc0);
  can.transmit(UCM_DATA_ID, txData, dlc);
  Serial1.println("Transmitting...");          
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
  Serial1.print("Analogue Input 0: "); Serial1.print(adc0);
  Serial1.print("     Shock Pot Percent: "); Serial1.println(calculate_shock_percent(adc0));
  // Serial1.print("Analogue Input 3: "); Serial1.println(adc3);
  // Serial1.println(" ");
  // //Driver Value
  // Serial1.print("Driver 1: "); Serial1.println(DriveValue1);
  // Serial1.print("Driver 2: "); Serial1.println(DriveValue2);
  // delay(300);
  //sprintf(cBuff,"Drive Cond 1: %d    Drive Cond 2: %d /r/n", DriveCondition1, DriveCondition2);
  //Serial1.print(cBuff);
  //sprintf(cBuff,"Drive PWM 1:  %d    Drive PWM 2: %d /r/n", PWM_Drive1, PWM_Drive2);
  //Serial1.println(cBuff);
}
