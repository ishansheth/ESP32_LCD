#include "lcd_gui.h"

int wifi_connect_status = WIFI_STATUS_NOT_SET;
char day_value_buf[16];
char month_value_buf[16];
char year_value_buf[16];

char stopwatch_buf[16];
static uint8_t stopwatch_hr = 0;
static uint8_t stopwatch_sec = -1;
static uint8_t stopwatch_min = 0;

lv_task_t * stopwatch_countdown_task;
lv_obj_t * stopwatch_label; 
lv_obj_t * stopwatch_lap_list; 
lv_obj_t * drawingpad_canvas; 

// static buffer for the drawingpad_canvas

bool start_stop_watch = false;
SemaphoreHandle_t stopwatch_semaphore;


void timelabel_text_anim(lv_task_t * t)
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

void datelabel_text_anim(lv_task_t * t)
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

void time_format_dropdown_event_handler(lv_obj_t * obj, lv_event_t event)
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

void date_format_dropdown_event_handler(lv_obj_t * obj, lv_event_t event)
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


void set_datetime_done_button_handler(lv_obj_t * obj, lv_event_t event)
{
    	if(event == LV_EVENT_CLICKED) {
		lv_obj_t* p = lv_obj_get_parent(obj);
		lv_obj_del(p);
        	printf("Selected day: %s\n", day_value_buf);
        	printf("Selected month: %s\n", month_value_buf);
        	printf("Selected year: %s\n", year_value_buf);
	}
}

void day_selector_event_handler(lv_obj_t * obj, lv_event_t event)
{
 	if(event == LV_EVENT_VALUE_CHANGED) 
	{
        	lv_roller_get_selected_str(obj, day_value_buf, sizeof(day_value_buf));
    	}	
}

void month_selector_event_handler(lv_obj_t * obj, lv_event_t event)
{
 	if(event == LV_EVENT_VALUE_CHANGED) 
	{
        	lv_roller_get_selected_str(obj, month_value_buf, sizeof(month_value_buf));
    	}	
}

void year_selector_event_handler(lv_obj_t * obj, lv_event_t event)
{
 	if(event == LV_EVENT_VALUE_CHANGED) 
	{
        	lv_roller_get_selected_str(obj, year_value_buf, sizeof(year_value_buf));
    	}	
}

