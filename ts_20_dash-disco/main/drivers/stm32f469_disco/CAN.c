/*
*   Dashboard Custom CANBUS Driver.
*   
*   PROGRAM STRUCTURE
*   =================
*   Declarations
*     CAN Variables
*     Program Variables
*     External LED Variables
*   -----------------
*   Functions
*     CAN_Init                          - Initialise CAN Bus Communication.
*     CAN2_RX0_IRQHandler               - CAN Bus Interrupt Handler/Redirector.
*     HAL_CAN_RxFifo0MsgPendingCallback - CAN Bus Interrupt Callback Function.
*     CAN_Transmit_Message              - Transmit Message.
*   -----------------
*/
#include "CAN.h"

// CAN Variables
CAN_HandleTypeDef hcan2;
CAN_FilterTypeDef sFilterConfig;
CAN_TxHeaderTypeDef TxHeader;
CAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[8];
uint32_t TxMailbox = CAN_TX_MAILBOX0;

// Program Variables
bool precharge_pressed = 0;
bool drive_pressed = 0;
bool apps_disagree = 0;
bool trailbraking_active = 0;

int ams_state = 0;
int heartbeat_counter = 0;

int16_t	motor_speed = 0;
int16_t heartbeat_state = 0;

uint16_t accum_lowest_voltage = 0;
uint16_t motor_highest_temp = 0;
uint16_t rineheart_highest_temp = 0;

float max_accum_temp = 0;

// LEDs used by the disco backboard.
extern GPIO_Struct PDOC_led, AMS_led, IMD_led, BSPD_led, multi1_led, multi2_led, multi3_led, multi4_led; // Originally defined in the LEDs.c file.


/*
* CAN Initialisation.
* Param:
*   None.
* Usage:
*   Called when hardware is being initiated.
*/
void CAN_Init() {

  /* NOTE:
  * The touchscreen disables the use of CAN1, hence we must use CAN2.
  * CAN1 is considered the MASTER on the board and defines configurations for other CANS, i.e. CAN2,
  * and so, we must enable it as well.
  */

  // Enable CAN1 & CAN2 Clock.
  __CAN1_CLK_ENABLE();
  __CAN2_CLK_ENABLE();

  // CAN2 GPIO DEFINITIONS: Rx Pin is B5, Tx Pin is B13.
  // Defining and enabling CAN2 Rx pin.
  GPIO_InitTypeDef GPIO_InitStruct_Rx2;
  GPIO_InitStruct_Rx2.Pin = GPIO_PIN_5;
  GPIO_InitStruct_Rx2.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct_Rx2.Pull = GPIO_NOPULL;
  GPIO_InitStruct_Rx2.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct_Rx2.Alternate = GPIO_AF9_CAN2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct_Rx2);

  // Defining and enabling CAN2 Tx pin.
  GPIO_InitTypeDef GPIO_InitStruct_Tx2;
  GPIO_InitStruct_Tx2.Pin = GPIO_PIN_13;
  GPIO_InitStruct_Tx2.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct_Tx2.Pull = GPIO_NOPULL;
  GPIO_InitStruct_Tx2.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct_Tx2.Alternate = GPIO_AF9_CAN2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct_Tx2);

  // Enabling CAN Rx interrupts and setting their priority.
  HAL_NVIC_SetPriority(CAN2_RX0_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(CAN2_RX0_IRQn);
 
  // Setting up CAN2 configuration.
  hcan2.Instance = CAN2;
  hcan2.Init.Prescaler = 5;
  hcan2.Init.Mode = CAN_MODE_NORMAL;
  hcan2.Init.SyncJumpWidth = CAN_SJW_4TQ;
  hcan2.Init.TimeSeg1 = CAN_BS1_15TQ;
  hcan2.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan2.Init.TimeTriggeredMode = DISABLE;
  hcan2.Init.AutoBusOff = DISABLE;
  hcan2.Init.AutoWakeUp = DISABLE;
  hcan2.Init.AutoRetransmission = DISABLE;
  hcan2.Init.ReceiveFifoLocked = DISABLE;
  hcan2.Init.TransmitFifoPriority = DISABLE;

  // Configure the CAN filter (No effect in our case).
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  sFilterConfig.FilterIdHigh = 0x0000;
  sFilterConfig.FilterIdLow = 0x0000;
  sFilterConfig.FilterMaskIdHigh = 0x0000;
  sFilterConfig.FilterMaskIdLow = 0x0000;
  sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  sFilterConfig.FilterActivation = ENABLE;
  sFilterConfig.FilterBank = 14;
  sFilterConfig.SlaveStartFilterBank = 14;
  
  //Initialise CAN.
  if ((HAL_CAN_Init(&hcan2) == HAL_OK) ) {
    // Indicate onboard LED4 when CAN has initalised.
    BSP_LED_Off(LED4);
  } 
  
  // Start the CAN communication and configure the filter.
  if ((HAL_CAN_Start(&hcan2) == HAL_OK) && (HAL_CAN_ConfigFilter(&hcan2, &sFilterConfig) == HAL_OK)) {
    // Indicate onboard LED when CAN has started with the filter.
    BSP_LED_On(LED3);
  }

  // Enable Rx notifications.
  if ((HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING) == HAL_OK)) {
    // Indicate onboard LED when CAN notifcations have been enabled.
    BSP_LED_On(LED2);
  }
}

