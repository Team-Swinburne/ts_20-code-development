/*  
*   Header File for Custom CANBUS Driver.
*/
#ifndef CAN_H
#define CAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx.h"
#include "stm32f469i_discovery.h"
#include "system_stm32f4xx.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_can.h"
#include "LEDs.h"
#include <stdbool.h>
#include <math.h>


// CAN IDs
//AMS
#define AMS_HEARTBEAT_ID  0x300 // Not Used.
#define AMS_DATA_ID       0x301

//Throttle
#define THROTTLE_HEARTBEAT_ID 0x304 // Not Used.
#define THROTTLE_SENSORS_ID   0x305 // Not Used.
#define THROTTLE_OUTPUT_ID    0x306
#define THROTTLE_ERRORS_ID    0x307

//Brake
#define BRAKE_SAFETY_ID 0x30A

//Temp Sensor
#define TEMP_SUMMARY_ID 0x311

//Dash
#define DASH_HEARTBEAT_ID 0x320

//Discharge
#define DISCHARGE_DATA_ID 0x340

//Orion
#define ORION_DATA_ID     0x20B	 		//orion battery data message
#define ORION_CURRENT_ID  0x70B			//orion current // Not Used.

//RMS
#define RMS_ID                    0x200         // RMS Base address
#define RMS_TEMPERATURE_SET_1     RMS_ID + 0xA0 // Not Used.
#define RMS_TEMPERATURE_SET_2     RMS_ID + 0xA1
#define RMS_TEMPERATURE_SET_3     RMS_ID + 0xA2
#define RMS_MOTOR_POSITION_INFO   RMS_ID + 0xA5

// Constants
#define VOLT_MIN            380
#define VOLT_MAX            592
#define ACC_TEMP_MIN		0
#define ACC_TEMP_MAX		80
#define	RINEHEART_TEMP_MIN	0
#define	RINEHEART_TEMP_MAX	80
#define	MOTOR_TEMP_MIN		0
#define	MOTOR_TEMP_MAX		80
#define MOTOR_SPEED_MIN		0.0f
#define MOTOR_SPEED_MAX		6000.0f



void CAN_Init(void);
void CAN2_RX0_IRQHandler(void);
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);
void CAN_Transmit_Message(uint32_t TxAddress, uint32_t DLC, uint8_t TxData[8]);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*CAN_H*/
