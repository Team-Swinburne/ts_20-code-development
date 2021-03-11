/*
*   Dashboard Custom Timer Driver.
*   
*   PROGRAM STRUCTURE
*   =================
*   Declarations
*     External CAN Heartbeat Variables.
*     Heartbeat Timer.
*   -----------------
*   Functions
*     Timer_Init                - Initialise the HAL Timer.
*     TIM1_UP_TIM10_IRQHandler  - Timer Interrupt Callback (Every Second).
*   -----------------
*/
#include "timer.h"

extern int heartbeat_counter; // Originally defined in the CAN.c file.
extern int16_t heartbeat_state; // Originally defined in the CAN.c file.
TIM_HandleTypeDef heartbeat_timer;


/*
* Hearbeat Timer Initialisation Procedure.
* Param:
*   None.
* Usage:
*   Used to initiate the timer used for hearbeat ticking.
*/
void Timer_Init() {
  // Enable the clock for the timer.
  __HAL_RCC_TIM1_CLK_ENABLE();
  
  // Hearbeat interrupt Initialisation
  HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* Configuring and enabling the timer.
  * Note that the prescaler and period determine when to count,
  * through trial and error, I managed to get it to tick every second with
  * the below configuration.
  */
  heartbeat_timer.Instance = TIM1;
  heartbeat_timer.Init.Prescaler = 15999;
  heartbeat_timer.Init.CounterMode = TIM_COUNTERMODE_UP;
  heartbeat_timer.Init.Period = 11249;
  heartbeat_timer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  heartbeat_timer.Init.RepetitionCounter = 0;
  HAL_TIM_Base_Init(&heartbeat_timer);
  
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource(&heartbeat_timer, &sClockSourceConfig);

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(&heartbeat_timer, &sMasterConfig);

  // Start the timer.
  HAL_TIM_Base_Start_IT(&heartbeat_timer);
}


/*
* Hearbeat Timer Interrupt Handler.
* Param:
*   None.
* Usage:
*   Automatically called according to the timer configuration, i.e. every second, to send a CAN message with the heartbeat information.
*/
void TIM1_UP_TIM10_IRQHandler(void) {
  // Handler the interrupt according to HAL.
  HAL_TIM_IRQHandler(&heartbeat_timer);

  // Increment the heartbeat.
  heartbeat_counter++;

  // The heartbeat data to transmit.
  uint8_t TxData[8];
  TxData[0] = heartbeat_state;
  TxData[1] = heartbeat_counter;

  // Transmit the data.
  CAN_Transmit_Message(DASH_HEARTBEAT_ID, 2, TxData);
  
}