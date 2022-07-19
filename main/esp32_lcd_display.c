#include <time.h>
#include <errno.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// freee RTOS related includes
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

// esp-idf includes
#include "esp_event.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "esp_log.h"

#include "driver/gpio.h"
#include "nvs_flash.h"

// LVGL related include
#include "lvgl.h"
#include "lvgl_helpers.h"


#define LCD_BACKLIGHT 23
#define PUSH_BUTTON 15

#define LV_TICK_PERIOD_MS 1
#define TAG "demo"

#define WIFI_SUCCESS 		1<<0
#define WIFI_FAILURE 		1<<1
#define WIFI_STATUS_NOT_SET 	1<<2

#define MAX_FAILURES 		10

static const char wifi_tag[] = "[WIFI Connect]";

SemaphoreHandle_t xGuiSemaphore;
SemaphoreHandle_t clock_semaphone;


static EventGroupHandle_t wifi_event_group;
static int s_retry_num = 0;
char strftime_buf[64];
char strfdate_buf[64];

char date_format_string[64] = "%d/%m/%Y %a";
char time_format_string[64] = "      %X";

static lv_obj_t * tv;
static lv_obj_t * t1;
static lv_obj_t * t2;
static lv_obj_t * t3;
static lv_style_t style_box;

static int wifi_connect_status = WIFI_STATUS_NOT_SET;

static void timelabel_text_anim(lv_task_t * t)
{
	// get the current time since epoch
	time_t now = time(NULL);
    	struct tm timeinfo;
	// convert now to calendar style time
    	localtime_r(&now, &timeinfo);
	// foramts the time in timeinfo and stores it in strftime_buf
    	lv_obj_t * label = t->user_data;

    	strftime(strftime_buf, sizeof(strftime_buf), time_format_string, &timeinfo);

	// depending on the status, set the corresponding string
	if(wifi_connect_status == WIFI_SUCCESS)
		lv_label_set_text(label,strftime_buf);
	else if(wifi_connect_status == WIFI_FAILURE)
	{
		snprintf(strftime_buf,sizeof(strftime_buf),"Couldn't initialize time!!");
		lv_label_set_text(label,strftime_buf);
	}

}

static void datelabel_text_anim(lv_task_t * t)
{
	// get the current time since epoch
	time_t now = time(NULL);
    	struct tm timeinfo;
	// convert now to calendar style time
    	localtime_r(&now, &timeinfo);
	// foramts the time in timeinfo and stores it in strftime_buf
    	lv_obj_t * label = t->user_data;

    	strftime(strfdate_buf, sizeof(strfdate_buf), date_format_string, &timeinfo);

	// depending on the status, set the corresponding string
	if(wifi_connect_status == WIFI_SUCCESS)
		lv_label_set_text(label,strfdate_buf);
	else if(wifi_connect_status == WIFI_FAILURE)
	{
		snprintf(strftime_buf,sizeof(strftime_buf),"!!!!!!!!!!");
		lv_label_set_text(label,strftime_buf);
	}

}

static void time_format_dropdown_event_handler(lv_obj_t * obj, lv_event_t event)
{
	if(event == LV_EVENT_VALUE_CHANGED) 
	{
		char buf[32];
        	lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
		if(!strcmp(buf,"hh:mm:ss 24h"))
		{
			strncpy(time_format_string,"      %T",sizeof(time_format_string));
		}
		else if(!strcmp(buf,"hh:mm 24h"))
		{
			strncpy(time_format_string,"      %R",sizeof(time_format_string));
		}
		else if(!strcmp(buf,"hh:mm:ss 12h"))
		{
			strncpy(time_format_string,"      %r",sizeof(time_format_string));
		}
		else if(!strcmp(buf,"hh:mm 12h"))
		{
			strncpy(time_format_string,"      %I:%M %p",sizeof(time_format_string));
		}
	}
}

