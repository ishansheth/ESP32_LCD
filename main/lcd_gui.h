#ifndef LCD_GUI_H__
#define LCD_GUI_H__

#include "lvgl.h"
#include <time.h>
#include <errno.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

#include "freertos/semphr.h"
#include "touch_driver.h"

#define WIFI_SUCCESS 		1<<0
#define WIFI_FAILURE 		1<<1
#define WIFI_STATUS_NOT_SET 	1<<2
#define TAG "demo"

// in C, static keyword is required because variables in header file have external linkage
// which will create multiple definition linker error. Same applies to the function
// If the function is static, it can not be called from other source file because it will 
// have internal linkage
// https://stackoverflow.com/questions/2328671/constant-variables-not-working-in-header

static char strftime_buf[32];
static char strfdate_buf[32];

static char date_format_string[16] = "%d/%m/%Y %a";
static char time_format_string[16] = "      %X";

// creating variable with external linkage because its used/set in multiple translation unit
extern int wifi_connect_status;

void timelabel_text_anim(lv_task_t * t);
void datelabel_text_anim(lv_task_t * t);
void time_format_dropdown_event_handler(lv_obj_t * obj, lv_event_t event);
void date_format_dropdown_event_handler(lv_obj_t * obj, lv_event_t event);
void set_datetime_done_button_handler(lv_obj_t * obj, lv_event_t event);
void set_date_calendar_event_handler(lv_obj_t * obj, lv_event_t event);
void create_set_datetime_window(lv_obj_t * parent);
void set_datetime_button_handler(lv_obj_t * obj, lv_event_t event);
void create_clock(lv_obj_t* parent);
void create_stopwatch(lv_obj_t* parent);
void create_stopwatch(lv_obj_t* parent);
void display_system_runtime_stat(lv_obj_t* parent);


#endif