void create_set_datetime_window(lv_obj_t * parent)
{

    	lv_obj_t * datetime_tileview = lv_cont_create(parent, NULL);
    	//lv_tileview_set_valid_positions(datetime_tileview, datetime_setting_valid_pos, 3);
	lv_obj_set_size(datetime_tileview,250,220);
	lv_obj_align(datetime_tileview,NULL,LV_ALIGN_CENTER,0,0);


	lv_obj_t *day_selector = lv_roller_create(datetime_tileview, NULL);
    	lv_roller_set_options(day_selector,"1\n"
                        "2\n"
                        "3\n"
                        "4\n"
                        "5\n"
                        "6\n"
                        "7\n"
                        "8\n"
                        "9\n"
                        "10\n"
                        "11\n"
                        "12\n"
                        "13\n"
                        "14\n"
                        "15\n"
                        "16\n"
                        "17\n"
                        "18\n"
                        "19\n"
                        "20\n"
                        "21\n"
                        "22\n"
                        "23\n"
                        "24\n"
                        "25\n"
                        "26\n"
                        "27\n"
                        "28\n"
                        "29\n"
                        "30\n"
                        "31\n",
                        LV_ROLLER_MODE_INFINITE);

    	lv_roller_set_visible_row_count(day_selector, 4);
    	lv_obj_align(day_selector, NULL, LV_ALIGN_IN_TOP_MID, -80, 10);
    	lv_obj_set_event_cb(day_selector, day_selector_event_handler);
	lv_obj_set_size(day_selector,50,130);


	lv_obj_t *month_selector = lv_roller_create(datetime_tileview, NULL);
    	lv_roller_set_options(month_selector,"Jan\n"
                        "Feb\n"
                        "Mar\n"
                        "Apr\n"
                        "May\n"
                        "Jun\n"
                        "Jul\n"
                        "Aug\n"
                        "Sep\n"
                        "Oct\n"
                        "Nov\n"
                        "Dec\n",
                        LV_ROLLER_MODE_INFINITE);

    	lv_roller_set_visible_row_count(month_selector, 4);
    	lv_obj_align(month_selector, NULL, LV_ALIGN_IN_TOP_MID, -10, 10);
    	lv_obj_set_event_cb(month_selector, month_selector_event_handler);
	lv_obj_set_size(month_selector,50,130);


	lv_obj_t *year_selector = lv_roller_create(datetime_tileview, NULL);
    	lv_roller_set_options(year_selector,"2000\n"
			"2001\n"
			"2002\n"
			"2003\n"
			"2004\n"
			"2005\n"
			"2006\n"
			"2007\n"
			"2008\n"
			"2009\n"
			"2010\n"
                        "2011\n"
                        "2012\n"
                        "2013\n"
                        "2014\n"
                        "2015\n"
                        "2016\n"
                        "2017\n"
                        "2018\n"
                        "2019\n"
                        "2020\n"
                        "2021\n"
                        "2022\n",
                        LV_ROLLER_MODE_INFINITE);

    	lv_roller_set_visible_row_count(year_selector, 4);
    	lv_obj_align(year_selector, NULL, LV_ALIGN_IN_TOP_MID, 70, 10);
    	lv_obj_set_event_cb(year_selector, year_selector_event_handler);
	lv_obj_set_size(year_selector,60,130);


	lv_obj_t* done_btn = lv_btn_create(datetime_tileview, NULL);
    	lv_obj_set_event_cb(done_btn, set_datetime_done_button_handler);
    	lv_obj_align(done_btn, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

    	lv_obj_t* done_btn_label = lv_label_create(done_btn, NULL);
    	lv_label_set_text(done_btn_label, "Done");

}


void set_datetime_button_handler(lv_obj_t * obj, lv_event_t event)
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
	/* TODO: setting date and time needs to be researched. By user its not recommended
    	lv_obj_t* btn1 = lv_btn_create(parent, NULL);
    	lv_obj_set_event_cb(btn1, set_datetime_button_handler);
    	lv_obj_align(btn1, NULL, LV_ALIGN_IN_TOP_MID, -80, 80);

    	lv_obj_t* label = lv_label_create(btn1, NULL);
    	lv_label_set_text(label, "Set Datetime");
	*/
	
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

void stopwatch_label_text_anim(lv_task_t * task)
{

	if (pdTRUE == xSemaphoreTake(stopwatch_semaphore, portMAX_DELAY)) 
	{
		if(start_stop_watch)
		{

			stopwatch_sec++;
			if(stopwatch_sec == 59)
			{
				stopwatch_min++;
				stopwatch_sec = 0;
			}
			if(stopwatch_min == 59)
			{
				stopwatch_hr++;
				stopwatch_min = 0;
			}
		}
		xSemaphoreGive(stopwatch_semaphore);

	}

	if(stopwatch_sec >= 0)
	{
	    	lv_obj_t * label = task->user_data;
		snprintf(stopwatch_buf,sizeof(stopwatch_buf),"%d:%d:%d",stopwatch_hr,stopwatch_min,stopwatch_sec);
		lv_label_set_text(label,stopwatch_buf);
	}
}

void start_stopwatch_button_handler(lv_obj_t * obj, lv_event_t event)
{
	if (pdTRUE == xSemaphoreTake(stopwatch_semaphore, portMAX_DELAY)) 
	{
		if(event == LV_EVENT_CLICKED) 
		{
			start_stop_watch = true;
		}
		xSemaphoreGive(stopwatch_semaphore);
	}
	uint16_t num_elements =	lv_list_get_size(stopwatch_lap_list);
	for(uint8_t i = 0; i < num_elements; i++)
		lv_list_remove(stopwatch_lap_list,i);

	lv_obj_set_hidden(stopwatch_lap_list, true);

}


void stop_stopwatch_button_handler(lv_obj_t * obj, lv_event_t event)
{
	if (pdTRUE == xSemaphoreTake(stopwatch_semaphore, portMAX_DELAY)) 
	{

		if(event == LV_EVENT_CLICKED) 
		{
			start_stop_watch = false;
			stopwatch_min = 0;
			stopwatch_sec = 0;
			stopwatch_hr = 0;
		}
		xSemaphoreGive(stopwatch_semaphore);
	}

}


void lap_stopwatch_button_handler(lv_obj_t * obj, lv_event_t event)
{

	if (pdTRUE == xSemaphoreTake(stopwatch_semaphore, portMAX_DELAY))
	{
		if(event == LV_EVENT_CLICKED) 
		{
			lv_obj_set_hidden(stopwatch_lap_list, false);
			lv_obj_t * list_btn = lv_list_add_btn(stopwatch_lap_list, NULL, stopwatch_buf);
		}
		xSemaphoreGive(stopwatch_semaphore);
	}
}


void create_stopwatch(lv_obj_t* parent)
{
	stopwatch_semaphore = xSemaphoreCreateMutex();

	// create container to display time and date
	lv_obj_t* cont = lv_cont_create(parent, NULL);
	// set thee alignment of the container
	lv_obj_align(cont,NULL,LV_ALIGN_CENTER,-100,-50);
	// set the size of container
	lv_obj_set_size(cont,140,45);

	lv_obj_t* stopwatch_label = lv_label_create(cont, NULL);
	// align the time display label
	lv_obj_align(stopwatch_label,NULL,LV_ALIGN_CENTER,-10,0);
	// create a task to update the time in the time display label
	lv_task_t * stopwatch_countdown_task = lv_task_create(stopwatch_label_text_anim, 900, LV_TASK_PRIO_LOW, stopwatch_label);

	// label for min,sec,millisec
	lv_label_set_text(stopwatch_label,"0:00:00");

	lv_obj_t* start_btn = lv_btn_create(parent, NULL);
    	lv_obj_set_event_cb(start_btn, start_stopwatch_button_handler);
    	lv_obj_align(start_btn, NULL, LV_ALIGN_IN_LEFT_MID, 15, 10);
	lv_obj_set_size(start_btn,60,40);

    	lv_obj_t* start_btn_label = lv_label_create(start_btn, NULL);
    	lv_label_set_text(start_btn_label, "Start");

	lv_obj_t* stop_btn = lv_btn_create(parent, NULL);
    	lv_obj_set_event_cb(stop_btn, stop_stopwatch_button_handler);
    	lv_obj_align(stop_btn, NULL, LV_ALIGN_IN_LEFT_MID, 15, 60);
	lv_obj_set_size(stop_btn,60,40);

    	lv_obj_t* stop_btn_label = lv_label_create(stop_btn, NULL);
    	lv_label_set_text(stop_btn_label, "Stop");

	lv_obj_t* lap_btn = lv_btn_create(parent, NULL);
    	lv_obj_set_event_cb(lap_btn, lap_stopwatch_button_handler);
    	lv_obj_align(lap_btn, NULL, LV_ALIGN_IN_LEFT_MID, 85, 10);
	lv_obj_set_size(lap_btn,60,40);

    	lv_obj_t* lap_btn_label = lv_label_create(lap_btn, NULL);
    	lv_label_set_text(lap_btn_label, "Lap");
	
	// create a lap list, set its alignment, set it hidden
	stopwatch_lap_list = lv_list_create(parent, NULL);
	lv_obj_align(stopwatch_lap_list,NULL,LV_ALIGN_CENTER,150,20);
	lv_obj_set_size(stopwatch_lap_list,100,150);
	lv_obj_set_hidden(stopwatch_lap_list, true);

}

void display_system_runtime_stat(lv_obj_t* parent)
{
	static lv_style_t label_shadow_style;
	lv_style_init(&label_shadow_style);
    	lv_style_set_text_color(&label_shadow_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);

	static char stat_buffer[512];
	lv_obj_t* stat_label = lv_label_create(parent, NULL);
	lv_obj_align(stat_label,NULL,LV_ALIGN_IN_TOP_LEFT,0,0);
	lv_obj_add_style(stat_label, LV_LABEL_PART_MAIN, &label_shadow_style);

	while(1)
	{
		vTaskGetRunTimeStats(stat_buffer);
		lv_label_set_text(stat_label,stat_buffer);
		vTaskDelay(pdMS_TO_TICKS(5000));
	}

}





