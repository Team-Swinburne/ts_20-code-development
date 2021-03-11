/*  
*   Header File for Custom LEDs Driver.
*/
#ifndef LEDS_H
#define LEDS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx.h"
#include "stm32f469i_discovery.h"
#include "system_stm32f4xx.h"
#include "gpio.h"
#include <stdbool.h>

void LED_Init(void);
void LED_Array_Control(int LED_Select);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LEDS_H*/
