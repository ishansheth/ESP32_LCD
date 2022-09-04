// libc includes
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
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_sntp.h"
#include "esp_log.h"

#include "driver/gpio.h"
#include "nvs_flash.h"

// LVGL related include
#include "lvgl.h"
#include "lvgl_helpers.h"

#include "lcd_gui.h"

#define LCD_BACKLIGHT 23
#define PUSH_BUTTON 15

#define LV_TICK_PERIOD_MS 1

#define MAX_FAILURES 		10

static const char wifi_tag[] = "[WIFI Connect]";

SemaphoreHandle_t clock_semaphore;
SemaphoreHandle_t tab_semaphore;

static EventGroupHandle_t wifi_event_group;
static int s_retry_num = 0;
lv_obj_t* sys_stat_tab;


static void create_demo_application(void)
{
	ESP_LOGI(TAG,"creating demo application");

	static lv_style_t style_btn;
	lv_style_init(&style_btn);
	lv_style_set_bg_color(&style_btn,LV_STATE_DEFAULT, LV_COLOR_BLACK);
	lv_style_set_bg_opa(&style_btn,LV_STATE_DEFAULT, LV_OPA_50);
	lv_style_set_border_width(&style_btn,LV_STATE_DEFAULT, 2);

	lv_obj_t* tv = lv_tabview_create(lv_scr_act(), NULL);
	lv_obj_add_style(tv,LV_OBJ_PART_MAIN, &style_btn);

    	lv_obj_t* t1 = lv_tabview_add_tab(tv, "Clock");
    	lv_obj_t* t2 = lv_tabview_add_tab(tv, "StopWatch");
    	sys_stat_tab = lv_tabview_add_tab(tv, "Sys Stat");
	xSemaphoreGive(tab_semaphore);

	create_clock(t1);
	create_stopwatch(t2);

	while (1) 
	{
        	vTaskDelay(pdMS_TO_TICKS(10));
       		lv_task_handler();
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
	
	// create event group
	wifi_event_group = xEventGroupCreate();
	
	// set connect to wifi event handler	
	esp_event_handler_instance_t wifi_handler_event_instance;
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,ESP_EVENT_ANY_ID,&wifi_event_handler,NULL,&wifi_handler_event_instance));

	// set obtained IP event handler
	esp_event_handler_instance_t got_ip_event_instance;
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,IP_EVENT_STA_GOT_IP,&ip_event_handler,NULL,&got_ip_event_instance));

	
	wifi_config_t wifi_config = {
		.sta = {
			.ssid = "Vodafone-8F54",
			.password="bb4LMgGLAFAmLzcJ",
			.threshold.authmode = WIFI_AUTH_WPA2_PSK,
			.pmf_cfg = {
				.capable=true,
				.required=false
			},
			},
		};

	// set wifi mode to wifi station	
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	// set the configuration
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA,&wifi_config));
	// start the wifi
	ESP_ERROR_CHECK(esp_wifi_start());
	ESP_LOGI(wifi_tag,"STA initialization complete");
	
	// wait until either WIFI_SUCCESS or WIFI_FAILURE bits are set in the wifi_event_group
	EventBits_t bits = xEventGroupWaitBits(wifi_event_group,WIFI_SUCCESS|WIFI_FAILURE,pdFALSE,pdFALSE,portMAX_DELAY);
	
	// check the set value inside bits
	if (bits & WIFI_SUCCESS)
		status = WIFI_SUCCESS;
	else if(bits & WIFI_FAILURE)
		status = WIFI_FAILURE;	
	else
	{
		ESP_LOGE(TAG,"Unexpected event");
		status = WIFI_FAILURE;
	}

	// deregister handlers and delete event group
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
		wifi_connect_status = WIFI_SUCCESS;
    	}
	return 1;
}


void prvStatTask(void* para)
{
	xSemaphoreTake( tab_semaphore, portMAX_DELAY );
	display_system_runtime_stat(sys_stat_tab);

}


void app_main(void)
{
	clock_semaphore = xSemaphoreCreateMutex();
	tab_semaphore = xSemaphoreCreateBinary();


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
	if(push_button_check_handle == pdPASS)
		ESP_LOGI(TAG,"Push button check task created");

	BaseType_t display_stat_handle = xTaskCreatePinnedToCore(prvStatTask, "DisplayRunTimeStat", 1024*2,  xTaskGetCurrentTaskHandle(), 10,NULL,0);
	if(display_stat_handle == pdPASS)
		ESP_LOGI(TAG,"Display Runtime Stat task created");

	// run LVGL GUI on core 1 of ESP32	
	BaseType_t gui_task_handle = xTaskCreatePinnedToCore(create_demo_application, "GUITask", 1024*4,  xTaskGetCurrentTaskHandle(), 10,NULL,1);
	if(gui_task_handle == pdPASS)
		ESP_LOGI(TAG,"GUI task created");

	ESP_ERROR_CHECK( nvs_flash_init() );

	if (pdTRUE == xSemaphoreTake(clock_semaphore, portMAX_DELAY)) 
	{
		obtain_time();
		xSemaphoreGive(clock_semaphore);
	}



    	/* A task should NEVER return */
    	free(buf1);
    	free(buf2);
    	vTaskDelete(NULL);

}


