/*
*   Dashboard Main Hardware Driver.
*   
*   PROGRAM STRUCTURE
*   =================
*   Functions
*     SystemClock_Config                - Hardware System Configuration.
*     hw_init                           - Main Program Hardware Initialisation.
*     hw_loop                           - Main Program Hardware Loop.
*   =================
*/
#include "driver.h"


/*
* stm32cube Framework System Clock Configuration.
* Param:
*   None.
* Usage:
*   Called when hardware is being initiated.
*/
void SystemClock_Config(void) {
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;

    /* Enable Power Control clock */
    __HAL_RCC_PWR_CLK_ENABLE();

    /* The voltage scaling allows optimizing the power consumption when the device is
       clocked below the maximum system frequency, to update the voltage scaling value
       regarding system frequency refer to product datasheet.  */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /*##-1- System Clock Configuration #########################################*/
    /* Enable HSE Oscillator and activate PLL with HSE as source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 360;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    /* Activate the Over-Drive mode */
    HAL_PWREx_EnableOverDrive();

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
       clocks dividers */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

    /*##-2- LTDC Clock Configuration ###########################################*/
    /* LCD clock configuration */
    /* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 MHz */
    /* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAIN = 192 MHz */
    /* PLLLCDCLK = PLLSAI_VCO Output/PLLSAIR = 192/4 = 48 MHz */
    /* LTDC clock frequency = PLLLCDCLK / RCC_PLLSAIDIVR_8 = 48/8 = 6 MHz */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
    PeriphClkInitStruct.PLLSAI.PLLSAIN = 192;
    PeriphClkInitStruct.PLLSAI.PLLSAIR = 4;
    PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_8;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
}


/*
* stm32cube Framework Hardware Initation.
* Param:
*   None.
* Usage:
*   Called by the main file to initiate hardware.
*/
void hw_init(void) {
    // Default HAL library initiation.
    HAL_Init();

  	// Configure the system clock 
  	SystemClock_Config();

  	// Initiate Start-up LEDs
  	BSP_LED_Init(LED1);
    BSP_LED_Init(LED2);
    BSP_LED_Init(LED3);
    BSP_LED_Init(LED4);

    // Start-up indication.
  	for (int i = 0; i < 6; i++) {
  		BSP_LED_Toggle(LED1);
  		HAL_Delay(25);
      BSP_LED_Toggle(LED2);
      HAL_Delay(25);
      BSP_LED_Toggle(LED3);
      HAL_Delay(25);
      BSP_LED_Toggle(LED4);
      HAL_Delay(25);
  	}
    BSP_LED_Off(LED1);
    BSP_LED_Off(LED2);
    BSP_LED_Off(LED3);
    BSP_LED_Off(LED4);

    // Touchscreen Specific Initialisations.
  	tft_init();
  	touchpad_init();

    // Initiate program specific drivers/libraries.
    GPIO_Init();
    LED_Init();
    CAN_Init();
    Timer_Init();
}

/*
* stm32cube Framework Hardware Loop.
* Param:
*   None.
* Usage:
*   Called by the main file to begin hardware infinite loop.
*/
void hw_loop(void) {
  while(1) {
    lv_task_handler();
    HAL_Delay(10);
  }
}
