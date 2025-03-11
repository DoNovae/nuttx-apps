/**
 * =====================================
 * ui_settings.c
 * -------------------------------------
 *  AIS DoNovae
 *  www.DoNovae.com
 *  Herve Bailly
 * =====================================
 *
 */
#include <stdio.h>
#include "display.h"
//#include  "gps.h"

#ifndef LAT_LONG_SCALE
#define LAT_LONG_SCALE ((float)10000000.0)
#endif

/*
 * ===================
 * Defines
 * -------------------
 */
#define DATA_LABEL_WIDTH 200
#define DATA_LABEL_HEIGTH 53
#define STR_LEN 16
#define DISPLAY_POSITION_POSX (14)
#define DISPLAY_POSITION_POSY (53+10)
#define DISPLAY_DATE_POSX (14)
#define DISPLAY_DATE_POSY (133+10)


/*
 * ===================
 * Extern
 * -------------------
 */
LV_IMG_DECLARE(Img_settings_s);

/*
 * ===================
 * Prototypes
 * -------------------
 */

/*
 * ===================
 * Globals
 * -------------------
 */
static lv_obj_t *Settings_display_s,*Vessels_display_s;
static lv_obj_t *Time_l,*Pos_l;
static char Pos_str[STR_LEN];
static char Time_str[STR_LEN];


/**
 * ===================
 * lv_settings_display
 * -------------------
 */
void lv_settings_display(lv_obj_t *parent)
{
	Settings_display_s=lv_img_create(parent);
    lv_img_set_src(Settings_display_s,&Img_settings_s);

    lv_display_cmd(parent);
    lv_display_status(parent);
	lv_audio_cmd(parent);
	lv_settings_data(parent);
}


/**
 * ===================
 * lv_settings_data
 * -------------------
 *
 */
void lv_settings_data(lv_obj_t *parent)
{
	Time_l = lv_label_create(parent);
	lv_obj_set_style_text_font(Time_l,&lv_font_doais_24,0);
	lv_obj_set_style_text_color(Time_l,lv_color_hex(DISPLAY_WHITE_RGB),0);
	lv_obj_set_width(Time_l,DATA_LABEL_WIDTH);
	lv_obj_set_style_text_align(Time_l, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_set_pos(Time_l,DISPLAY_DATE_POSX,DISPLAY_DATE_POSY);

	Pos_l = lv_label_create(parent);
	lv_obj_set_style_text_font(Pos_l,&lv_font_doais_24,0);
	lv_obj_set_style_text_color(Pos_l,lv_color_hex(DISPLAY_WHITE_RGB),0);
	lv_obj_set_width(Pos_l,DATA_LABEL_WIDTH);
	lv_obj_set_style_text_align(Pos_l, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_set_pos(Pos_l,DISPLAY_POSITION_POSX,DISPLAY_POSITION_POSY);

	lv_settings_update_pos(0,0);
	lv_settings_update_time(0,0,0);
}


/*
 * ===================
 * lv_settings_update_pos
 * -------------------
 */
void lv_settings_update_pos(int32_t lon_d,int32_t lat_d)
{
	memset((void*)Pos_str,0,STR_LEN);
	sprintf(Pos_str,"%+07.2f%+06.2f",(float)lon_d/LAT_LONG_SCALE,(float)lat_d/LAT_LONG_SCALE);
	lv_label_set_text_static(Pos_l,Pos_str);
}


/*
 * ===================
 * lv_settings_update_time
 * -------------------
 */
void lv_settings_update_time(int32_t hour_i32, int32_t mn_i32,int32_t s_i32)
{
	memset((void*)Time_str,0,STR_LEN);
	sprintf(Time_str,"%02d:%02d:%02d",hour_i32,mn_i32,s_i32);
	lv_label_set_text_static(Time_l,Time_str);
}

