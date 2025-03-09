#include <stdio.h>
#include  "display.h"

/*
 * ===================
 * Extern
 * -------------------
 */

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
lv_obj_t *Wind_display_s;


static lv_meter_indicator_t *Indic_wind;
static lv_meter_indicator_t *Indic_gwa_wind;
static lv_obj_t *Wind_label;
static lv_obj_t *Spd_w_label;
static lv_obj_t *Gws_label;
static lv_obj_t *Gwdt_label;


/**
 * -------------------
 * lv_Wind_display_s
 * -------------------
 */
void lv_wind_display(lv_obj_t *parent)
{
   Wind_display_s = lv_meter_create(parent);
   lv_obj_align(Wind_display_s, LV_ALIGN_CENTER, 0, 6);
   lv_obj_set_size(Wind_display_s, 210, 210);

   /*Add a scale first*/
   lv_meter_scale_t *scale = lv_meter_add_scale(Wind_display_s);
   lv_meter_set_scale_ticks(Wind_display_s, scale, 37, 2, 9, lv_palette_main(LV_PALETTE_GREY));
   lv_meter_set_scale_range(Wind_display_s, scale, -180, 180, 360, 90);

   lv_meter_scale_t *scale2 = lv_meter_add_scale(Wind_display_s);
   lv_meter_set_scale_ticks(Wind_display_s, scale2, 12, 0, 0, lv_palette_main(LV_PALETTE_GREY));
   lv_meter_set_scale_major_ticks(Wind_display_s, scale2, 1, 3, 14, lv_palette_main(LV_PALETTE_GREY), 14); /*Every tick is major*/
   lv_meter_set_scale_range(Wind_display_s, scale2, -150, 180, 330, 120);

   /*Add a red arc to the start*/
   Indic_wind = lv_meter_add_arc(Wind_display_s, scale, 4, lv_palette_main(LV_PALETTE_RED), 2);
   lv_meter_set_indicator_start_value(Wind_display_s, Indic_wind, -60);
   lv_meter_set_indicator_end_value(Wind_display_s, Indic_wind, -20);

   /*Make the tick lines red at the start of the scale*/
   Indic_wind = lv_meter_add_scale_lines(
     Wind_display_s, scale, lv_palette_main(LV_PALETTE_RED), lv_palette_main(LV_PALETTE_RED), false, 0);
   lv_meter_set_indicator_start_value(Wind_display_s, Indic_wind, -60);
   lv_meter_set_indicator_end_value(Wind_display_s, Indic_wind, -20);

   /*Add a green arc to the end*/
   Indic_wind = lv_meter_add_arc(Wind_display_s, scale, 4, lv_palette_main(LV_PALETTE_GREEN), 2);
   lv_meter_set_indicator_start_value(Wind_display_s, Indic_wind, 20);
   lv_meter_set_indicator_end_value(Wind_display_s, Indic_wind, 60);

   /*Make the tick lines green at the end of the scale*/
   Indic_wind = lv_meter_add_scale_lines(
     Wind_display_s, scale, lv_palette_main(LV_PALETTE_GREEN), lv_palette_main(LV_PALETTE_GREEN), false, 0);
   lv_meter_set_indicator_start_value(Wind_display_s, Indic_wind, 20);
   lv_meter_set_indicator_end_value(Wind_display_s, Indic_wind, 60);

   /*Add a needle line indicator*/
   Indic_wind = lv_meter_add_needle_line(Wind_display_s, scale, 6, lv_palette_main(LV_PALETTE_GREY), -10);

   /*Add a ground wind angle needle line indicator*/
   Indic_gwa_wind = lv_meter_add_needle_line(Wind_display_s, scale, 6, lv_palette_main(LV_PALETTE_ORANGE), -48);

   Wind_label = lv_label_create(parent);
   lv_obj_align(Wind_label, LV_ALIGN_TOP_LEFT, 5, 2);
#if LV_FONT_MONTSERRAT_20
   lv_obj_set_style_text_font(Wind_label, &lv_font_montserrat_20, 0);
#endif
   lv_label_set_text_static(Wind_label, "AWS:  --\nkt");

   Spd_w_label = lv_label_create(parent);
   lv_obj_align(Spd_w_label, LV_ALIGN_TOP_RIGHT, -5, 2);
#if LV_FONT_MONTSERRAT_20
   lv_obj_set_style_text_font(Spd_w_label, &lv_font_montserrat_20, 0);
#endif
   lv_label_set_text_static(Spd_w_label, "SPD:  --\n      kt");

   Gws_label = lv_label_create(parent);
   lv_obj_align(Gws_label, LV_ALIGN_BOTTOM_LEFT, 5, -2);
#if LV_FONT_MONTSERRAT_20
   lv_obj_set_style_text_font(Gws_label, &lv_font_montserrat_20, 0);
#endif
   lv_label_set_text_static(Gws_label, "GWS:\n-- kt");

   Gwdt_label = lv_label_create(parent);
   lv_obj_align(Gwdt_label, LV_ALIGN_BOTTOM_RIGHT, -5, -2);
#if LV_FONT_MONTSERRAT_20
   lv_obj_set_style_text_font(Gwdt_label, &lv_font_montserrat_20, 0);
#endif
   //lv_label_set_text_static(Gwdt_label, "GWD:\n--" LV_SYMBOL_DEGREES "t");
   lv_label_set_text_static(Gwdt_label, "GWD:\n-- D t");
 }

/* ==================
 * wind_update_cb
 * ------------------
 */

 void wind_update_cb()
 {
//   lv_label_set_text(Wind_label,
//                     (String("AWS:   ") += (fresh(shipDataModel.environment.wind.apparent_wind_speed.age) ? String(shipDataModel.environment.wind.apparent_wind_speed.kn, 1) += "\nkt" : String("--\nkt")))
//                       .c_str());
//   lv_label_set_text(Spd_w_label,
//                     (String("SPD:  ") += (fresh(shipDataModel.navigation.speed_through_water.age) ? String(shipDataModel.navigation.speed_through_water.kn, 1) += "\n     kt" : String("--\n     kt")))
//                       .c_str());
//   lv_label_set_text(Gws_label,
//                     (String("GWS:\n") += (fresh(shipDataModel.environment.wind.ground_wind_speed.age) ? String(shipDataModel.environment.wind.ground_wind_speed.kn, 1) += " kt" : String("-- kt")))
//                       .c_str());
//   lv_label_set_text(Gwdt_label,
//                     (String("GWD:\n") += (fresh(shipDataModel.environment.wind.ground_wind_dir_true.age) ? String(shipDataModel.environment.wind.ground_wind_dir_true.deg, 0) += LV_SYMBOL_DEGREES "t" : String("--" LV_SYMBOL_DEGREES "t")))
//                       .c_str());
//   set_wind_value(Indic_wind, fresh(shipDataModel.environment.wind.apparent_wind_angle.age) ? shipDataModel.environment.wind.apparent_wind_angle.deg : 0);
//   set_wind_value(Indic_gwa_wind, fresh(shipDataModel.environment.wind.ground_wind_angle.age) ? shipDataModel.environment.wind.ground_wind_angle.deg : 0);
 }