/*
* HAL CAN Interrupt Redirection Procedure.
* Param:
*   None.
* Usage:
*   Automatically called when an interrupt is detected on the CAN2 Rx pin.
*   Redirects program to the interrupt callback function.
*/
void CAN2_RX0_IRQHandler(void) { 
  HAL_CAN_IRQHandler(&hcan2);
}


/*
* HAL CAN Interrupt Callback Function.
* Param:
*   CAN_HandleTypeDef pointer, determines which CAN line is calling the interrupt.
*   (Always CAN2 in our case)
* Usage:
*   Automatically called by the interrupt redirection procedure.
*/
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
  
  // Toggle onboard LED to show CAN traffic.
  BSP_LED_Toggle(LED4);

  // Get Rx message - We do not need to check if it exists as the interrupt implies that something has arrived.
  HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData);

  // Determine how to handle the data according to its ID.
  switch(RxHeader.StdId) {
    case AMS_DATA_ID:
      ams_state = RxData[0];
      GPIO_State(&AMS_led, RxData[1]);
      GPIO_State(&IMD_led, RxData[5]);
      break;

    case THROTTLE_OUTPUT_ID:
      precharge_pressed = RxData[1];
      drive_pressed = RxData[2];
      break;

    case THROTTLE_ERRORS_ID:
      apps_disagree = RxData[0];
      trailbraking_active = RxData[1];
      break;

    case TEMP_SUMMARY_ID:   
      max_accum_temp = (float)RxData[1];
      break;

    case ORION_DATA_ID: 
      if ((heartbeat_counter > 2) && (RxHeader.DLC == 7)) {
        accum_lowest_voltage = RxData[5] | (RxData[4] << 8);
      }
			break;

    case RMS_TEMPERATURE_SET_2:
      rineheart_highest_temp = (RxData[0] | (RxData[1] << 8)) / 10;
      break;
    
    case RMS_TEMPERATURE_SET_3:
      motor_highest_temp = (RxData[4] | (RxData[5] << 8)) / 10;
			break;
    
    case RMS_MOTOR_POSITION_INFO:
      motor_speed = RxData[2] | (RxData[3] << 8);
      LED_Array_Control(round(((motor_speed - MOTOR_SPEED_MIN) / MOTOR_SPEED_MAX) * 11.0f));
			break;
    
    case DISCHARGE_DATA_ID:
      if(RxData[0] == 0) {
        GPIO_State(&PDOC_led, true);
      } else {
        GPIO_State(&PDOC_led, false);
      }
			break;
    
    case BRAKE_SAFETY_ID:
				GPIO_State(&PDOC_led, RxData[4]);
			break;
  }
}



/*
* CAN Transmit Message.
* Param:
*   uint32_t TxAddress, The address we wish to transmit the message to (should be defined in driver.c under CAN IDs).
*   uint32_t DLC, the length of the message.
*   uint8_t TxData[8], the data to be sent.
* Usage:
*   Called by heartbeat timer interrupt function to send a message every second.
*   Can be used to send other messages in the future.
*/
void CAN_Transmit_Message(uint32_t TxAddress, uint32_t DLC, uint8_t TxData[8]) {
  // Define the TxHeader for transmitting the heartbeat CAN message.
  TxHeader.StdId = TxAddress;
  TxHeader.IDE = CAN_ID_STD;
  TxHeader.RTR = CAN_RTR_DATA;
  TxHeader.DLC = DLC;
  TxHeader.TransmitGlobalTime = DISABLE;
    
  // Transmit the data.
  HAL_CAN_AddTxMessage(&hcan2, &TxHeader, TxData, &TxMailbox);

  // Wait for the transmission to complete.
  while(HAL_CAN_GetTxMailboxesFreeLevel(&hcan2) != 3) {}
}