#include "Ticker.h"
#include "ArduinoModbus.h"

//=========================================================================================
// Pinout of Arduino
//=========================================================================================
#define LED_power     2
#define LED_indi      3
#define b_temp_set    5
#define b_temp_up     6
#define b_temp_down   7
#define button_go     8

//=========================================================================================
// Settings
//=========================================================================================
#define BUTTON_PRESS_DELAY 200

//=========================================================================================
// Modbus Addresses
//=========================================================================================
//Holding registers
#define COOK_HOURS_ADR 0x1000
#define COOK_MINUTES_ADR 0x1001
#define COOK_SECONDS_ADR 0x1002
#define BEGIN_TEMP_ADR 0x1003
#define COOK_TEMP_ADR 0x1004
#define FINISH_TEMP_ADR 0x1005
#define TEMP_RISE_ADR 0x1006
#define TEMP_FALL_ADR 0x1007

//Input registers
#define OVEN_STATE_ADR 0x2000
#define CURRENT_TEMP_ADR 0x2001
#define TIME_REMAIN_HOURS_ADR 0x2002
#define TIME_REMAIN_MINUTES_ADR 0x2003
#define TIME_REMAIN_SECONDS_ADR 0x2004

//Coils
#define COOK_START_ADR 0x01
#define STOP_COMMAND_ADR 0x02

//=========================================================================================
// Structs
//=========================================================================================
//struct that stores the values for the cooking profiles
typedef struct {
  uint8_t TimeHours;
  uint8_t TimeMinutes;
  uint8_t TimeSeconds; 
  uint8_t BeginTemp;
  uint8_t CookTemp;
  uint8_t FinishTemp;
  float TempRise;
  float TempCool;
} CookingProfile;

CookingProfile OvenSettings;

typedef enum {
  IDLE_STATE = 1,
  HEATING_STATE = 2,
  COOKING_STATE = 3,
  COOLING_STATE = 4
} States;

typedef struct {
  States OvenState = IDLE_STATE;
  uint8_t CurrentTemp = 30;
  uint32_t TimeRemaining;
} Status;

Status OvenStatus;

//=========================================================================================
// Function prototypes
//=========================================================================================
void PressButton(int button);
void IncrementTemp();
void DecrementTemp();
void SendStatus();
void DecreaseTime();
void CheckStopCommand();
void GetCookCommand();

//=========================================================================================
// Timers
//=========================================================================================
Ticker timer1(SendStatus, 1000, 0, MILLIS);
Ticker timer2(IncrementTemp, 60000, 0, MILLIS);
Ticker timer3(DecreaseTime, 1000, 0, MILLIS);
Ticker timer4(DecrementTemp, 60000, 0, MILLIS);

void PressButton(int button)
{
  digitalWrite(button, HIGH);
  delay(BUTTON_PRESS_DELAY);
  digitalWrite(button, LOW);
  delay(BUTTON_PRESS_DELAY);
}

void IncrementTemp()
{
  OvenStatus.CurrentTemp++;

  /*
   * Set (Go to edit temp mode) -> Up (increase temp) -> Set (confirm temp change) -> Set (exit edit temp mode)
   */

  PressButton(b_temp_set);
  PressButton(b_temp_up);
  PressButton(b_temp_set);
  PressButton(b_temp_set);
}

void DecrementTemp()
{
  OvenStatus.CurrentTemp--;

    /*
   * Set (Go to edit temp mode) -> Down (decrease temp) -> Set (confirm temp change) -> Set (exit edit temp mode)
   */

  PressButton(b_temp_set);
  PressButton(b_temp_down);
  PressButton(b_temp_set);
  PressButton(b_temp_set);
}

//Sends the current status of the composite oven
void SendStatus()
{
  //Serial.write(0x55); //Start of frame
  //Serial.write(OvenStatus.OvenState); //State
  //Serial.write(OvenStatus.CurrentTemp); //Current Temp
  //Serial.write(OvenStatus.TimeRemaining / 3600); //Hour
  //Serial.write((OvenStatus.TimeRemaining % 3600)/60); //Minute
  //Serial.write((OvenStatus.TimeRemaining % 3600) % 60); //Second

  ModbusRTUServer.inputRegisterWrite(OVEN_STATE_ADR, OvenStatus.OvenState);
  ModbusRTUServer.inputRegisterWrite(CURRENT_TEMP_ADR, OvenStatus.CurrentTemp);
  ModbusRTUServer.inputRegisterWrite(TIME_REMAIN_HOURS_ADR, OvenStatus.TimeRemaining / 3600);
  ModbusRTUServer.inputRegisterWrite(TIME_REMAIN_MINUTES_ADR, (OvenStatus.TimeRemaining % 3600) / 60);
  ModbusRTUServer.inputRegisterWrite(TIME_REMAIN_SECONDS_ADR, (OvenStatus.TimeRemaining % 3600) % 60);
}

void DecreaseTime()
{
  OvenStatus.TimeRemaining--;
}

