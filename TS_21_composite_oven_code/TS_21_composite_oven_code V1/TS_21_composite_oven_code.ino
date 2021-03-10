/*
 * Team Swinburne FSAE 2021
 * Composite Oven Temperature Ramp Controller
 * Verison 1 (06/03/2021)
 * Thomas Bennett - Lead Autonomous Engineer
 */

// User Variables (edit these)
int temp_goal   = 36;      // the cooking temperature (celsius)
int step_up     = 6;       // the amount of degrees (celsius) per minute to ramp up
int step_down   = 6;        // the amount of degrees (celsius) per minute to ramp down
float duration  = 0.167;        // the amount of hours to hold the cooking temperature at

// Global Variables
float ramp_up_delay;
float ramp_down_delay;
float cooking_duration;
int temp_act;
int timer;

// Pinout
#define LED_power     2
#define LED_done      3
#define b_temp_set    5
#define b_temp_up     6
#define b_temp_down   7
#define button_go     8

void set()          // set the temperature
{
  Serial.println("Setting a new temperature.");
    digitalWrite(b_temp_set, HIGH);         // presses the temperature up button
    delay(100);                             // waits for helf a second
    digitalWrite(b_temp_set, LOW);          // releases the temperature up button
    delay(100);                             // waits for half a second
}

void ramp_up()      // ramp up the temperature
{
  Serial.println("Now heating the oven...");
    while(temp_act != temp_goal)
    {
      temp_act++;                           // increment the set temperature
      digitalWrite(b_temp_up, HIGH);        // presses the temperature up button
      delay(100);                           // waits for helf a second
      digitalWrite(b_temp_up, LOW);         // releases the temperature up button
      delay(100);                           // waits for half a second
      set();                                // press the set temperature button
      //Serial.println("Temperature has been changed.");
      delay(ramp_up_delay);                 // wait one minute (plus however long the button pressing takes)
    }
}

void ramp_down()    // bring down the temperature
{
  Serial.println("Now cooling the oven...");

      while(temp_act != 30)
      {
        temp_act--;                             // increment the set temperature
          digitalWrite(b_temp_down, HIGH);        // presses the temperature down button
          delay(100);                           // waits for helf a second
          digitalWrite(b_temp_down, LOW);         // releases the temperature down button
          delay(100);                           // waits for half a second
          set();                                // press the set temperature button
        Serial.println("Temperature has been changed.");
        delay(ramp_down_delay);                         // wait one minute (plus however long the button pressing takes)
      }
  Serial.println("The oven is now cool!");
  digitalWrite(LED_done, HIGH);                 // turn on done LED to let the mechanical monkeys know it is okay to open the magic box.
}


void setup() 
{
  // outputs
  pinMode(LED_power, OUTPUT);
  pinMode(LED_done, OUTPUT); 
  pinMode(b_temp_set, OUTPUT); 
  pinMode(b_temp_up, OUTPUT); 
  pinMode(b_temp_down, OUTPUT);  
  digitalWrite(LED_power, LOW);         // ensure the power LED is off
  digitalWrite(LED_done, LOW);          // ensure the done LED is off

  // inputs
  pinMode(button_go, INPUT_PULLUP);            // start the cycle when button is pressed
  
  // numbers
  temp_act = 30;                    // for the arduino to remember the actual temperature, which starts at and returns to 30 degrees
  timer    = 0;                     // arduino stopwatch to know when to change between stages
  ramp_up_delay    = (60*1000)/(step_up - 1);    // calculating period of each temperature change baased on steps per minute 
  ramp_down_delay  = (1/(step_down*60*1000));  // calculating period of each temperature change baased on steps per minute
  cooking_duration = duration*60*60*1000;  // converting number of hours to milliseconds

  Serial.begin(9600);
  Serial.println("Hello World!");
}

void loop()         // waiting and changing between functions
{
  digitalWrite(LED_power, HIGH);        // turn on power LED
  Serial.println("Ready.");
  while(true)                           // wait until button is pressed (should be integrated into debounce later)
  {
    if (digitalRead(button_go) == 0)    // press the button to begin cooking process (should add a debounce later)
    {
    Serial.println("Lets's cook!");
    ramp_up();                                            // slowly increase the temperature to the temperature goal
    Serial.println("The oven has finished heating.");
    timer = millis();                                     // set a software stop-watch
    while(millis() - timer < cooking_duration)       // wait until the desired cooking duration has elapsed
    {
      delay(1000);
    }
    Serial.println("Cooking finish, beginning to cool.");
    ramp_down();                                           //  slowly decrease the temperature to 30 degrees
    break;
    }
  }
}
