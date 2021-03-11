/*
*   Dashboard Custom LED Driver Used For Setting Up The Backboard LEDs.
*   
*   PROGRAM STRUCTURE
*   =================
*	Definitions
*		LED Variables
*		Backboard LED Strip Arrays
*	-----------------
*   Functions
*     LED_Init 			- Configure the specified LEDS.
*     LED_Array_Control - Control the LED Strip on the backboard.
*   -----------------
*/
#include "LEDs.h"

GPIO_Struct PDOC_led, AMS_led, IMD_led, BSPD_led, multi1_led, multi2_led, multi3_led, multi4_led;

// LED Array controls used by the LED array at the top of the board.
const int LED_ARRAY_DIRECTIONS[12][4] = {
	{1,1,0,0},
	{1,1,0,0},
	{1,0,1,0},
	{1,0,1,0},
	{0,1,1,0},
	{1,0,0,1},
	{0,1,1,0},
	{1,0,0,1},
	{0,1,0,1},
	{0,1,0,1},
	{0,0,1,1},
	{0,0,1,1}
};
const bool LED_ARRAY_OUTPUTS[12][4] = {
	{false,true,false,false},
	{true,false,false,false},
	{false,false,true,false},
	{true,false,false,false},
	{false,false,true,false},
	{false,false,false,true},
	{false,true,false,false},
	{true,false,false,false},
	{false,false,false,true},
	{false,true,false,false},
	{false,false,false,true},
	{false,false,true,false}
};


/*
* Disco Backboard LED initialisation.
* Param:
*   None.
* Usage:
*   Called when hardware is being initiated.
*/
void LED_Init() {
  GPIO_Pin_Init(&AMS_led, 'G', 13, true);
  GPIO_Pin_Init(&PDOC_led, 'A', 1, true);
  GPIO_Pin_Init(&IMD_led, 'G', 14, true);
  GPIO_Pin_Init(&BSPD_led, 'G', 9, true);
  GPIO_Pin_Init(&multi1_led, 'B', 14, true);
  GPIO_Pin_Init(&multi2_led, 'B', 15, true);
  GPIO_Pin_Init(&multi3_led, 'H', 6, true);
  GPIO_Pin_Init(&multi4_led, 'A', 7, true);
}


/*
* LED Array control procedure.
* Param:
*   int LED_Select, used to select which of the one LEDs you wish to turn on in the LED array.
* Usage:
*   Used for turning on LED array, e.g.
*   ===================
*   oooo oooo oooo oooo
*   ===================
*   
*   led_array_control(5);
*
*   ===================
*   oooo xooo oooo oooo
*   ===================
*/
void LED_Array_Control(int LED_Select) {
  if(LED_ARRAY_DIRECTIONS[LED_Select][0] == 1)
	{
    GPIO_Direction(&multi1_led,true);
    GPIO_State(&multi1_led,LED_ARRAY_OUTPUTS[LED_Select][0]);
	}else
	{
		GPIO_Direction(&multi1_led,false);
	}
	if(LED_ARRAY_DIRECTIONS[LED_Select][1] == 1)
	{
    GPIO_Direction(&multi2_led,true);
    GPIO_State(&multi2_led,LED_ARRAY_OUTPUTS[LED_Select][1]);
	}else
	{
    GPIO_Direction(&multi2_led,false);
	}
	if(LED_ARRAY_DIRECTIONS[LED_Select][2] == 1)
	{
		GPIO_Direction(&multi3_led,true);
    GPIO_State(&multi3_led,LED_ARRAY_OUTPUTS[LED_Select][2]);
	}else
	{
		GPIO_Direction(&multi3_led,false);
	}
	if(LED_ARRAY_DIRECTIONS[LED_Select][3] == 1)
	{
		GPIO_Direction(&multi4_led,true);
    GPIO_State(&multi4_led,LED_ARRAY_OUTPUTS[LED_Select][3]);
	}else
	{
		GPIO_Direction(&multi4_led,false);
	}
}