void CheckStopCommand()
{
  if(ModbusRTUServer.coilRead(STOP_COMMAND_ADR))
  {
    ModbusRTUServer.coilWrite(COOK_START_ADR, 0);
    ModbusRTUServer.coilWrite(STOP_COMMAND_ADR, 0);
    OvenStatus.OvenState = IDLE_STATE;
    timer2.stop();
    timer3.stop();
    timer4.stop();
  }
}

//Waits until it gets the command from the PC to cook
void GetCookCommand()
{
  /*
  if(!(Serial.available() > 0)) return; //return if no data to read

  if(Serial.read() != 0x22) return; //return if first byte isn't 0x22

  OvenSettings.TimeHours = Serial.read();
  OvenSettings.TimeMinutes = Serial.read();
  OvenSettings.TimeSeconds = Serial.read();
  OvenSettings.CookTemp = Serial.read();
  OvenSettings.TempRise = (Serial.read())/100.0;
  OvenSettings.TempCool = (Serial.read())/100.0;

  OvenStatus.OvenState = HEATING_STATE;
  OvenStatus.TimeRemaining = 3600*OvenSettings.TimeHours + 60*OvenSettings.TimeMinutes + OvenSettings.TimeSeconds;
  timer2.interval((60000)/(OvenSettings.TempRise));
  timer2.start();
  */

  if(ModbusRTUServer.coilRead(COOK_START_ADR))
  {
    OvenSettings.TimeHours = ModbusRTUServer.holdingRegisterRead(COOK_HOURS_ADR);
    OvenSettings.TimeMinutes = ModbusRTUServer.holdingRegisterRead(COOK_MINUTES_ADR);
    OvenSettings.TimeSeconds = ModbusRTUServer.holdingRegisterRead(COOK_SECONDS_ADR);
    OvenSettings.BeginTemp = ModbusRTUServer.holdingRegisterRead(BEGIN_TEMP_ADR);
    OvenSettings.CookTemp = ModbusRTUServer.holdingRegisterRead(COOK_TEMP_ADR);
    OvenSettings.FinishTemp = ModbusRTUServer.holdingRegisterRead(FINISH_TEMP_ADR);
    OvenSettings.TempRise = ModbusRTUServer.holdingRegisterRead(TEMP_RISE_ADR)/100.0;
    OvenSettings.TempCool = ModbusRTUServer.holdingRegisterRead(TEMP_FALL_ADR)/100.0;

    ModbusRTUServer.coilWrite(COOK_START_ADR, 0);
    OvenStatus.OvenState = HEATING_STATE;
    OvenStatus.TimeRemaining = 3600L*OvenSettings.TimeHours + 60*OvenSettings.TimeMinutes + OvenSettings.TimeSeconds;
    OvenStatus.CurrentTemp = OvenSettings.BeginTemp;
    timer2.interval((60000)/(OvenSettings.TempRise));
    timer2.start();
  }
}

void setup() 
{
  Serial.begin(9600);

  pinMode(b_temp_set, OUTPUT);
  pinMode(b_temp_up, OUTPUT);
  pinMode(b_temp_down, OUTPUT);

  timer1.start();

  if (!ModbusRTUServer.begin(0x01, 9600)) 
  {
    while (1);
  }

  /*
  ModbusRTUServer.configureInputRegisters(0x10, 1);
  ModbusRTUServer.inputRegisterWrite(0x10, 1000);
  */

  //Configure all Modbus addresses
  ModbusRTUServer.configureHoldingRegisters(COOK_HOURS_ADR, 8);
  ModbusRTUServer.configureCoils(COOK_START_ADR, 2);
  ModbusRTUServer.configureInputRegisters(OVEN_STATE_ADR, 5);

  //Set all Modbus address values
  ModbusRTUServer.coilWrite(COOK_START_ADR, 0);
  ModbusRTUServer.coilWrite(STOP_COMMAND_ADR, 0);
}

void loop() 
{  
  ModbusRTUServer.poll();

  timer1.update();

  switch(OvenStatus.OvenState)
  {
    case IDLE_STATE:
      GetCookCommand();
      break;

    case HEATING_STATE:
      CheckStopCommand();
      timer2.update();

      if(OvenStatus.CurrentTemp >= OvenSettings.CookTemp)
      {
        timer2.stop();
        OvenStatus.OvenState = COOKING_STATE;
        timer3.start();
      }
      break;

    case COOKING_STATE:
      CheckStopCommand();
      timer3.update();
      
      if(OvenStatus.TimeRemaining == 0)
      {
        timer3.stop();
        OvenStatus.OvenState = COOLING_STATE;
        timer4.interval((60000)/(OvenSettings.TempCool));
        timer4.start();
      }
      break;

    case COOLING_STATE:
      CheckStopCommand();
      timer4.update();

      if(OvenStatus.CurrentTemp <= OvenSettings.FinishTemp)
      {
        timer4.stop();
        OvenStatus.OvenState = IDLE_STATE;
      }
      break;

    default:
      break;
  }
  
  delay(100);
}
