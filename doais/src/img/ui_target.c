/**
 * =====================================
 * ui_target.c
 * -------------------------------------
 *  AIS DoNovae
 *  www.DoNovae.com
 *  Herve Bailly
 * =====================================
 *
 */

#include <stdio.h>
#include <lvgl/src/misc/lv_anim.h>
#include <lvgl/src/misc/lv_types.h>
#include <lvgl/src/draw/lv_draw.h>
#include <lvgl/src/draw/lv_draw_rect.h>
#include <lvgl/src/draw/lv_draw_arc.h>
#include <lvgl/src/draw/lv_draw_label.h>
#include <lvgl/src/core/lv_obj_draw.h>


#include  "display.h"
#include  "types.h"

/*
 * ===================
 * Defines
 * -------------------
 */
#define SPEED_STR_LEN 16
#define SPEED_HEADING_LABEL_WITH 230

#define TARGET_CANVAS_WIDTH  224
#define TARGET_CANVAS_HEIGHT 224
#define CANVAS_BUF_SIZE (TARGET_CANVAS_WIDTH*TARGET_CANVAS_HEIGHT*LV_IMG_PX_SIZE_ALPHA_BYTE)

/*
 * ===================
 * Extern
 * -------------------
 */
LV_IMG_DECLARE(Img_target_s);
LV_IMG_DECLARE(Img_bell_s);
LV_IMG_DECLARE(Target_canavas_s);


/*
 * ===================
 * Globals
 * -------------------
 */
static lv_obj_t *Target_display_ps,*Bell_display_ps;
static char Speed_str[SPEED_STR_LEN];
static lv_obj_t *Speed_Heading_ps, *Target_canvas_ps;

static uint8_t Target_canavas_au8[CANVAS_BUF_SIZE];



/*
 * ===================
 * Prototypes
 * -------------------
 */
void lv_display_target(lv_obj_t *parent);
void lv_update_target(void);
void lv_target_bell(lv_obj_t *parent);


/*
 * ================================================================
 *           TARGET FUNCTIONS
 */

/**
 * ===================
 * lv_display_speed
 * -------------------
 *
 */
void lv_display_speed(lv_obj_t *parent)
{
	Speed_Heading_ps = lv_label_create(parent);
	lv_obj_set_style_text_font(Speed_Heading_ps,&lv_font_doais_speed,0);
	lv_obj_set_style_text_color(Speed_Heading_ps,lv_color_hex(DISPLAY_WHITE_RGB),0);
	lv_obj_set_width(Speed_Heading_ps,SPEED_HEADING_LABEL_WITH);
	lv_obj_set_style_text_align(Speed_Heading_ps, LV_TEXT_ALIGN_LEFT, 0);
	lv_obj_align(Speed_Heading_ps, LV_ALIGN_TOP_LEFT,0,4);
	lv_update_speed((float)0.0,0);
}

/*
 * ===================
 * lv_update_speed
 * -------------------
 */
void lv_update_speed(float speed_f, uint16_t heading_u16)
{
	uint16_t spd_u16;

	spd_u16=(uint16_t)(speed_f*10+(float)0.5);
	memset((void*)Speed_str,0,SPEED_STR_LEN);
	sprintf(Speed_str,"%03dk%03d" LV_SYMBOL_DEGREE,spd_u16,heading_u16);
	lv_label_set_text_static(Speed_Heading_ps,Speed_str);
}

/**
 * ===================
 * lv_target_display
 * -------------------
 *
 */
void lv_target_display(lv_obj_t *parent)
{
	/*
	 * Display bkgd image
	 */
	Target_display_ps=lv_img_create(parent);
	lv_img_set_src(Target_display_ps,&Img_target_s);
	lv_obj_align(Target_display_ps,LV_ALIGN_CENTER, 0, 0);

	lv_display_speed(parent);
	lv_display_speed(parent);

	lv_display_cmd(parent);
	lv_target_cmd(parent);
	lv_audio_cmd(parent);
	lv_target_bell(parent);
	lv_display_status(parent);

	lv_display_target(parent);
}



/*
 * ===================
 * lv_target_update
 * -------------------
 *
 */
void lv_target_update(void)
{
	lv_update_target();
}

/*
 * ===================
 * lv_target_bell
 * -------------------
 *
 */
#define BELL_CORNER_RL_POSX 23
#define BELL_CORNER_RL_POSY 129
#define BELL_CORNER_WIDTH 50
#define BELL_CORNER_HEIGHT 50
#define BELL_ANIM_DURATION_MS 250

/*
 * Set the zoom factor of the image.
 * Zoom factor.
 *    - 256 or LV_ZOOM_IMG_NONE for no zoom
 *    - <256: scale down
 *    - >256 scale up
 *    - 128 half size
 *    - 512 double size
 */
