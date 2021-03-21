/*  
*   Header File for Custom GPIO Driver.
*/
#ifndef GPIO_H
#define GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx.h"
#include "stm32f469i_discovery.h"
#include "system_stm32f4xx.h"
#include "stm32f4xx_hal_gpio.h"
#include <stdbool.h>

typedef struct {
  GPIO_TypeDef* GPIO_Port;
  GPIO_InitTypeDef GPIO_InitSt;
} GPIO_Struct;

void GPIO_Init(void);
void GPIO_Pin_Init(GPIO_Struct *gpio_object, char gpioPort, int gpioPin, bool direction);
void GPIO_State(GPIO_Struct *gpio_object, bool state);
void GPIO_Direction(GPIO_Struct *gpio_object, bool direction);



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*GPIO_H*/