static void date_format_dropdown_event_handler(lv_obj_t * obj, lv_event_t event)
{
	if(event == LV_EVENT_VALUE_CHANGED) 
	{
		char buf[32];
        	lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
		if(!strcmp(buf,"dd/mm/yyyy"))
		{
			strncpy(date_format_string,"%d/%m/%Y %a",sizeof(date_format_string));
		}
		else if(!strcmp(buf,"dd/mm/yy"))
		{
			strncpy(date_format_string,"%d/%m/%y %a",sizeof(date_format_string));
		}
		else if(!strcmp(buf,"yyyy/mm/dd"))
		{
			strncpy(date_format_string,"%Y/%m/%d %a",sizeof(date_format_string));
		}
		else if(!strcmp(buf,"yy/mm/dd"))
		{
			strncpy(date_format_string,"%y/%m/%d %a",sizeof(date_format_string));
		}
	}
}


static void set_datetime_done_button_handler(lv_obj_t * obj, lv_event_t event)
{
    	if(event == LV_EVENT_CLICKED) {
		lv_obj_t* p = lv_obj_get_parent(obj);
		lv_obj_del(p);
	}
}

static void set_date_calendar_event_handler(lv_obj_t * obj, lv_event_t event)
{
    	if(event == LV_EVENT_VALUE_CHANGED) 
	{
		lv_calendar_date_t * date = lv_calendar_get_pressed_date(obj);
        	if(date) 
		{
            		printf("Clicked date: %02d.%02d.%d\n", date->day, date->month, date->year);
        	}
	}
}

