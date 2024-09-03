#include  "ui_clock.h"
#include <stdio.h>


 extern const lv_img_dsc_t img_hand;
  void noop_update_cb(void) {}
  void noop_init_cb(lv_obj_t* parent) {}

  RTC_TimeTypeDef RTCtime;
  RTC_DateTypeDef RTCdate;



  lv_obj_t *clock_display;

  lv_meter_indicator_t *indic_sec;
  lv_meter_indicator_t *indic_min;
  lv_meter_indicator_t *indic_hour;
  lv_meter_indicator_t *indic_min_meter;
  lv_meter_indicator_t *indic_hour_meter;


  lv_obj_t *labelDate;
  lv_obj_t *labelTime;

  /**
   * A clock display 
   */
void lv_clock_display(lv_obj_t *parent) {
    //clock_display = lv_meter_create(parent);
    clock_display = lv_meter_create(lv_scr_act());
    lv_obj_set_style_pad_all(clock_display, 5, LV_PART_MAIN);

    lv_obj_set_size(clock_display, 200, 200);
    lv_obj_center(clock_display);

    labelDate = lv_label_create(parent);
    lv_obj_set_pos(labelDate, 10, 10);
#if LV_FONT_MONTSERRAT_20
    lv_obj_set_style_text_font(labelDate, &lv_font_montserrat_20, 0);
#endif
    lv_obj_align(labelDate, LV_ALIGN_TOP_LEFT, 2, 2);

    labelTime = lv_label_create(parent);
    //lv_obj_set_pos(labelTime, 250, 10);
#if LV_FONT_MONTSERRAT_20
    lv_obj_set_style_text_font(labelTime, &lv_font_montserrat_20, 0);
#endif
    lv_obj_align(labelTime, LV_ALIGN_TOP_LEFT, 230, 2);

    /*Create a scale for the minutes*/
    /*61 ticks in a 360 degrees range (the last and the first line overlaps)*/
    lv_meter_scale_t *scale = lv_meter_add_scale(clock_display);
    lv_meter_set_scale_ticks(clock_display, scale, 61, 1, 10, lv_palette_main(LV_PALETTE_GREY));
    lv_meter_set_scale_range(clock_display, scale, 0, 60, 360, 270);

    /*Create another scale for the hours. It's only visual and contains only major ticks*/
    lv_meter_scale_t *scale_hour = lv_meter_add_scale(clock_display);
    lv_meter_set_scale_ticks(clock_display, scale_hour, 12, 0, 0, lv_palette_main(LV_PALETTE_GREY));           /*12 ticks*/
    lv_meter_set_scale_major_ticks(clock_display, scale_hour, 1, 2, 20, lv_palette_main(LV_PALETTE_GREY), 10); /*Every tick is major*/
    lv_meter_set_scale_range(clock_display, scale_hour, 1, 12, 330, 300);                                      /*[1..12] values in an almost full circle*/

    /*Add a the hands from images*/
    LV_IMG_DECLARE(img_hand)
    lv_meter_indicator_t * indic_min_meter = lv_meter_add_needle_img(clock_display, scale_hour, &img_hand, 5, 5);
    lv_meter_indicator_t * indic_hour_meter = lv_meter_add_needle_img(clock_display, scale_hour, &img_hand, 5, 5);

    indic_sec = lv_meter_add_needle_line(clock_display, scale, 2, lv_palette_main(LV_PALETTE_GREY), -10);
    indic_min = lv_meter_add_needle_line(clock_display, scale, 4, lv_palette_main(LV_PALETTE_GREEN), -20);
    indic_hour = lv_meter_add_needle_line(clock_display, scale, 7, lv_palette_main(LV_PALETTE_RED), -42);

    lv_label_set_text(labelDate, " ");
    lv_label_set_text(labelTime, " ");
  }


//
//  void lv_clock_display(lv_obj_t *parent) {
//	  clock_display = lv_meter_create(lv_scr_act());
//  lv_obj_set_size(clock_display, 200, 200);
//  lv_obj_center(clock_display);
//
//  /*Create a scale for the minutes*/
//  /*61 ticks in a 360 degrees range (the last and the first line overlaps)*/
//  lv_meter_scale_t * scale_min = lv_meter_add_scale(clock_display);
//  lv_meter_set_scale_ticks(clock_display, scale_min, 61, 1, 10, lv_palette_main(LV_PALETTE_GREY));
//  lv_meter_set_scale_range(clock_display, scale_min, 0, 60, 360, 270);
//
//  /*Create another scale for the hours. It's only visual and contains only major ticks*/
//  lv_meter_scale_t * scale_hour = lv_meter_add_scale(clock_display);
//  lv_meter_set_scale_ticks(clock_display, scale_hour, 12, 0, 0, lv_palette_main(LV_PALETTE_GREY));               /*12 ticks*/
//  lv_meter_set_scale_major_ticks(clock_display, scale_hour, 1, 2, 20, lv_color_black(), 10);    /*Every tick is major*/
//  lv_meter_set_scale_range(clock_display, scale_hour, 1, 12, 330, 300);       /*[1..12] values in an almost full circle*/
//
//  LV_IMG_DECLARE(img_hand)
//
//  /*Add a the hands from images*/
//  indic_min = lv_meter_add_needle_img(clock_display, scale_min, &img_hand, 5, 5);
//  indic_hour = lv_meter_add_needle_img(clock_display, scale_min, &img_hand, 5, 5);
//  }
//
   void set_clock_value(void *indic, int32_t v) {
    lv_meter_set_indicator_value(clock_display, (lv_meter_indicator_t *)indic, v);
  }

   uint8_t minutes=0;
   void clock_update_cb(void) {
	// Getime
    set_clock_value(indic_hour, minutes>>1);
    set_clock_value(indic_min, minutes);
    minutes=(minutes==59)?minutes=0:minutes+1;
  }