static void set_zoom(void * img, int32_t v)
{
	lv_img_set_zoom(img, v);
}


/*
 * ===================
 * lv_target_bell
 * -------------------
 *
 */
void lv_target_bell(lv_obj_t *parent)
{
	lv_anim_t a;
	Bell_display_ps=lv_img_create(parent);
	lv_img_set_src(Bell_display_ps,&Img_bell_s);
	lv_obj_set_pos(Bell_display_ps,BELL_CORNER_RL_POSX,BELL_CORNER_RL_POSY);
	lv_img_set_size_mode(Bell_display_ps,LV_IMG_SIZE_MODE_VIRTUAL);

	/*
	 * Animation
	 */
	lv_anim_init(&a);
	lv_anim_set_var(&a,Bell_display_ps);
	lv_anim_set_exec_cb(&a, set_zoom);
	lv_anim_set_values(&a,256,300);
	lv_anim_set_time(&a,BELL_ANIM_DURATION_MS);
	lv_anim_set_repeat_count(&a,LV_ANIM_REPEAT_INFINITE);

	lv_anim_start(&a);
	//lv_obj_add_flag(Bell_display_ps,LV_OBJ_FLAG_HIDDEN);
}


/*
 * ===================
 * target_cmd_event_cb
 * -------------------
 *
 */
void target_cmd_event_cb(lv_event_t *e)
{
	lv_obj_t *obj = lv_event_get_target(e);
	lv_event_code_t code = lv_event_get_code(e);

	LOG_D("target_cmd_event_cb: code(%d)",code);

	// Prevent LVGL sending further input-device-related events
	lv_indev_wait_release(lv_indev_get_act());

	if (code==LV_EVENT_PRESSED)
	{
		uint32_t id = lv_btnmatrix_get_selected_btn(obj);
		const char *txt = lv_btnmatrix_get_btn_text(obj, id);

		if (txt != NULL)
		{
			LOG_D("target_cmd_event_cb: %s",txt);
			if (strcmp("+",txt)==0)
			{
			} else if (strcmp("-",txt)==0)
			{
			} else if (strcmp("X",txt) == 0)
			{
			} else
			{
				LOG_W("target_cmd_event_cb: wrong cmd(%s)",txt);
			}
		}
	}
}

/*
 * ===================
 * lv_display_target
 * -------------------
 *
 */
#define TARGET_CORNER_RL_POSX 96
#define TARGET_CORNER_RL_POSY 16
void lv_display_target(lv_obj_t *parent)
{
	Target_canvas_ps=lv_canvas_create(parent);
	memset(Target_canavas_au8,0,CANVAS_BUF_SIZE);
	lv_canvas_set_buffer(Target_canvas_ps,Target_canavas_au8,TARGET_CANVAS_WIDTH,TARGET_CANVAS_HEIGHT,LV_IMG_CF_TRUE_COLOR_ALPHA);
	//lv_canvas_fill_bg(Target_canvas_ps,lv_color_white(),LV_OPA_COVER);
	lv_obj_set_pos(Target_canvas_ps,TARGET_CORNER_RL_POSX,TARGET_CORNER_RL_POSY);

	lv_update_status();
}


/*
 * ===================
 * lv_update_target
 * -------------------
 *
 */
void lv_update_target(void)
{
	/*
	 * Edit Target_canavas_au8
	 */
	lv_canvas_copy_buf(Target_canvas_ps,Target_canavas_s.data,0,0,TARGET_CANVAS_WIDTH,TARGET_CANVAS_HEIGHT);

	/*
	 * Display target
	 */
	lv_canvas_set_buffer(Target_canvas_ps,Target_canavas_au8,TARGET_CANVAS_WIDTH,TARGET_CANVAS_HEIGHT,LV_IMG_CF_TRUE_COLOR_ALPHA);
}





