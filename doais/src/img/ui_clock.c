#include <stdio.h>
#include  "display.h"

/*
 * ===================
 * Extern
 * -------------------
 */
extern const lv_img_dsc_t Img_hand; //LV_IMG_DECLARE(Img_hand)

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
lv_obj_t *Clock_display;


RTC_TimeTypeDef RTCtime;
RTC_DateTypeDef RTCdate;

lv_obj_t *LabelDate;
lv_obj_t *LabelTime;

lv_meter_indicator_t *Indic_sec;
lv_meter_indicator_t *Indic_min;
lv_meter_indicator_t *Indic_hour;
lv_meter_indicator_t *Indic_min_meter;
lv_meter_indicator_t *Indic_hour_meter;




/**
 * -------------------
 * lv_parent
 * -------------------
 */
void lv_clock_display(lv_obj_t *parent)
{
	Clock_display = lv_meter_create(lv_scr_act());
	lv_obj_set_style_pad_all(Clock_display, 5, LV_PART_MAIN);

	lv_obj_set_size(Clock_display,200,200);
	lv_obj_center(Clock_display);

	LabelDate = lv_label_create(parent);
	lv_obj_set_pos(LabelDate, 10, 10);
#if LV_FONT_MONTSERRAT_20
	lv_obj_set_style_text_font(LabelDate, &lv_font_montserrat_20, 0);
#endif
	lv_obj_align(LabelDate, LV_ALIGN_TOP_LEFT, 2, 2);

	LabelTime = lv_label_create(parent);
#if LV_FONT_MONTSERRAT_20
	lv_obj_set_style_text_font(LabelTime, &lv_font_montserrat_20, 0);
#endif
	lv_obj_align(LabelTime, LV_ALIGN_TOP_LEFT, 230, 2);

	/*Create a scale for the minutes*/
	/*61 ticks in a 360 degrees range (the last and the first line overlaps)*/
	lv_meter_scale_t *scale = lv_meter_add_scale(Clock_display);
	lv_meter_set_scale_ticks(Clock_display, scale, 61, 1, 10, lv_palette_main(LV_PALETTE_GREY));
	lv_meter_set_scale_range(Clock_display, scale, 0, 60, 360, 270);

	/*Create another scale for the hours. It's only visual and contains only major ticks*/
	lv_meter_scale_t *scale_hour = lv_meter_add_scale(Clock_display);
	lv_meter_set_scale_ticks(Clock_display, scale_hour, 12, 0, 0, lv_palette_main(LV_PALETTE_GREY));           /*12 ticks*/
	lv_meter_set_scale_major_ticks(Clock_display, scale_hour, 1, 2, 20, lv_palette_main(LV_PALETTE_GREY), 10); /*Every tick is major*/
	lv_meter_set_scale_range(Clock_display, scale_hour, 1, 12, 330, 300);                                      /*[1..12] values in an almost full circle*/

	/*Add a the hands from images*/
	lv_meter_indicator_t * Indic_min_meter = lv_meter_add_needle_img(Clock_display, scale_hour, &Img_hand, 5, 5);
	lv_meter_indicator_t * Indic_hour_meter = lv_meter_add_needle_img(Clock_display, scale_hour, &Img_hand, 5, 5);

	Indic_sec = lv_meter_add_needle_line(Clock_display, scale, 2, lv_palette_main(LV_PALETTE_GREY), -10);
	Indic_min = lv_meter_add_needle_line(Clock_display, scale, 4, lv_palette_main(LV_PALETTE_GREEN), -20);
	Indic_hour = lv_meter_add_needle_line(Clock_display, scale, 7, lv_palette_main(LV_PALETTE_RED), -42);

	lv_label_set_text(LabelDate, " ");
	lv_label_set_text(LabelTime, " ");
}

/*
 * set_clock_value
 */
void set_clock_value(void *indic, int32_t v)
{
	lv_meter_set_indicator_value(Clock_display,(lv_meter_indicator_t *)indic,v);
}

/*
 * clock_update_cb
 */
uint8_t Minutes=0;
void clock_update_cb()
{
	// Getime
	set_clock_value(Indic_hour,Minutes>>1);
	set_clock_value(Indic_min,Minutes);
	Minutes=(Minutes==59)?Minutes=0:Minutes+1;
}




