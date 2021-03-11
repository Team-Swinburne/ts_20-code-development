#ifndef DRIVER_H
#define DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx.h"
#include "stm32f469i_discovery.h"
#include "tft/tft.h"
#include "touchpad/touchpad.h"
#include "system_stm32f4xx.h"
#include "CAN.h"
#include "timer.h"

void SystemClock_Config(void);
void LED_Init(void);
void LED_Array_Control(int LED_Select);
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);
void TIM1_UP_TIM10_IRQHandler(void);
void hw_init(void);
void hw_loop(void);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*DRIVER_H*/
