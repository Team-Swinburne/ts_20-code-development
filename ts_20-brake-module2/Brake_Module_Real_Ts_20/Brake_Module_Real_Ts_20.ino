#include <Wire.h>
#include <eXoCAN.h>
#include <Ticker.h>

//Declarations
#define Serial2_UART_INSTANCE    2 //ex: 2 for Serial12 (USART2)
#define PIN_Serial2_RX           PA10
#define PIN_Serial2_TX           PA9
#define TXLED PB0
#define RXLED PB1

//Brake Micro-Controlller Pins
#define pin_S1          PA4   // Sensor_1
#define pin_S2          PA5   // Sensor_2
#define pin_HP          PA6   // High_Pressure
#define pin_LP          PA7   // Low_Pressure
#define pin_CS          PA8   // kW5 or Current Sensor
#define pin_BSPD        PA9   // BSPD_OK     (no delay)
#define pin_BSPD_delay  PB12  // BSPD Delay  (10 second delay) (digital signal)
#define pin_clock       PB13  // Clock signal

//   Intervals
#define         BRAKE_HEARTRATE         1       // 1 Hz
#define         BUTTON_CHECK_INTERVAL   0.2     // 5 Hz 
#define         CAN_BROADCAST_INTERVAL  0.02    // 50 Hz  


//   Brake limits
int16_t         brake1_min              = 0;   // TS_19 = 980
int16_t         brake1_max              = 0;   // TS_19 = 1755
int16_t         brake2_min              = 0;   // TS_19 = 400
int16_t         brake2_max              = 0;   // TS_19 = 1250

//   Brake readings
int16_t         brake1_output           = 0;
float           brake_output_1_percent   = 0;
int16_t         brake2_output           = 0;
float           brake_output_2_percent   = 0;
int             brake_avg_percent       = 0;


//   Brake Characteristics
#define Deadzone        30    // 40% Pedal deadzone

float Sensor_1;
float Sensor_2;
int High_Pressure;
int Low_Pressure;
int kW5;       
int BSPD_OK;       

//Ticker 
void heartbeat_tx();
Ticker heartbeat_timer(heartbeat_tx, 1000); 


// tx frame setup #1
int UCMID = 0x069;
int UCM_Heartbeat = 0x068;
uint32_t last_counter = 0;
uint32_t Heartbeat_Counter = 0;
uint8_t Heartbeat_Data[2];
uint8_t Heartbeat_State = 0;
eXoCAN can;

// the setup function runs once when you press reset or power the board
void setup() {
  
  pinMode(PC13, OUTPUT);
//--------------------------------------------------//
// Initiallising Serial1
//--------------------------------------------------//
    // Initiallising Serial
  Serial2.begin(115200);
  Serial2.println("Setup Successful.");
//-----------------------//
//Ticker Setup
//-----------------------//
heartbeat_timer.start();
}
//----------------------------------------------------//
//----------------------------------------------------//
//----------------------------------------------------//
//----------------------------------------------------//
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
  brake_avg_percent = (brake_output_1_percent + brake_output_2_percent)/2; 
}

void updateBrakeOne(){   
    brake_output_1_percent = ((Sensor_1 - brake1_min)/(brake1_max - brake1_min))*100;
  //Serial2.println("Brake One Raw        : ", Sensor_1, "Brake One Precentage : ", brake_output_1_percent);

  // Filter data to ensure it is within 0-100 percent
  if(brake_output_1_percent < Deadzone)
  {
    brake_output_1_percent = 0;
  }else
  {
    brake_output_1_percent = ((brake_output_1_percent - Deadzone) / (100- Deadzone)) * 100;
  }
  if(brake_output_1_percent > 100)
  {
    brake_output_1_percent = 100;
  }
  
  // Display the second brake percentage over serial
  //Serial2.println("Brake One Percentage : %f\n", brake_output_1_percent);
}

void updateBrakeTwo(){   
  brake_output_2_percent = ((Sensor_2 - brake2_min)/(brake2_max - brake2_min))*100;
  //Serial2.println("Brake Two Raw        : ", Sensor_2, "Brake Two Precentage : ", brake_output_2_percent);

  // Filter data to ensure it is within 0-100 percent
  if(brake_output_2_percent < Deadzone)
  {
    brake_output_2_percent = 0;
  }else
  {
    brake_output_2_percent = ((brake_output_2_percent - Deadzone) / (100- Deadzone)) * 100;
  }
  if(brake_output_2_percent > 100)
  {
    brake_output_2_percent = 100;
  }
  
  // Invert brake input
  brake_output_2_percent = 100 - brake_output_2_percent;
  // Display the second brake percentage over serial
  //Serial2.println("Brake Two Percentage : %f\r\n", brake_output_2_percent);
}

  // Initialize LED for troubleshooting.
  //pinMode(PC13, OUTPUT);

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


// Arduinos "Main" function //
void loop() {
  updateValues();
  heartbeat_timer.update();
}