static void create_set_datetime_window(lv_obj_t * parent)
{
	lv_obj_t* set_container = lv_cont_create(parent,NULL);
	lv_obj_set_size(set_container,250,220);
	lv_obj_align(set_container,NULL,LV_ALIGN_CENTER,0,0);

	lv_obj_t  * calendar = lv_calendar_create(set_container, NULL);
    	lv_obj_set_size(calendar, 230, 150);
    	lv_obj_align(calendar, NULL, LV_ALIGN_CENTER, 0, -25);
    	lv_obj_set_event_cb(calendar, set_date_calendar_event_handler);

	lv_obj_t* btn1 = lv_btn_create(set_container, NULL);
    	lv_obj_set_event_cb(btn1, set_datetime_done_button_handler);
    	lv_obj_align(btn1, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

    	lv_obj_t* label = lv_label_create(btn1, NULL);
    	lv_label_set_text(label, "Clear");

}


static void set_datetime_button_handler(lv_obj_t * obj, lv_event_t event)
{
    	if(event == LV_EVENT_CLICKED) 
	{
		create_set_datetime_window(lv_obj_get_parent(obj));	
    	}
}


void create_clock(lv_obj_t* parent)
{

	lv_disp_size_t disp_size = lv_disp_get_size_category(NULL);
	lv_coord_t grid_w = lv_page_get_width_grid(parent, disp_size <= LV_DISP_SIZE_SMALL ? 1 : 2, 1);
	lv_coord_t grid_h = lv_page_get_height_grid(parent, disp_size <= LV_DISP_SIZE_SMALL ? 1 : 2, 1);

	ESP_LOGI(TAG,"width:%d",grid_w);
	ESP_LOGI(TAG,"height:%d",grid_h);

	// create container to display time and date
	lv_obj_t* cont = lv_cont_create(parent, NULL);
	// set thee alignment of the container
	lv_obj_align(cont,NULL,LV_ALIGN_IN_TOP_MID,-110,10);
	// set the size of container
	lv_obj_set_size(cont,140,45);

	// create a label to display the time
	lv_obj_t* timelabel = lv_label_create(cont, NULL);
	// align the time display label
	lv_obj_align(timelabel,NULL,LV_ALIGN_IN_TOP_MID,-40,6);
	// create a task to update the time in the time display label
	lv_task_create(timelabel_text_anim, 1000, LV_TASK_PRIO_LOW, timelabel);
	// set the text of the time display label	
	snprintf ( strftime_buf, sizeof(strftime_buf), "Initializing..." );
	lv_label_set_text(timelabel,strftime_buf);

	// create a label to display the date
	lv_obj_t* datelabel = lv_label_create(cont, NULL);
	// align the date display label
	lv_obj_align(datelabel,NULL,LV_ALIGN_IN_TOP_MID,-40,23);
	// set the text of the time display label	
	// create a task to update the time in the time display label
	lv_task_create(datelabel_text_anim, 1000, LV_TASK_PRIO_LOW, datelabel);
	snprintf ( strfdate_buf, sizeof(strfdate_buf), "..........." );
	lv_label_set_text(datelabel,strfdate_buf);

	// create set date/time button and its label
    	lv_obj_t* btn1 = lv_btn_create(parent, NULL);
    	lv_obj_set_event_cb(btn1, set_datetime_button_handler);
    	lv_obj_align(btn1, NULL, LV_ALIGN_IN_TOP_MID, -80, 80);

    	lv_obj_t* label = lv_label_create(btn1, NULL);
    	lv_label_set_text(label, "Set Datetime");

	
	// create a label for the date format drop down
	lv_obj_t* date_format_list_text = lv_label_create(parent, NULL);	
	// align the label
    	lv_obj_align(date_format_list_text, NULL, LV_ALIGN_IN_TOP_MID, 35, 10);
	lv_label_set_recolor(date_format_list_text,true);
	// set text of color
	lv_label_set_text(date_format_list_text,"#ff0000 Date format:#");
	
	// create a list of dateformat
	lv_obj_t * date_format_list = lv_dropdown_create(parent, NULL);
    	lv_dropdown_set_options(date_format_list, "dd/mm/yyyy\n"
					"dd/mm/yy\n"
				        "yyyy/mm/dd\n"
				        "yy/mm/dd");
	// set size of a list
	lv_obj_set_size(date_format_list,150,40);
	// align the list
    	lv_obj_align(date_format_list, date_format_list_text, LV_ALIGN_OUT_BOTTOM_MID, 0, 6);
    	lv_obj_set_event_cb(date_format_list, date_format_dropdown_event_handler);


	// create a label for the time format drop down
	lv_obj_t* time_format_list_text = lv_label_create(parent, NULL);	
	// align the label
    	lv_obj_align(time_format_list_text, NULL, LV_ALIGN_IN_TOP_MID, 35, 75);
	lv_label_set_recolor(time_format_list_text,true);
	// set text of color
	lv_label_set_text(time_format_list_text,"#ff0000 Time format:#");


	// create a list of timeformat
	lv_obj_t * time_format_list = lv_dropdown_create(parent, NULL);
    	lv_dropdown_set_options(time_format_list, "hh:mm:ss 24h\n"
					"hh:mm 24h\n"
				        "hh:mm:ss 12h\n"
				        "hh:mm 12h");
	// set size of a list
	lv_obj_set_size(time_format_list,150,40);
	// align the list
    	lv_obj_align(time_format_list, time_format_list_text, LV_ALIGN_OUT_BOTTOM_MID, 0, 6);
    	lv_obj_set_event_cb(time_format_list, time_format_dropdown_event_handler);

}


void create_stopwatch(lv_obj_t* parent)
{

}

static void create_demo_application(void)
{
	ESP_LOGI(TAG,"creating demo application");

	static lv_style_t style_btn;
	lv_style_init(&style_btn);
	lv_style_set_bg_color(&style_btn,LV_STATE_DEFAULT, LV_COLOR_BLACK);
	lv_style_set_bg_opa(&style_btn,LV_STATE_DEFAULT, LV_OPA_50);
	lv_style_set_border_width(&style_btn,LV_STATE_DEFAULT, 2);


	tv = lv_tabview_create(lv_scr_act(), NULL);
	lv_obj_add_style(tv,LV_OBJ_PART_MAIN, &style_btn);

    	t1 = lv_tabview_add_tab(tv, "Clock");
    	t2 = lv_tabview_add_tab(tv, "StopWatch");
    	t3 = lv_tabview_add_tab(tv, "Timer");

	create_clock(t1);
	create_stopwatch(t2);


	while (1) {
        	/* Delay 1 tick (assumes FreeRTOS tick is 10ms */
        	vTaskDelay(pdMS_TO_TICKS(10));

        	/* Try to take the semaphore, call lvgl related function on success */


        	if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
            		lv_task_handler();
            		xSemaphoreGive(xGuiSemaphore);
       		}

    	}

}


