/*
 * Team Swinburne FSAE 2021
 * Composite Oven Temperature Controller
 * Version 3 (07/03/2021)
 * Thomas Bennett - Lead Autonomous Engineer
 * 
 * Written for an Arduino Uno connected to a bread-board with 
 * opto-isolaters that short a button on a small composties oven.
 */

#define test 0               // uncomment to run the oven as normal
//#define test 1               // uncomment to test the program before connecting it to the oven

//=========================================================================================
// OVEN VARIABLES
//=========================================================================================
// User Variables (edit these)
uint8_t temp_goal   = 112;      // the cooking temperature (celsius)
uint8_t step_up     = 2;        // the amount of degrees (celsius) per minute to ramp up
uint8_t step_down   = 2;        // the amount of degrees (celsius) per minute to ramp down
uint8_t duration    = 4;        // the amount of hours to hold the cooking temperature at

// Oven Settings (only edited when changing the code)
uint8_t up   = 1;
uint8_t down = 0;
unsigned long cooking_duration;              // (CALCULATED)
unsigned long delay_b_press   = 200;         // how long the button is "pressed" in milliseconds
unsigned long delay_led_flash = 500;         // how long an led will be flashed
unsigned long delay_step;                    // (CALCULATED) how long between temperature changes
unsigned long delay_cook      = 10000;       // how long delay is while in the cooking stage
uint8_t temp_inti             = 30;          // the temperature the oven is set to before the button is pressed
uint8_t temp_act;                            // (CHANGES) a variable for the arduino to remember what temperature the oven is (should be) at
uint8_t message;                             // (CHANGES) what "message" for the LED to flash

//=========================================================================================
// Pinout of Arduino
//=========================================================================================
#define LED_power     2
#define LED_indi      3
#define b_temp_set    5
#define b_temp_up     6
#define b_temp_down   7
#define button_go     8

/*
void read_button()                  // debounce -- TO BE WRITTEN!
{
  unsigned long last_debounce_time = 0;
  unsigned long debounce_delay     = 50;
  
  while()
  {
    
  }
}
*/

void flash_LED(uint8_t message)             // to blink without delay or to blink with delay
{
  switch(message) {
    case 0 :                  // flash repeatedly
      // to be written
      break;
    case 1 :                  // flash once
      for(int i = 0; i < message; i++)
      {
        digitalWrite(LED_indi, HIGH);
        delay(delay_led_flash);
        digitalWrite(LED_indi, LOW);
        //delay(delay_led_flash);
      }
      break;
    case 2 :                  // flash twice
      for(int i = 0; i < message; i++)
      {
        digitalWrite(LED_indi, HIGH);
        delay(delay_led_flash);
        digitalWrite(LED_indi, LOW);
        delay(delay_led_flash);
      }
      break;
    case 3 :                  // flash thrice
      for(int i = 0; i < message; i++)
      {
        digitalWrite(LED_indi, HIGH);
        delay(delay_led_flash);
        digitalWrite(LED_indi, LOW);
        delay(delay_led_flash);
      }
      break;
    default : //Optional
      break;
  }
}

void press_button(uint8_t button, uint8_t ramp_step, uint8_t ramp_direction)
{
  Serial.println("TS  -  Pressing buttons");
  flash_LED(1);     // flash indicator LED once
  
  for (int i = 0; i < ramp_step; i++)
  {
    // press the button (either increasing or decreasing the temperature)
    digitalWrite(button, HIGH);
    delay(delay_b_press);
    digitalWrite(button, LOW);         
    //delay(delay_b_press);
 
    // increment the actual temperature (according to the arduino)
    if(ramp_direction == 1)
    {
      temp_act++;
    }
    else if(ramp_direction == 0)
    {
      temp_act--;
    }           
  }

  //set the temperature
  digitalWrite(b_temp_set, HIGH);
  delay(delay_b_press);
  digitalWrite(b_temp_set, LOW);
  //delay(delay_b_press);
  Serial.print("TS - The arduino thinks the set temperature is:   ");
  Serial.println(temp_act);
}

void setup() 
{
  // GPIO setup
  pinMode(LED_power, OUTPUT);  
  digitalWrite(LED_power, LOW);                 // ensure the power LED is off
  pinMode(LED_indi, OUTPUT);
  digitalWrite(LED_indi, LOW);                  // ensure the indication LED is off
  pinMode(b_temp_set, OUTPUT); 
  pinMode(b_temp_up, OUTPUT); 
  pinMode(b_temp_down, OUTPUT);
  pinMode(button_go, INPUT_PULLUP);             // start the cycle when button is pressed
  
  Serial.begin(9600);
  Serial.println("\nHello World!");

  // Calculate numbers for displaying the percentage
  //...
  
  // Calculate the programs variables based for testing or for controlling the oven
  switch(test)
  {
   case 0 :
      cooking_duration = duration*60*60*1000;   // convert the number of hours to milliseconds for delay()
      delay_step       = 60000;                 // one minute
      break;
   case 1 :
      cooking_duration = 60000;                 // one minute of cooking time
      delay_step       = 6000;                  // 6 seconds between temperature changes
      temp_goal   = 40;
      step_up     = 2;
      step_down   = 2;
      break;
   default :
      Serial.println("ERROR:      calculations in setup failed.");
      break;
  }
}

void loop()
{
  digitalWrite(LED_power, HIGH);                          // turn on power LED
  temp_act = temp_inti;                                   // for the arduino to remember the actual temperature, which starts at the defined temperature
  Serial.println("Ready.");
  
  while(true)
  {
    if (digitalRead(button_go) == 0)                      // press the button to begin cooking process (should add a debounce later)
    {
      digitalWrite(LED_indi, LOW);                          // ensure the indicator LED is turn off
      Serial.println("OVEN STAGE:  Heating.");
      while(temp_act < temp_goal)
      {
        press_button(b_temp_up, step_up, up);
        delay(delay_step);
      }
      Serial.println("OVEN STAGE:  Cooking.");
      unsigned long millis_timer = millis();
      while (millis() - millis_timer < cooking_duration)
      {
        delay(delay_cook);                                  // working hard or hardly working? (olnly here for serial print)
        Serial.println("TS  -  Cooking...");
        flash_LED(2);     // flash indicator LED once
      }
      Serial.println("OVEN STAGE:  Cooling.");
      while(temp_act > temp_inti)
      {
        press_button(b_temp_down, step_down, down);
        delay(delay_step);
      }
      digitalWrite(LED_indi, HIGH);                          // turn on indicator LED
    }
  }
}
