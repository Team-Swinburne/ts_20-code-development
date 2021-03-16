/*  
*   Header File for Timer GPIO Driver.
*/
#ifndef TIMER_H
#define TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx.h"
#include "stm32f469i_discovery.h"
#include "system_stm32f4xx.h"
#include "stm32f4xx_hal_rcc.h"
#include "CAN.h"


void Timer_Init(void);
void TIM1_UP_TIM10_IRQHandler(void);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*TIMER_H*/
