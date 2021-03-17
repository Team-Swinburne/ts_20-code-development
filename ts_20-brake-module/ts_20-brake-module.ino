// --------------------------------------------- 
//    TEAM SWINBURNE - TS20
//    BRAKE MODULE
//    MICHAEL COCHRANE & THOMAS BENNETT
// ---------------------------------------------
//      Microcontroller Pin-Out
// ---------------------------------------------
//PA0 - PA0 TEMP
//PA2 - UARTTX
//PA3 - UARTRX
//...
//PA8 - 5KW (ISO)
//PA9 - BSPD OK (ISO)
//PA11 - USBDM
//PA12 - USBDP
//PA13 - SWDIO
//PA14 - SWCLK
//...
//PB0 - CAN TX INDC
//PB1 - CAN RX INDC
//...
//PB6 - SENSOR2
//PB7 - SENSOR1
//PB8 - CAN1 RD
//PB9 - CAN1 TD
//...
//PB12 - BSPD DELAY (ISO)
//PB13 - CLOCK (ISO)
//PB14 - HIGH PRESSURE (ISO)
//PB15 - LOW PRESSURE (ISO)
//...
//PC13 - LED_BUILTIN
//PC14 - XTALC
//PC15 - XTALD
//...
//PD0 - XTALA
//PD1 - XTALB
// ---------------------------------------------
// LMV338 - information
// https://forum.arduino.cc/index.php?topic=285688.15
// --------------------------------------------

#include <Wire.h>
#include <eXoCAN.h>
#include <Ticker.h>

//   Intervals
#define         BRAKE_HEARTRATE         1       // 1 Hz
#define         BUTTON_CHECK_INTERVAL   0.2     // 5 Hz 
#define         CAN_BROADCAST_INTERVAL  0.02    // 50 Hz  

//   Brake Characteristics
#define Deadzone        30    // 40% Pedal deadzone

//   Brake Micro-Controlller Pins
#define pin_S1          PA4   // Sensor_1
#define pin_S2          PA5   // Sensor_2
#define pin_HP          PA6   // High_Pressure
#define pin_LP          PA7   // Low_Pressure
#define pin_CS          PA8   // 5kW or Current Sensor
#define pin_BSPD     PA9   // BSPD_OK     (no delay)
#define pin_BSPD_delay  PB12  // BSPD Delay  (10 second delay) (digital signal)
#define pin_clock       PB13  // Clock signal
#define TXLED      PB0   // CAN TX Indicator
#define RXLED      PB1   // CAN RX Indicator

//   Brake limits
int16_t         brake1_min              = 0;   // TS_19 = 980
int16_t         brake1_max              = 0;   // TS_19 = 1755
int16_t         brake2_min              = 0;   // TS_19 = 400
int16_t         brake2_max              = 0;   // TS_19 = 1250

//Brake Sensors
float Sensor_1;
float Sensor_2;
bool High_Pressure;
bool Low_Pressure;
bool kW5;
bool BSPD_OK;

//   Brake readings
int16_t         brake1_output           = 0;
float           brake1_output_percent   = 0;
int16_t         brake2_output           = 0;
float           brake2_output_percent   = 0;
int             brake_avg_percent       = 0;

//   CAN Structure
//CAN             can(PB9, PB9);
//DigitalOut      can_rx_led(RXLED);
//DigitalOut      can_tx_led(TXLED);
//CANMessage      can_msg;

//   CAN Initialization
//int BRAKE_HEARTBEAT_ID = 0x305;
//int BRAKE_SENSORS_ID   = 0x306;
//int BRAKE_SAFETY_ID    = 0x307;
int rxMsgID = 0x306;    // ID of brake sensor data -- for rx filtering
uint8_t   txData[8];
uint8_t   txDataLen = 8;
uint32_t  txDly = 5000; // Message Transmit Period in Milliseconds
uint32_t  last = 0;
int       id;
int       fltIdx;
uint8_t   rxbytes[8];
uint8_t   rxData[8];
int BMD_Heartbeat = 0x070;
uint32_t last_counter = 0;
uint32_t Heartbeat_Counter = 0;
uint8_t Heartbeat_Data[2];
uint8_t Heartbeat_State = 0;
eXoCAN can;

void heartbeat_tx();
Ticker heartbeat_timer(heartbeat_tx, 1000); 

//void CAN1_Transmit(){
  // Load Brake Data
  //txData[0] = brake1_output_percent;
  //txData[1] = brake2_output_percent;
  //txData[2] = brake_avg_percent;
  //txData[3] = BSPD_OK; 
  // Transmit data
  //if (millis() / txDly != last)             // tx every txDly
  //{
    //digitalToggle(TXLED);
    //Serial2.println("Transmitting...");
    //last = millis() / txDly;
    //can.transmit(txMsgID, txData, txDataLen);
    //digitalToggle(TXLED);
    //Serial2.println("Brake Data Transmit Success!"); 
  //}
