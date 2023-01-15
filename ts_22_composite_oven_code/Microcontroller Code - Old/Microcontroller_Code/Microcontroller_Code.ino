#include "Ticker.h"

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

//struct that stores the values for the cooking profiles
typedef struct {
  uint8_t TimeHours;
  uint8_t TimeMinutes;
  uint8_t TimeSeconds; 
  uint8_t CookTemp;
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

void IncrementTemp()
{
  OvenStatus.CurrentTemp++;

  //Emulate increment temp button press.
  delay(BUTTON_PRESS_DELAY);
  digitalWrite(b_temp_up, HIGH);
  delay(BUTTON_PRESS_DELAY);
  digitalWrite(b_temp_up, LOW);
  
  //Emulate set temp button press.
  digitalWrite(b_temp_set, HIGH);
  delay(BUTTON_PRESS_DELAY);
  digitalWrite(b_temp_set, LOW);
  delay(BUTTON_PRESS_DELAY);
}

void DecrementTemp()
{
  OvenStatus.CurrentTemp--;

  //Emulate decrement temp button press.
  delay(BUTTON_PRESS_DELAY);
  digitalWrite(b_temp_down, HIGH);
  delay(BUTTON_PRESS_DELAY);
  digitalWrite(b_temp_down, LOW);
  
  //Emulate set temp button press.
  digitalWrite(b_temp_set, HIGH);
  delay(BUTTON_PRESS_DELAY);
  digitalWrite(b_temp_set, LOW);
  delay(BUTTON_PRESS_DELAY);
}

//Sends the current status of the composite oven
void SendStatus()
{
  Serial.write(0x55); //Start of frame
  Serial.write(OvenStatus.OvenState); //State
  Serial.write(OvenStatus.CurrentTemp); //Current Temp
  Serial.write(OvenStatus.TimeRemaining / 3600); //Hour
  Serial.write((OvenStatus.TimeRemaining % 3600)/60); //Minute
  Serial.write((OvenStatus.TimeRemaining % 3600) % 60); //Second
}

void DecreaseTime()
{
  OvenStatus.TimeRemaining--;
}

//=========================================================================================
// Timers
//=========================================================================================
Ticker timer1(SendStatus, 1000, 0, MILLIS);
Ticker timer2(IncrementTemp, 60000, 0, MILLIS);
Ticker timer3(DecreaseTime, 1000, 0, MILLIS);
Ticker timer4(DecrementTemp, 60000, 0, MILLIS);

//Waits until it gets the command from the PC to cook
void GetCookCommand()
{
  if(!(Serial.available() > 0)) return; //return if no data to read

  if(Serial.read() != 0x22) return; //return if first byte isn't 0x22

  OvenSettings.TimeHours = Serial.read();
  OvenSettings.TimeMinutes = Serial.read();
  OvenSettings.TimeSeconds = Serial.read();
  OvenSettings.CookTemp = Serial.read();
  OvenSettings.TempRise = (Serial.read())/10.0;
  OvenSettings.TempCool = (Serial.read())/10.0;

  OvenStatus.OvenState = HEATING_STATE;
  OvenStatus.TimeRemaining = 3600*OvenSettings.TimeHours + 60*OvenSettings.TimeMinutes + OvenSettings.TimeSeconds;
  timer2.interval((60000)/(OvenSettings.TempRise));
  timer2.start();

  //Emulate set temp button press.
  digitalWrite(b_temp_set, HIGH);
  delay(BUTTON_PRESS_DELAY);
  digitalWrite(b_temp_set, LOW);
  delay(BUTTON_PRESS_DELAY);
}

void setup() 
{
  Serial.begin(9600);

  pinMode(b_temp_set, OUTPUT);
  pinMode(b_temp_up, OUTPUT);
  pinMode(b_temp_down, OUTPUT);

  timer1.start();
}

void loop() 
{
  timer1.update();

  switch(OvenStatus.OvenState)
  {
    case IDLE_STATE:
      GetCookCommand();
      break;

    case HEATING_STATE:
      timer2.update();

      if(OvenStatus.CurrentTemp >= OvenSettings.CookTemp)
      {
        timer2.stop();
        OvenStatus.OvenState = COOKING_STATE;
        timer3.start();
      }
      break;

    case COOKING_STATE:
      timer3.update();
      
      if(OvenStatus.TimeRemaining == 0)
      {
        timer3.stop();
        OvenStatus.OvenState = COOLING_STATE;
        timer4.interval((60000)/(OvenSettings.TempCool));
        timer4.start();

        //Emulate set temp button press.
        digitalWrite(b_temp_set, HIGH);
        delay(BUTTON_PRESS_DELAY);
        digitalWrite(b_temp_set, LOW);
        delay(BUTTON_PRESS_DELAY);
      }
      break;

    case COOLING_STATE:
      timer4.update();

      if(OvenStatus.CurrentTemp <= 30)
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
