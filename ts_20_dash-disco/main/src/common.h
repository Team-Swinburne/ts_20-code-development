/**
 * @file screen3.h
 *
 */


#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#ifdef LV_CONF_INCLUDE_SIMPLE
#include "lvgl.h"
#include "lv_ex_conf.h"
#else
#include "../../../lvgl/lvgl.h"
#include "../../../lv_ex_conf.h"
#endif



/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void warning_lines();
void draw_precharge_warning();
void draw_drive_warning();
void draw_disagree_warning();
void draw_trailbrake_warning();
void gauge_handler(lv_task_t * task);
void can_iterator(lv_task_t * task);
void can_info_handler(lv_task_t * task);

/**********************
 *      MACROS
 **********************/
