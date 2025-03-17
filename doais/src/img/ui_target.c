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

#include  "display.h"
#include  "types.h"

/*
 * ===================
 * Defines
 * -------------------
 */
#define SPEED_STR_LEN 16
#define SPEED_HEADING_LABEL_WITH 230

/*
 * ===================
 * Extern
 * -------------------
 */
LV_IMG_DECLARE(Img_target_s);
LV_IMG_DECLARE(Img_bell_s);

/*
 * ===================
 * Globals
 * -------------------
 */
static lv_obj_t *Target_display_ps,*Bell_display_ps;
static char Speed_str[SPEED_STR_LEN];
static lv_obj_t *Speed_Heading_ps;

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
}



/*
 * ===================
 * lv_target_update
 * -------------------
 *
 */
void lv_target_update(void)
{
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

