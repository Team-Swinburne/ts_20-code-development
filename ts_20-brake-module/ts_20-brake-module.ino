//    TEAM SWINBURNE - TS20
//    BRAKE MODULE
//    MICHAEL COCHRANE & THOMAS BENNETT

#include <Wire.h>
#include <eXoCAN.h>
#include "Ticker.h"

#define SERIAL_UART_INSTANCE    2  //ex: 1 for Serial1 (USART1)

// Intervals -------------------------------------
#define    HEARTRATE               1000       // 1 Hz
#define    BUTTON_CHECK_INTERVAL   200     // 5 Hz 
#define    CAN_BROADCAST_INTERVAL  20    // 50 Hz  

// Brake Characteristics -------------------------
#define Deadzone        30    // 30% Pedal deadzone

// Brake Micro-Controlller Pins
#define pin_S1          PA4   // Sensor_1
#define pin_S2          PA5   // Sensor_2
#define pin_HP          PA7   // High_Pressure
#define pin_LP          PA6   // Low_Pressure
#define pin_CS          PA8   // 5kW or Current Sensor
#define pin_BSPD        PA9   // BSPD_OK     (no delay)
#define pin_BSPD_delay  PB12  // BSPD Delay  (10 second delay) (digital signal)
#define pin_clock       PB13  // Clock signal
#define pin_LED_tx      PB0   // CAN TX Indicator
#define pin_LED_rx      PB1   // CAN RX Indicator

// Brake limits -----------------------------------
int16_t     brake1_min          = 0;   // TS_19 = 980
int16_t     brake1_max          = 0;   // TS_19 = 1755
int16_t     brake2_min          = 0;   // TS_19 = 400
int16_t     brake2_max          = 0;   // TS_19 = 1250

// Brake readings ---------------------------------
int16_t     brake1_raw          = 0;
float       brake1_percent      = 0;
int16_t     brake2_raw          = 0;
float       brake2_percent      = 0;
int         brake_avg_percent   = 0;


// CAN Initialization ------------------------------
int BRAKE_HEARTBEAT_ID = 0x308;
int BRAKE_SAFETY_ID    = 0x309;
int BRAKE_SENSORS_ID   = 0x30A;
int rxMsgID = 0x306;    // ID of brake sensor data -- for rx filtering
int       id;
int       fltIdx;
uint8_t   rxbytes[8];
uint8_t   rxData[8];
uint8_t heartbeat_cnt = 0;
uint8_t heartbeat_state = 1;
uint8_t High_Pressure = 0;
uint8_t Low_Pressure = 0;
uint8_t five_kW = 0;
uint8_t BSPD_OK = 0;
eXoCAN can;

void Heartbeat_TX();
void Safety_TX();
void Sensors_TX();
void CANRecieve();
void updateValues();

Ticker ticker_heartbeat(Heartbeat_TX,HEARTRATE);
Ticker ticker_safety_tx(Safety_TX,CAN_BROADCAST_INTERVAL);
Ticker ticker_sensors_tx(Sensors_TX,CAN_BROADCAST_INTERVAL);
//--------------------------------------------------//
// The setup function runs once when you press reset or power the board
//--------------------------------------------------//
void setup() {  
  // Configuring the Micro-Controller Input Pins.
  pinMode(pin_S1, INPUT);       //Sensor_1
  pinMode(pin_S2, INPUT);       //Sensor_2
  pinMode(pin_HP, INPUT);       //High_Pressure
  pinMode(pin_LP, INPUT);       //Low_Pressure
  pinMode(pin_CS, INPUT);       //5kW or Current Sensor
  pinMode(pin_BSPD, INPUT);     //BSPD_OK

  // Initialize LED for troubleshooting.
  pinMode(PC13, OUTPUT);

  // Initialize timer for the functions.
  ticker_heartbeat.start();
  ticker_safety_tx.start();
  delay(5); // start sensor tx timer 5ms behind since the board can't transmit 2 CAN messages at the same time
  ticker_sensors_tx.start();
  
  // Initiallising CANBUS
  can.begin(STD_ID_LEN, BR500K, PORTB_8_9_XCVR);          // 11b IDs, 250k bit rate, Pins 8 and 9 with a transceiver chip
  can.filterMask16Init(0, 0, 0x7ff, 0, 0);                // filter bank 0, filter 0: don't pass any, flt 1: pass all msgs
  pinMode(pin_LED_tx, OUTPUT);                            // CAN TX Indicator LED
  pinMode(pin_LED_rx, OUTPUT);                            // CAN RX Indicator LED
 
  // Initiallising Serial2
  Serial2.begin(115200);
  Serial2.println("Setup Successful.");
}