void time_sync_notification_cb(struct timeval *tv)
{
   	ESP_LOGI(wifi_tag, "Notification of a time synchronization event");
}

static void initialize_sntp(void)
{
	ESP_LOGI(wifi_tag, "Initializing SNTP");
	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	sntp_setservername(0, "pool.ntp.org");
	sntp_set_time_sync_notification_cb(time_sync_notification_cb);
	sntp_init();

}

void button_push_handler(void* arg)
{
	while(1)
	{
		int value = gpio_get_level(PUSH_BUTTON);
		if (value == 1)
			gpio_set_level(LCD_BACKLIGHT,1);
		else if(value == 0)
	  		gpio_set_level(LCD_BACKLIGHT,0);

		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}





static void lv_tick_task(void *arg) 
{
    (void) arg;
    lv_tick_inc(LV_TICK_PERIOD_MS);
}



static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
	{
		ESP_LOGI(wifi_tag,"Connecting to AP....");
		esp_wifi_connect();
	}
	else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
	{
		if(s_retry_num < MAX_FAILURES)
		{
			ESP_LOGI(wifi_tag,"Reconnecting to AP....");
			esp_wifi_connect();
			s_retry_num++;

		}	
		else
		{
			ESP_LOGI(wifi_tag,"Could not connect to WIFI!!");
			xEventGroupSetBits(wifi_event_group,WIFI_FAILURE);
		}
	}
}

static void ip_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
	{
		ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
		ESP_LOGI(wifi_tag,"STA IP: " IPSTR,IP2STR(&event->ip_info.ip));
		s_retry_num=0;
		xEventGroupSetBits(wifi_event_group,WIFI_SUCCESS);
		
	}
}


static int initialise_wifi(void)
{

	int status = WIFI_FAILURE;
	ESP_ERROR_CHECK(esp_netif_init());
	
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	esp_netif_create_default_wifi_sta();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	wifi_event_group = xEventGroupCreate();
	
	esp_event_handler_instance_t wifi_handler_event_instance;
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,ESP_EVENT_ANY_ID,&wifi_event_handler,NULL,&wifi_handler_event_instance));

	esp_event_handler_instance_t got_ip_event_instance;
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,IP_EVENT_STA_GOT_IP,&ip_event_handler,NULL,&got_ip_event_instance));

	wifi_config_t wifi_config = {
		.sta = {
			.ssid = "FRITZ!Box 7520 HD",
			.password="04879856581287203287",
			.threshold.authmode = WIFI_AUTH_WPA2_PSK,
			.pmf_cfg = {
				.capable=true,
				.required=false
			},
			},
		};

	
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA,&wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());
	ESP_LOGI(wifi_tag,"STA initialization complete");
	
	EventBits_t bits = xEventGroupWaitBits(wifi_event_group,WIFI_SUCCESS|WIFI_FAILURE,pdFALSE,pdFALSE,portMAX_DELAY);
	
	if (bits & WIFI_SUCCESS)
		status = WIFI_SUCCESS;
	else if(bits & WIFI_FAILURE)
		status = WIFI_FAILURE;	
	else
	{
		ESP_LOGE(TAG,"Unexpected event");
		status = WIFI_FAILURE;
	}

	ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT,IP_EVENT_STA_GOT_IP,got_ip_event_instance));
	ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT,ESP_EVENT_ANY_ID,wifi_handler_event_instance));
	vEventGroupDelete(wifi_event_group);

	return status;
}