//};

//void CANRecieve(){
  //if (can.receive(id, fltIdx, rxbytes) > -1) // poll for rx
  //{
    //digitalToggle(RXLED);        // turn on CAN RX indicator
    //rxData[8]=rxbytes;
    //Serial2.println("Receiving...");
    //digitalToggle(RXLED);        // turn off CAN RX indicator
  //}
//};

void updateValues(){
  Sensor_1        = analogRead(pin_S1);
  Sensor_2        = analogRead(pin_S2);
  High_Pressure   = digitalRead(pin_HP);
  Low_Pressure    = digitalRead(pin_LP);
  kW5             = digitalRead(pin_CS);
  BSPD_OK         = digitalRead(pin_BSPD);
  //BSPD_OK         = digitalRead(pin_BSPD_delay)       //uncomment to include 10 second delay after BSPD is tripped

  updateBrakeOne();
  updateBrakeTwo();   // inverted
  brake_avg_percent = (brake1_output_percent + brake2_output_percent)/2; 
}

void updateBrakeOne(){   
  brake1_output_percent = ((Sensor_1 - brake1_min)/(brake1_max - brake1_min))*100;
  Serial2.printf("Brake One Raw        : ", Sensor_1, "Brake One Precentage : ", brake1_output_percent);

  // Filter data to ensure it is within 0-100 percent
  if(brake1_output_percent < Deadzone)
  {
    brake1_output_percent = 0;
  }else
  {
    brake1_output_percent = ((brake1_output_percent - Deadzone) / (100- Deadzone)) * 100;
  }
  if(brake1_output_percent > 100)
  {
    brake1_output_percent = 100;
  }
  
  // Display the second brake percentage over serial
  Serial2.printf("Brake One Percentage : %f\n", brake1_output_percent);
}

void updateBrakeTwo(){   
  brake2_output_percent = ((Sensor_2 - brake2_min)/(brake2_max - brake2_min))*100;
  Serial2.printf("Brake Two Raw        : ", Sensor_2, "Brake Two Precentage : ", brake2_output_percent);

  // Filter data to ensure it is within 0-100 percent
  if(brake2_output_percent < Deadzone)
  {
    brake2_output_percent = 0;
  }else
  {
    brake2_output_percent = ((brake2_output_percent - Deadzone) / (100- Deadzone)) * 100;
  }
  if(brake2_output_percent > 100)
  {
    brake2_output_percent = 100;
  }
  
  // Invert brake input
  brake2_output_percent = 100 - brake2_output_percent;
  // Display the second brake percentage over serial
  Serial2.printf("Brake Two Percentage : %f\r\n", brake2_output_percent);
}

void serialPrint() {
  Serial2.printf("Sensor 1:      ", Sensor_1); 
  Serial2.printf("Sensor 2:      ", Sensor_2); 
  Serial2.printf("High Pressure: ", High_Pressure); 
  Serial2.printf("Low  Pressure: ", Low_Pressure); 
  Serial2.printf("5kW:           ", kW5);
  Serial2.printf("BSPD OK:       ", BSPD_OK);
}

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


  
  // Initiallising CANBUS
  can.begin(STD_ID_LEN, BR250K, PORTB_8_9_XCVR);          // 11b IDs, 250k bit rate, Pins 8 and 9 with a transceiver chip
  can.filterMask16Init(0, 0, 0x7ff, 0, 0);                // filter bank 0, filter 0: don't pass any, flt 1: pass all msgs
  pinMode(TXLED, OUTPUT);                            // CAN TX Indicator LED
  pinMode(RXLED, OUTPUT);                            // CAN RX Indicator LED
 
  // Initiallising Serial
  Serial2.begin(115200);
  Serial2.printf("Setup Successful.");
  
  heartbeat_timer.start();
}

//--------------------------//
//----Heartbeat Function----//
//--------------------------//
void heartbeat_tx(){    
          Heartbeat_Counter = Heartbeat_Counter + 1;
          digitalToggle(TXLED);
          Heartbeat_Data[0] = Heartbeat_State;
          Heartbeat_Data[1] = Heartbeat_Counter;
          can.transmit(BMD_Heartbeat, Heartbeat_Data, 2);
          digitalToggle(TXLED);
          digitalToggle(PC13);
          if (Heartbeat_Counter == 256){
          Heartbeat_Counter = 0; 
          };
                    
  };
//--------------------------------------------------//
// Arduinos "Main" function //
//--------------------------------------------------//
void loop() {
  updateValues();         //read micro-contropller pins and calculates brake values
  //CANTransmit();
  //CANRecieve();
  //digitalToggle(PC13);
  updateValues();
  updateBrakeOne();
  updateBrakeTwo();
  heartbeat_timer.update();
  //if (brake_output_percent > old_brake_output_percent); Serial.println("Brake!); 
  delay(100);             // wait 0.1 seconds
  }
