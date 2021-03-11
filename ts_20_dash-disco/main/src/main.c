/**
  ******************************************************************************
  * @file    main.c
  * @author  Andrew Gray, Christian Lazarovski, Tansel Kahrahman
  * @version V1.1
  * @date    02-09-2020
  * @brief   Default main function with splashscreen and menu call.
  ******************************************************************************
*/

//test

#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "lvgl.h"
#include "driver.h"

#include "menu.h"
#include "splash.h"

#include "screen1.h"
#include "screen2.h"
#include "screen3.h"

#include "common.h"

int wait_counter = 0;
lv_task_t * wait_iterator_task;

//this function iterates a timer oncer per second 
void wait_iterator(lv_task_t * t){
  wait_counter++;
}

int main(void)
{
	lv_init();
	hw_init();

  //init the timer task
  wait_iterator_task = lv_task_create(wait_iterator,1000,LV_TASK_PRIO_MID,NULL);
  
  //load the splash screen
  load_splash();

  //wait for 5 seconds, this works for sim and stm board
  while(wait_counter < 5){
    lv_task_handler();
  }

  //delete the iterator task from counting
  lv_task_del(wait_iterator_task);

  //init the menu
  menuInit(lv_theme_night_init(63488, NULL)); 
  hw_loop();

  //delay hardware app layer of stm32 board for splash to show
  /*
  for (int i = 0; i < 2000; i++) {
    lv_task_handler();
    HAL_Delay(1);
  }*/

  /*for (int i = 0; i < 2000; i++) {
    lv_task_handler();
    usleep(1000);
  }*/

  //runs the main menu, the hub of our dashboard.
  //menuInit(lv_theme_night_init(63488, NULL));   
	//hw_loop();

  return 0;
}