static int obtain_time(void)
{	
	if (initialise_wifi() == WIFI_SUCCESS)
		initialize_sntp();
	
	time_t now;
    	struct tm timeinfo;
    	time(&now);
    	localtime_r(&now, &timeinfo);

	int retry = 0;
    	const int retry_count = 20;

	while(timeinfo.tm_year < (2022 - 1900) && ++retry < retry_count) {
		vTaskDelay(500 / portTICK_PERIOD_MS);
	        time(&now);
	    	localtime_r(&now, &timeinfo);
	}

    	if (timeinfo.tm_year < (2022 - 1900)) {
    		ESP_LOGI(wifi_tag, "System time NOT set.");
		wifi_connect_status = WIFI_FAILURE;
    	}
    	else 
	{
    		ESP_LOGI(wifi_tag, "System time is set.");

		setenv("TZ", "CEST-2", 1);
    		tzset();
    		localtime_r(&now, &timeinfo);
    		strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    		ESP_LOGI(TAG, "The current date/time in DE is: %s", strftime_buf);
		wifi_connect_status = WIFI_SUCCESS;

    	}
	return 1;
}

void app_main(void)
{
	xGuiSemaphore = xSemaphoreCreateMutex();
	clock_semaphone = xSemaphoreCreateMutex();

	lv_init();

    	lvgl_driver_init();

    	lv_color_t* buf1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    	assert(buf1 != NULL);

    	lv_color_t* buf2 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    	assert(buf2 != NULL);

    	static lv_disp_buf_t disp_buf;

    	uint32_t size_in_px = DISP_BUF_SIZE;


    	/* Initialize the working buffer depending on the selected display.
     	* NOTE: buf2 == NULL when using monochrome displays. */
    	lv_disp_buf_init(&disp_buf, buf1, buf2, size_in_px);

    	lv_disp_drv_t disp_drv;
    	lv_disp_drv_init(&disp_drv);
    	disp_drv.flush_cb = disp_driver_flush;

    	disp_drv.buffer = &disp_buf;
    	lv_disp_drv_register(&disp_drv);

    	lv_indev_drv_t indev_drv;
    	lv_indev_drv_init(&indev_drv);
    	indev_drv.read_cb = touch_driver_read;
    	indev_drv.type = LV_INDEV_TYPE_POINTER;
    	lv_indev_drv_register(&indev_drv);

    	/* Create and start a periodic timer interrupt to call lv_tick_inc */
    	const esp_timer_create_args_t periodic_timer_args = {
	        .callback = &lv_tick_task,
	        .name = "periodic_gui"
    	};

	esp_timer_handle_t periodic_timer;
	ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
	ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));


	gpio_reset_pin(LCD_BACKLIGHT);
	gpio_set_direction(LCD_BACKLIGHT,GPIO_MODE_OUTPUT);	

	BaseType_t push_button_check_handle = xTaskCreatePinnedToCore(button_push_handler, "CheckButtonStatus", 1024,  xTaskGetCurrentTaskHandle(), 10,NULL,0);

	// run LVGL GUI on core 1 of ESP32
	
	BaseType_t gui_task_handle = xTaskCreatePinnedToCore(create_demo_application, "GUITask", 1024*4,  xTaskGetCurrentTaskHandle(), 10,NULL,1);

	ESP_ERROR_CHECK( nvs_flash_init() );

	if (pdTRUE == xSemaphoreTake(clock_semaphone, portMAX_DELAY)) 
	{
		obtain_time();
		xSemaphoreGive(clock_semaphone);
	}



    	/* A task should NEVER return */
    	free(buf1);
    	free(buf2);
    	vTaskDelete(NULL);

}


