/**
  ******************************************************************************
  * @file    screen1.c
  * @author  Andrew Gray, Christian Lazarovski, Tansel Kahrahman
  * @version V1.1
  * @date    02-09-2020
  * @brief   Main Screen #1.
  ******************************************************************************
*/

/*********************
 *      INCLUDES
 *********************/

#include "lvgl.h"
#include "lv_conf.h"

#include "splash.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

LV_IMG_DECLARE(ts20splash)

static lv_style_t cont_colour;

void load_splash()
{
    lv_obj_t * scr = lv_cont_create(NULL, NULL); //creates the screen scr
    lv_obj_set_size(scr, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));

    lv_style_copy(&cont_colour, lv_cont_get_style(scr, LV_CONT_STYLE_MAIN));
    cont_colour.body.main_color = LV_COLOR_BLACK;
    cont_colour.body.grad_color = LV_COLOR_BLACK;
    lv_cont_set_style(scr, LV_CONT_STYLE_MAIN, &cont_colour);
    lv_disp_load_scr(scr);

    lv_obj_t * wp = lv_img_create(scr, NULL);
    lv_img_set_src(wp, &ts20splash);
    //lv_obj_set_height(wp, 400);
    //lv_obj_set_width(wp, 400);
    //lv_obj_set_pos(wp, 0, 0);
    lv_obj_align(wp, scr, LV_ALIGN_CENTER, 0, 0);


}