void lv_display_target_ais()
{
	float scale_px_nm_d32;
	float max_nm_d32;
	float alert_circ_d32;

	/*
	 * Edit Target_canavas_au8
	 */
	lv_canvas_copy_buf(Target_canvas_ps,Target_canavas_s.data,0,0,TARGET_CANVAS_WIDTH,TARGET_CANVAS_HEIGHT);

/*	lv_draw_label_dsc_t label_dsc;
	    lv_draw_label_dsc_init(&label_dsc);
	    label_dsc.color = lv_palette_main(LV_PALETTE_ORANGE);
	    label_dsc.text = "S";*/


	lv_draw_rect_dsc_t rect_dsc;
	    lv_draw_rect_dsc_init(&rect_dsc);
	    rect_dsc.bg_opa = LV_OPA_COVER;
	    rect_dsc.border_width = 2;
	    rect_dsc.border_opa = LV_OPA_90;
	    rect_dsc.border_color = lv_color_black();

	lv_layer_type_t layer;
	lv_canvas_init_layer(Target_canvas_ps, &layer);
	 lv_area_t coords_rect = {30, 20, 100, 70};
	    lv_draw_rect(&layer, &rect_dsc, &coords_rect);

	    //lv_draw_label(&layer, &label_dsc, &coords_text);

	    lv_canvas_finish_layer(Target_canvas_ps, &layer);




	/*
	 * Circles
	 */
	//	Ais_display::target.drawCircle(DISPLAY_TARGET_CENTER_X,DISPLAY_TARGET_CENTER_Y,DISPLAY_TARGET_R3,TFT_BLACK);
	//	scale_px_nm_d32=(float)DISPLAY_TARGET_R3/Monitoring.settings_s.display_target_step_nm_u32/(float)MONITORING_DISPLAY_STEPS_NB;
	//	max_nm_d32=(float)settings_s.display_target_step_nm_u32*(float)MONITORING_DISPLAY_STEPS_NB;
	//	alert_circ_d32=(float)DISPLAY_TARGET_R3*(float)settings_s.cpa_warn_10thnm_u32/(float)10.0/max_nm_d32;
	//	switch (zoom_s){
	//	case AIS_MONITORING_Z0:
	//		Ais_display::target.drawCircle(DISPLAY_TARGET_CENTER_X,DISPLAY_TARGET_CENTER_Y,DISPLAY_TARGET_R2,TFT_BLACK);
	//		Ais_display::target.drawCircle(DISPLAY_TARGET_CENTER_X,DISPLAY_TARGET_CENTER_Y,DISPLAY_TARGET_R1,TFT_BLACK);
	//		break;
	//	case AIS_MONITORING_Z1:
	//		Ais_display::target.drawCircle(DISPLAY_TARGET_CENTER_X,DISPLAY_TARGET_CENTER_Y,DISPLAY_TARGET_R32,TFT_BLACK);
	//		scale_px_nm_d32*=(float)3/2;
	//		max_nm_d32=(float)max_nm_d32*(float)2.0/(float)3.0;
	//		alert_circ_d32=alert_circ_d32*(float)3.0/(float)2.0;
	//		break;
	//	case AIS_MONITORING_Z2:
	//		scale_px_nm_d32*=(float)3;
	//		max_nm_d32=(float)max_nm_d32/(float)3;
	//		alert_circ_d32=alert_circ_d32*(float)3.0;
	//		break;
	//	}
	//	Ais_display::target.drawCircle(DISPLAY_TARGET_CENTER_X,DISPLAY_TARGET_CENTER_Y,(int16_t)(alert_circ_d32+(float)0.5),DISPLAY_RED);
	//
	//	/*
	//	 * Lines
	//	 */
	//	Ais_display::target.drawLine(0,DISPLAY_TARGET_CENTER_Y,DISPLAY_TARGET_SX,DISPLAY_TARGET_CENTER_Y,TFT_BLACK);
	//	Ais_display::target.drawLine(DISPLAY_TARGET_CENTER_X,0,DISPLAY_TARGET_CENTER_X,DISPLAY_TARGET_SY,TFT_BLACK);
	//
	//	/*
	//	 * Cross
	//	 */
	//	if (is_cross_b) {
	//		Ais_display::target.drawLine(DISPLAY_TARGET_CENTER_X-10,DISPLAY_TARGET_CENTER_Y-10,DISPLAY_TARGET_CENTER_X+10,DISPLAY_TARGET_CENTER_Y+10,DISPLAY_RED);
	//		Ais_display::target.drawLine(DISPLAY_TARGET_CENTER_X-10,DISPLAY_TARGET_CENTER_Y+10,DISPLAY_TARGET_CENTER_X+10,DISPLAY_TARGET_CENTER_Y-10,DISPLAY_RED);
	//	}
	//
	//	/*
	//	 * Display filtered vessels
	//	 */
	//	display_target_ais_filtering(max_nm_d32,scale_px_nm_d32,MONITORING_STATUS_NONE);
	//	display_target_ais_filtering(max_nm_d32,scale_px_nm_d32,MONITORING_STATUS_ALERT);
	//
	//	// Scale in NM
	//	Ais_display::draw_target_scale((int16_t)(max_nm_d32+(float)0.5));
	//	M5.Lcd.pushImage(DISPLAY_TARGET_POSX,DISPLAY_TARGET_POSY,DISPLAY_TARGET_SX,DISPLAY_TARGET_SY,(m5gfx::rgb565_t*)Ais_display::target.getBuffer());
	//	*/


	/*
	 * Display target
	 */
	lv_canvas_set_buffer(Target_canvas_ps,Target_canavas_au8,TARGET_CANVAS_WIDTH,TARGET_CANVAS_HEIGHT,LV_IMG_CF_TRUE_COLOR_ALPHA);
}
