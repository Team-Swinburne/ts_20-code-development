/**
  ******************************************************************************
  * @file    menu.c
  * @author  Andrew Gray, Christian Lazarovski, Tansel Kahrahman
  * @version V1.3
  * @date    14-10-2020
  * @brief   Menu screens, intitial screen on startup with important information
  * regarding hardware status.
  ******************************************************************************
*/

/*********************
 *      INCLUDES
 *********************/
#include "menu.h"
#include "screen1.h"
#include "screen2.h"
#include "screen3.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/*********************
 *      DEFINES
 *********************/

/*********************
 *      EXTERN
 *      VARIABLES
 *********************/
// FROM driver.c
extern int ams_state;
extern bool precharge_pressed;
extern bool drive_pressed;
extern float max_accum_temp;
extern uint16_t accum_lowest_voltage;
extern uint16_t motor_highest_temp;
extern uint16_t rineheart_highest_temp;

//FROM common.c
extern lv_obj_t * driveWarningLine;
extern lv_obj_t * prechargeWarningLine;
extern lv_style_t style_line;
extern lv_point_t line_points[];
extern lv_obj_t * header;
extern lv_obj_t * ams_label;

extern lv_task_t * gauge_handler_task;
extern lv_task_t * can_iterator_task;
extern lv_task_t * can_info_task;

extern lv_obj_t * header;

extern lv_obj_t * motor_bar;
extern lv_obj_t * motor_temp_value;

extern lv_obj_t * rineheart_bar;
extern lv_obj_t * rineheart_temp_label;

extern lv_obj_t * accum_temp;
extern lv_obj_t * accum_temp_label;

extern lv_obj_t * accum_volt;
extern lv_obj_t * accum_volt_label;
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void create_tab1(lv_obj_t * parent);
static void create_tab2(lv_obj_t * parent);
static void create_tab3(lv_obj_t * parent);

static void navButton1Handler(lv_obj_t * obj, lv_event_t event);
static void navButton2Handler(lv_obj_t * obj, lv_event_t event);
static void navButton3Handler(lv_obj_t * obj, lv_event_t event);

//FROM common.c
extern void gauge_handler(lv_task_t * task);
extern void can_iterator(lv_task_t * task);
extern void can_info_handler(lv_task_t * task);
extern void draw_precharge_warning();
extern void draw_drive_warning();
extern void header_tab_create();

static void motor_bar_colour(lv_task_t * motor_bar_colour_task);
static void rine_bar_colour(lv_task_t * rine_bar_colour_task);
static void accum_t_bar_colour(lv_task_t * accum_t_bar_colour_task);
static void accum_v_bar_colour(lv_task_t * accum_v_bar_colour_task);
/**********************
 *  STATIC VARIABLES
 **********************/
static lv_task_t * motor_bar_colour_task;
static lv_task_t * rine_bar_colour_task;
static lv_task_t * accum_t_bar_colour_task;
static lv_task_t * accum_v_bar_colour_task;

static lv_obj_t * traction_switch_label;
static lv_obj_t * torque_switch_label;

static lv_obj_t * ddlist;

static lv_theme_t * th;
static lv_style_t h_style;

static lv_style_t motor_colour;
static lv_style_t rine_colour;
static lv_style_t accum_t_colour;
static lv_style_t accum_v_colour;

static uint16_t ddlist_value;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * @param th pointer to a theme
 */
