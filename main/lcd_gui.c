#include "lcd_gui.h"


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
	}
}

 void set_date_calendar_event_handler(lv_obj_t * obj, lv_event_t event)
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

 void create_set_datetime_window(lv_obj_t * parent)
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

