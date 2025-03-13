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
#include  "types.h"

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
static lv_obj_t *Settings_display_ps, *Light_slider_ps;
static lv_obj_t *Time_ps,*Pos_ps, *Slider_l_ps;
static char Pos_str[STR_LEN];
static char Time_str[STR_LEN];


/**
 * ===================
 * lv_settings_display
 * -------------------
 */
void lv_settings_display(lv_obj_t *parent)
{
	Settings_display_ps=lv_img_create(parent);
    lv_img_set_src(Settings_display_ps,&Img_settings_s);

    lv_display_cmd(parent);
    lv_display_status(parent);
	lv_audio_cmd(parent);
	lv_settings_data(parent);
	lv_wifi_cmd(parent);
	lv_light_cmd(parent);
}


/**
 * ===================
 * lv_settings_data
 * -------------------
 *
 */
void lv_settings_data(lv_obj_t *parent)
{
	Time_ps = lv_label_create(parent);
	lv_obj_set_style_text_font(Time_ps,&lv_font_doais_24,0);
	lv_obj_set_style_text_color(Time_ps,lv_color_white(),0);
	lv_obj_set_width(Time_ps,DATA_LABEL_WIDTH);
	lv_obj_set_style_text_align(Time_ps, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_set_pos(Time_ps,DISPLAY_DATE_POSX,DISPLAY_DATE_POSY);

	Pos_ps = lv_label_create(parent);
	lv_obj_set_style_text_font(Pos_ps,&lv_font_doais_24,0);
	lv_obj_set_style_text_color(Pos_ps,lv_color_white(),0);
	lv_obj_set_width(Pos_ps,DATA_LABEL_WIDTH);
	lv_obj_set_style_text_align(Pos_ps, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_set_pos(Pos_ps,DISPLAY_POSITION_POSX,DISPLAY_POSITION_POSY);

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
	lv_label_set_text_static(Pos_ps,Pos_str);
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
	lv_label_set_text_static(Time_ps,Time_str);
}



/*
 * ===================
 * slider_event_cb
 * -------------------
 *
 */
static void light_slider_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);
    LOG_D("light_slider_event_cb: value(%d)",(int)lv_slider_get_value(slider));
}

/*
 * ===================
 * lv_light_cmd
 * -------------------
 *
 */
#define LIGHT_SLIDER_HEIGHT 120
#define LIGHT_SLIDER_WIDTH 10

void lv_light_cmd(lv_obj_t *parent)
{
	Light_slider_ps=lv_slider_create(parent);

	lv_slider_set_range(Light_slider_ps, 0, 255);
	lv_slider_set_mode(Light_slider_ps, LV_SLIDER_MODE_NORMAL);
	lv_obj_set_size(Light_slider_ps, LIGHT_SLIDER_WIDTH, LIGHT_SLIDER_HEIGHT);
	lv_obj_set_style_bg_color(Light_slider_ps, lv_palette_darken(LV_PALETTE_GREY,4),LV_PART_MAIN);
	lv_obj_set_style_bg_color(Light_slider_ps, lv_palette_lighten(LV_PALETTE_GREY,2),LV_PART_KNOB);
	 lv_obj_set_style_bg_color(Light_slider_ps,lv_color_hex(DISPLAY_DARKGREY_RGB),LV_PART_INDICATOR);
	 lv_obj_add_event_cb(Light_slider_ps,light_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	//Slider_l_ps = lv_label_create(parent);
	//lv_label_set_text(Slider_l_ps, "0%");
	lv_obj_align(Light_slider_ps,LV_ALIGN_BOTTOM_RIGHT,-72,-35);
	lv_slider_set_value(Light_slider_ps, 20, LV_ANIM_OFF);
}