//--------------------------------------------------//
// Arduinos "Main" function //
//--------------------------------------------------//
void loop() {
  updateValues();         //read micro-contropller pins and calculates brake values
  CANRecieve();
  ticker_heartbeat.update();
  ticker_safety_tx.update();
  ticker_sensors_tx.update();
  SerialPrint();
  //if (brake_output_percent > old_brake_output_percent); Serial2.println("Brake!); 
}

void Heartbeat_TX() {
  if (heartbeat_cnt >= 255) {
    heartbeat_cnt = 0;
  }
  heartbeat_cnt++;
  uint8_t txData[2] = {heartbeat_state, heartbeat_cnt};
  uint8_t txDataLen = 2;
  can.transmit(BRAKE_HEARTBEAT_ID, txData, txDataLen);
  //digitalToggle(pin_LED_tx);
  digitalToggle(PC13);
  //Serial2.print("Heartbeat: ");
  //Serial2.println(heartbeat_cnt);
  //Serial2.println(status_can);
}
void Safety_TX(){
  //digitalToggle(pin_LED_tx);
  uint8_t txData[4] = {0, 0, 0, 0};
  uint8_t txDataLen = 4;
  can.transmit(BRAKE_SAFETY_ID, txData, txDataLen);
}

void Sensors_TX(){
  digitalToggle(pin_LED_tx);
  uint8_t txData[3] = {brake1_percent, brake2_percent, brake_avg_percent};
  uint8_t txDataLen = 3;
  can.transmit(BRAKE_SENSORS_ID, txData, txDataLen);
  digitalToggle(pin_LED_tx);
}

void CANRecieve(){
  if (can.receive(id, fltIdx, rxbytes) > -1) // poll for rx
  {
    digitalToggle(pin_LED_rx);        // turn on CAN RX indicator
    //rxData[]=rxbytes;
    Serial2.println("Receiving...");
    digitalToggle(pin_LED_rx);        // turn off CAN RX indicator
  }
};

void updateValues(){
  brake1_raw          = analogRead(pin_S1);
  brake2_raw          = analogRead(pin_S2);
  High_Pressure       = digitalRead(pin_HP);
  Low_Pressure        = digitalRead(pin_LP);
  five_kW             = digitalRead(pin_CS);
  BSPD_OK             = digitalRead(pin_BSPD);
  //BSPD_OK         = digitalRead(pin_BSPD_delay)       //uncomment to include 10 second delay after BSPD is tripped

  updateBrakeOne(); 
  updateBrakeTwo();   // inverted
  brake_avg_percent = (brake1_percent + brake2_percent)/2; 
}

void updateBrakeOne(){   
  brake1_percent = ((brake1_raw - brake1_min)/(brake1_max - brake1_min))*100;
  // Filter data to ensure it is within 0-100 percent and ignore the deadzone
  brake1_percent = max(brake1_percent - Deadzone,brake1_percent);
  brake1_percent = ((brake1_percent - Deadzone) / (100- Deadzone)) * 100;
  brake1_percent = min(brake1_percent, float(100));
}

void updateBrakeTwo(){   
  brake2_percent = ((brake2_raw - brake2_min)/(brake2_max - brake2_min))*100;
  // Filter data to ensure it is within 0-100 percent
  brake2_percent = max(brake2_percent - Deadzone,brake2_percent);
  brake2_percent = ((brake2_percent - Deadzone) / (100- Deadzone)) * 100;
  brake2_percent = min(brake2_percent, float(100));
  // Invert brake input
  brake2_percent = 100 - brake2_percent;
}

void SerialPrint() {
  Serial2.print("Sensor 1 & 2:      ");
  Serial2.print(brake1_raw); 
  Serial2.print("     ");
  Serial2.println(brake2_raw); 
  Serial2.print("High Pressure: ");
  Serial2.println(High_Pressure); 
  Serial2.print("Low  Pressure: ");
  Serial2.println(Low_Pressure); 
  Serial2.print("5kW:       ");
  Serial2.println(five_kW);
  Serial2.print("BSPD OK:       ");
  Serial2.println(BSPD_OK);
  Serial2.println(" ");
  delay(500);
}