void menuInit(lv_theme_t * th)
{   //INITALISES and creates the menu.
    lv_style_copy(&h_style, &lv_style_transp);
    h_style.body.padding.inner = LV_DPI / 10;
    h_style.body.padding.left = LV_DPI / 4; 
    h_style.body.padding.right = LV_DPI / 4;
    h_style.body.padding.top = LV_DPI / 10;
    h_style.body.padding.bottom = LV_DPI / 10;
    h_style.text.font = &lv_font_roboto_22;
    h_style.text.color = LV_COLOR_WHITE;
    
    //BEGIN SCREEN SETUP
    lv_theme_set_current(th);
    th = lv_theme_get_current();    
    lv_obj_t * scr = lv_cont_create(NULL, NULL); //creates the screen scr
    lv_disp_load_scr(scr);

    //creates the tabview
    lv_obj_t * tv = lv_tabview_create(scr, NULL);
    lv_obj_set_size(tv, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
    lv_obj_set_pos(tv, 0,50);
    //create the tabs
    lv_obj_t * tab1 = lv_tabview_add_tab(tv, "Home Screen"); //tab1.
    lv_obj_t * tab2 = lv_tabview_add_tab(tv, "Nav and TV Control");
    lv_obj_t * tab3 = lv_tabview_add_tab(tv, "Nav Buttons for Testing");

    //END SCREEN SETUP
    lv_tabview_set_btns_hidden(tv, true); //tab buttons are hidden
                                          //swiping to get between tabs.

    //BEGIN SCREEN CONTENT.
    header_tab_create();
    create_tab1(tab1); //each tab uses 
    create_tab2(tab2); //it's own function.
    create_tab3(tab3);
    //END SCREEN CONTENT
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void create_tab1(lv_obj_t * parent) //TAB 1 CREATION.
{ 
    //creates a container "h". This becomes the parent object for all of our widgets.
    lv_obj_t * h = lv_cont_create(parent, NULL); 
    lv_obj_set_style(h, &h_style);
    lv_obj_set_click(h, false);
    lv_cont_set_fit(h, LV_FIT_TIGHT);
    lv_cont_set_layout(h, LV_LAYOUT_COL_M);
    lv_obj_align(h, parent, LV_ALIGN_IN_TOP_LEFT, 200, 20);

    // Padding used by all bars.
    lv_style_t style_padding, style_padding2;
    style_padding.body.padding.bottom = style_padding.body.padding.top = style_padding.body.padding.left = style_padding.body.padding.right = 10; // Horizontal bars
    style_padding2.body.padding.bottom = style_padding2.body.padding.top = style_padding2.body.padding.left = style_padding2.body.padding.right = 13; // Vertical bar (accum voltage)
    


    lv_obj_t * motorTempLabel = lv_label_create(h,NULL);
    lv_label_set_text(motorTempLabel,"MOTOR TEMP");
    lv_obj_set_style(motorTempLabel, &h_style);

    motor_bar = lv_bar_create(h, NULL);
    //Sets the styling.    
    lv_style_copy(&motor_colour, lv_bar_get_style(motor_bar, LV_BAR_STYLE_INDIC));
    motor_colour.body.padding = style_padding.body.padding; // Apply paddding onto bar from padding set above.
    
    lv_bar_set_range(motor_bar, 0, 80);
    lv_bar_set_anim_time(motor_bar, 500);
    lv_bar_set_value(motor_bar, 0, LV_ANIM_ON);
    lv_obj_set_size(motor_bar, 300, 60);
    lv_bar_set_style(motor_bar, LV_BAR_STYLE_INDIC, &motor_colour);

    motor_temp_value = lv_label_create(parent, NULL);
    lv_label_set_text(motor_temp_value, "0 C");
    lv_obj_align(motor_temp_value, motor_bar, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

    lv_obj_t * rineheart_label = lv_label_create(h,NULL);
    lv_label_set_text(rineheart_label,"RINEHEART TEMP");
    lv_obj_set_style(rineheart_label, &h_style);

    rineheart_bar = lv_bar_create(h, NULL);
    lv_style_copy(&rine_colour, lv_bar_get_style(rineheart_bar, LV_BAR_STYLE_INDIC));
    rine_colour.body.padding = style_padding.body.padding; // Apply paddding onto bar from padding set above.

    lv_bar_set_range(rineheart_bar, 0, 80); //max val 80
    lv_bar_set_anim_time(rineheart_bar, 500);
    lv_bar_set_value(rineheart_bar, 0, LV_ANIM_ON);
    lv_obj_set_size(rineheart_bar, 300, 60);
    lv_bar_set_style(rineheart_bar, LV_BAR_STYLE_INDIC, &rine_colour);

    rineheart_temp_label = lv_label_create(parent, NULL);
    lv_label_set_text(rineheart_temp_label, "0C");
    lv_obj_align(rineheart_temp_label, rineheart_bar, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

    lv_obj_t * accum_label = lv_label_create(h,NULL);
    lv_label_set_text(accum_label,"ACCUMULATOR TEMP");
    lv_obj_set_style(accum_label, &h_style);

    accum_temp = lv_bar_create(h,NULL);
    lv_style_copy(&accum_t_colour, lv_bar_get_style(accum_temp, LV_BAR_STYLE_INDIC));
    accum_t_colour.body.padding = style_padding.body.padding; // Apply paddding onto bar from padding set above.    

    lv_bar_set_range(accum_temp, 0, 80);
    lv_bar_set_anim_time(accum_temp, 500);
    lv_bar_set_value(accum_temp, 0, LV_ANIM_ON);
    lv_obj_set_size(accum_temp, 300, 60);
    lv_bar_set_style(accum_temp, LV_BAR_STYLE_INDIC, &accum_t_colour);

    accum_temp_label = lv_label_create(parent, NULL);
    lv_label_set_text(accum_temp_label, "0C");
    lv_obj_align(accum_temp_label, accum_temp, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

    //vertical container for the accumulator voltage.
    lv_obj_t * vert_container = lv_cont_create(parent, NULL); 
    lv_obj_set_style(vert_container, &h_style);
    lv_obj_set_click(vert_container, false);
    lv_cont_set_fit(vert_container, LV_FIT_TIGHT);
    lv_cont_set_layout(vert_container, LV_LAYOUT_COL_M);
    lv_obj_align(vert_container, parent, LV_ALIGN_IN_TOP_LEFT, 550, 20);

    lv_obj_t * accum_vert_label = lv_label_create(vert_container,NULL);
    lv_label_set_text(accum_vert_label,"ACCUMULATOR VOLTS");
    lv_obj_set_style(accum_vert_label, &h_style);

    accum_volt = lv_bar_create(vert_container,NULL);
    lv_style_copy(&accum_v_colour, lv_bar_get_style(accum_volt, LV_BAR_STYLE_INDIC));
    accum_v_colour.body.padding = style_padding2.body.padding; // Apply paddding onto bar from padding set above. 

    lv_bar_set_range(accum_volt, 0, 600);
    lv_bar_set_anim_time(accum_volt, 500);
    lv_bar_set_value(accum_volt, 0, LV_ANIM_ON);
    lv_obj_set_size(accum_volt, 80, 260);
    lv_bar_set_style(accum_volt, LV_BAR_STYLE_INDIC, &accum_v_colour);

    accum_volt_label = lv_label_create(parent, NULL);
    lv_label_set_text(accum_volt_label,"0V");
    lv_obj_align(accum_volt_label, accum_volt, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

    warning_lines(); //call from common.c

    //TASK CREATION.
    gauge_handler_task = lv_task_create(gauge_handler,100,LV_TASK_PRIO_HIGH,NULL);
    can_iterator_task = lv_task_create(can_iterator,1000,LV_TASK_PRIO_MID,NULL);
    can_info_task = lv_task_create(can_info_handler,100,LV_TASK_PRIO_MID,NULL);

    motor_bar_colour_task = lv_task_create(motor_bar_colour, 100, LV_TASK_PRIO_HIGH, NULL);
    rine_bar_colour_task = lv_task_create(rine_bar_colour, 100, LV_TASK_PRIO_HIGH, NULL);
    accum_t_bar_colour_task = lv_task_create(accum_t_bar_colour, 100, LV_TASK_PRIO_HIGH, NULL);
    accum_v_bar_colour_task = lv_task_create(accum_v_bar_colour, 100, LV_TASK_PRIO_HIGH, NULL);
}

static void create_tab2(lv_obj_t * parent) //this is gonna have our nav buttons.
{
    //Sets the styling.
    lv_page_set_scrl_layout(parent, LV_LAYOUT_PRETTY);

    //creates a container "h". This becomes the parent object for all of our widgets.
    lv_obj_t * h = lv_cont_create(parent, NULL); 
    lv_obj_set_style(h, &h_style);
    lv_obj_set_click(h, false);
    lv_cont_set_fit(h, LV_FIT_TIGHT);
    lv_cont_set_layout(h, LV_LAYOUT_COL_M);
    lv_obj_align(h, parent, LV_ALIGN_IN_TOP_LEFT, 0, 0);

    lv_obj_t * traction_label = lv_label_create(h, NULL);
    lv_label_set_text(traction_label, "TRACTION CONTROL");
    lv_obj_set_style(traction_label, &h_style);

    /* Create a slider in the center of the display */
    lv_obj_t * traction_switch = lv_sw_create(h, NULL);
    lv_obj_set_width(traction_switch, LV_DPI * 1.2);

    lv_obj_t * torque_label = lv_label_create(h, NULL);
    lv_label_set_text(torque_label, "TORQUE VECTORING");
    lv_obj_set_style(torque_label, &h_style);

    /* Create a slider in the center of the display */
    lv_obj_t * torque_switch = lv_sw_create(h, NULL);
    lv_obj_set_width(torque_switch, LV_DPI * 1.2);

    lv_obj_t * ddlist_label = lv_label_create(h, NULL);
    lv_label_set_text(ddlist_label, "EVENT SELECT");
    lv_obj_align(ddlist_label, ddlist, LV_ALIGN_OUT_TOP_MID, 0, 0);
    lv_obj_set_style(ddlist_label, &h_style);

    /*Create a drop down list*/
    ddlist = lv_ddlist_create(h, NULL);
    lv_ddlist_set_options(ddlist, "Acceleration\nSkid Pan\nAuto Cross\nEndurance");

    lv_ddlist_set_fix_width(ddlist, 150);
    lv_ddlist_set_fix_height(ddlist, 150);
    lv_ddlist_set_draw_arrow(ddlist, true);

    /* Enable auto-realign when the size changes.
     * It will keep the bottom of the ddlist fixed*/
    lv_obj_set_auto_realign(ddlist, true);

    /*It will be called automatically when the size changes*/
    lv_obj_align(ddlist, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -0);

    lv_obj_t * navButton1 = lv_btn_create(h,NULL);
    lv_obj_set_event_cb(navButton1, navButton1Handler);
    lv_obj_t * navButton1Label = lv_label_create(navButton1,NULL);
    lv_label_set_text(navButton1Label,"DRIVE");

}

static void create_tab3(lv_obj_t * parent)
{
    //Sets the styling.
    lv_page_set_scrl_layout(parent, LV_LAYOUT_PRETTY);
    
    //creates a container "h". This becomes the parent object for all of our widgets.
    lv_obj_t * h = lv_cont_create(parent, NULL); 
    lv_obj_set_style(h, &h_style);
    lv_obj_set_click(h, false);
    lv_cont_set_fit(h, LV_FIT_TIGHT);
    lv_cont_set_layout(h, LV_LAYOUT_COL_M);
    lv_obj_align(h, parent, LV_ALIGN_IN_TOP_LEFT, 0, 0);

    lv_obj_t * navButton1 = lv_btn_create(h,NULL);
    lv_obj_set_event_cb(navButton1, navButton1Handler);//Give the button a function
    lv_obj_t * navButton1Label = lv_label_create(navButton1,NULL);
    lv_label_set_text(navButton1Label,"To Screen1.c");

    lv_obj_t * navButton2 = lv_btn_create(h,NULL);
    lv_obj_set_event_cb(navButton2, navButton2Handler);//Give the button a function
    lv_obj_t * navButton2Label = lv_label_create(navButton2,NULL);
    lv_label_set_text(navButton2Label,"To Screen2.c");

    lv_obj_t * navButton3 = lv_btn_create(h,NULL);
    lv_obj_set_event_cb(navButton3, navButton3Handler); //Give the button a function
    lv_obj_t * navButton3Label = lv_label_create(navButton3,NULL);
    lv_label_set_text(navButton3Label,"To Screen3.c");
}

static void navButton1Handler(lv_obj_t * obj, lv_event_t event)
{
    lv_obj_t * currentScreen = lv_scr_act(); //gets the screen.
    if (event == LV_EVENT_RELEASED)
    {
        ddlist_value = lv_ddlist_get_selected(ddlist);
        
        lv_task_del(can_iterator_task); //tasks need to be deleted too.
        lv_task_del(gauge_handler_task);
        lv_obj_del(currentScreen);  //literally just deletes the screen.

        switch (ddlist_value)
        {
        case 0:
            screen1Init(lv_theme_night_init(63488, NULL));
            break;
        
        case 1:
            screen2Init(lv_theme_night_init(63488, NULL));
            break;

        case 2:
            screen2Init(lv_theme_night_init(63488, NULL));
            break;

        case 3:
            screen3Init(lv_theme_night_init(63488, NULL));
            break;
        
        default:
            break;
        }
    }
}

static void navButton2Handler(lv_obj_t * obj, lv_event_t event)
{
    lv_obj_t * currentScreen = lv_scr_act(); //gets the screen.
    if ( event == LV_EVENT_RELEASED)
    {
        lv_task_del(can_iterator_task);
        lv_task_del(gauge_handler_task);
        lv_obj_del(currentScreen);  //literally just deletes the screen.
        screen2Init(lv_theme_night_init(63488, NULL));
    }
}

static void navButton3Handler(lv_obj_t * obj, lv_event_t event)
{
    lv_obj_t * currentScreen = lv_scr_act(); //gets the screen.
    if ( event == LV_EVENT_RELEASED)
    {
        lv_task_del(can_iterator_task);
        lv_task_del(gauge_handler_task);
        lv_obj_del(currentScreen);  //literally just deletes the screen.
        screen3Init(lv_theme_night_init(63488, NULL));
    }
}

/*
* NOTE: THE FOLLOWING BAR
* COLOURS USE HARD CODED MAX VALUES
* TO FIND THE PERCENTAGE
* THESE (as well as the bar values)
* themselves, need to be changed
* if these values change. */
static void motor_bar_colour(lv_task_t * t)
{
    float percentage = lv_bar_get_value(motor_bar);
    percentage = percentage/80;
    percentage = percentage*100;
    printf("Percentage = %f",percentage);

    if (percentage >= 90)
    {
        motor_colour.body.main_color = LV_COLOR_RED;
        motor_colour.body.grad_color = LV_COLOR_RED;
    }
    else if (percentage >= 80)
    {
        motor_colour.body.main_color = LV_COLOR_ORANGE;
        motor_colour.body.grad_color = LV_COLOR_ORANGE;
    }
    else if (percentage >= 60)
    {
        motor_colour.body.main_color = LV_COLOR_YELLOW;
        motor_colour.body.grad_color = LV_COLOR_YELLOW;
    }
    else if (percentage >= 20)
    {
        motor_colour.body.main_color = LV_COLOR_GREEN;
        motor_colour.body.grad_color = LV_COLOR_GREEN;
    }
    else if (percentage <= 20)
    {
        motor_colour.body.main_color = LV_COLOR_BLUE;
        motor_colour.body.grad_color = LV_COLOR_BLUE;
    }
}

static void rine_bar_colour(lv_task_t * t)
{
    float percentage = lv_bar_get_value(rineheart_bar);
    percentage = percentage/80;
    percentage = percentage*100;
    printf("Percentage = %f",percentage);

    if (percentage >= 90)
    {
        rine_colour.body.main_color = LV_COLOR_RED;
        rine_colour.body.grad_color = LV_COLOR_RED;
    }
    else if (percentage >= 80)
    {
        rine_colour.body.main_color = LV_COLOR_ORANGE;
        rine_colour.body.grad_color = LV_COLOR_ORANGE;
    }
    else if (percentage >= 60)
    {
        rine_colour.body.main_color = LV_COLOR_YELLOW;
        rine_colour.body.grad_color = LV_COLOR_YELLOW;
    }
    else if (percentage >= 20)
    {
        rine_colour.body.main_color = LV_COLOR_GREEN;
        rine_colour.body.grad_color = LV_COLOR_GREEN;
    }
    else if (percentage <= 20)
    {
        rine_colour.body.main_color = LV_COLOR_BLUE;
        rine_colour.body.grad_color = LV_COLOR_BLUE;
    }
}

static void accum_t_bar_colour(lv_task_t * t)
{
    float percentage = lv_bar_get_value(accum_temp);
    percentage = percentage/80;
    percentage = percentage*100;
    printf("Percentage = %f",percentage);

    if (percentage >= 90)
    {
        accum_t_colour.body.main_color = LV_COLOR_RED;
        accum_t_colour.body.grad_color = LV_COLOR_RED;
    }
    else if (percentage >= 80)
    {
        accum_t_colour.body.main_color = LV_COLOR_ORANGE;
        accum_t_colour.body.grad_color = LV_COLOR_ORANGE;
    }
    else if (percentage >= 60)
    {
        accum_t_colour.body.main_color = LV_COLOR_YELLOW;
        accum_t_colour.body.grad_color = LV_COLOR_YELLOW;
    }
    else if (percentage >= 20)
    {
        accum_t_colour.body.main_color = LV_COLOR_GREEN;
        accum_t_colour.body.grad_color = LV_COLOR_GREEN;
    }
    else if (percentage <= 20)
    {
        accum_t_colour.body.main_color = LV_COLOR_BLUE;
        accum_t_colour.body.grad_color = LV_COLOR_BLUE;
    }
}

static void accum_v_bar_colour(lv_task_t * t)
{
    float percentage = lv_bar_get_value(accum_volt);
    percentage = percentage/600;
    percentage = percentage*100;
    if (percentage >= 40)
    {
        accum_v_colour.body.main_color = LV_COLOR_GREEN;
        accum_v_colour.body.grad_color = LV_COLOR_GREEN;
    }
    else if (percentage >= 20)
    {
        accum_v_colour.body.main_color = LV_COLOR_YELLOW;
        accum_v_colour.body.grad_color = LV_COLOR_YELLOW;
    }
    else if (percentage >= 10)
    {
        accum_v_colour.body.main_color = LV_COLOR_ORANGE;
        accum_v_colour.body.grad_color = LV_COLOR_ORANGE;
    }
    else if (percentage <= 10)
    {
        accum_v_colour.body.main_color = LV_COLOR_RED;
        accum_v_colour.body.grad_color = LV_COLOR_RED;
    }
}