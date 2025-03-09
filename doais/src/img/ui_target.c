#include <stdio.h>
#include <lvgl/src/misc/lv_anim.h>
#include  "display.h"
#include  "types.h"


/*
 * ===================
 * Defines
 * -------------------
 */
#define SPEED_HEADING_LABEL_WITH 230
#define DISPLAY_CMD_BTN_WITH 148
#define DISPLAY_CMD_BTN_HIGH 24
#define TARGET_CMD_BTN_WITH 99
#define TARGET_CMD_BTN_HIGH 48
#define TARGET_AUDIO_BTN_WITH 49
#define TARGET_AUDIO_BTN_HIGH 24
#define BUTTON_BG_COLOR lv_palette_lighten(LV_PALETTE_GREY,2)
#define BUTTON_TEXT_COLOR lv_palette_darken(LV_PALETTE_GREY,3)


/*
 * ===================
 * Extern
 * -------------------
 */
LV_IMG_DECLARE(Img_target_s);
LV_IMG_DECLARE(Img_bell_s);



/*
 * ===================
 * Static prototypes
 * -------------------
 */
void displays_cmd_event_cb(lv_event_t *e);
void target_cmd_event_cb(lv_event_t *e);
void target_audio_event_cb(lv_event_t *e);
void lv_target_bell(lv_obj_t *parent);

/*
 * ===================
 * Globals
 * -------------------
 */
static lv_obj_t *Target_display_s,*Bell_display_s;
static lv_obj_t *Speed_Heading_l,*Status_header_l;
static lv_obj_t *Target_display_btn;
static lv_obj_t *Target_btn;
static lv_obj_t *Audio_btn,*Audio_l;
static lv_style_t Style_btn, Style_bg;
static char Status_str[]={LV_SYMBOL_SIGNAL LV_SYMBOL_WIFI LV_SYMBOL_VOLUME_MID};


static const char *Target_display_btnm_map[]=
{
		"TGT",LV_SYMBOL_LEFT,LV_SYMBOL_RIGHT,""
};

static const char *Target_btnm_map[]=
{
		"+","-","\n",
		" ","X",""
};



/*
 * ===============================
 *           FUNCTIONS
 * ===============================
 */

/*
 * ===================
 * lv_display_init
 * -------------------
 */
void lv_display_init(void)
{
	/*
	 * Displays
	 */
	Displays_as[DISPLAY_TARGET_ID].display_ps=lv_obj_create(NULL);
	Displays_as[DISPLAY_TARGET_ID].init_cb=lv_target_display;
	Displays_as[DISPLAY_TARGET_ID].update_cb=lv_target_update;
	Displays_as[DISPLAY_VESSELS_ID].display_ps=lv_obj_create(NULL);
	Displays_as[DISPLAY_VESSELS_ID].init_cb=lv_vessels_display;
	Displays_as[DISPLAY_VESSELS_ID].update_cb=lv_vessels_update;
	Displays_as[DISPLAY_SETTINGS_ID].display_ps=lv_obj_create(NULL);
	Displays_as[DISPLAY_SETTINGS_ID].init_cb=lv_settings_display;
	Displays_as[DISPLAY_SETTINGS_ID].update_cb=lv_settings_update;
	Display_id=DISPLAY_TARGET_ID;
	lv_display_load(Display_id);

	/*
	 * Buttons styles
	 */
	lv_style_init(&Style_bg);
	lv_style_set_pad_all(&Style_bg,0);
	lv_style_set_pad_gap(&Style_bg,0);
	lv_style_set_clip_corner(&Style_bg,true);
	lv_style_set_radius(&Style_bg, LV_RADIUS_CIRCLE);
	lv_style_set_border_width(&Style_bg, 0);
	lv_style_set_border_width(&Style_bg, 0);
	lv_style_set_border_color(&Style_bg,lv_color_hex(DISPLAY_DARKGREY_RGB));
	lv_style_set_bg_color(&Style_bg,BUTTON_BG_COLOR);

	lv_style_init(&Style_btn);
	lv_style_set_radius(&Style_btn, 0);
	lv_style_set_border_width(&Style_btn, 1);
	lv_style_set_border_opa(&Style_btn, LV_OPA_50);
	lv_style_set_border_side(&Style_btn,LV_BORDER_SIDE_INTERNAL);
	lv_style_set_radius(&Style_btn, 0);
	lv_style_set_border_color(&Style_btn,lv_color_hex(DISPLAY_DARKGREY_RGB));
	lv_style_set_bg_color(&Style_btn,BUTTON_BG_COLOR);
	lv_style_set_text_font(&Style_btn,&lv_font_montserrat_12);
	lv_style_set_text_color(&Style_btn,BUTTON_TEXT_COLOR);
}

/*
 * ===================
 * lv_display_inits
 * -------------------
 */
void lv_display_load(Display_id_e id_e)
{
	// Remove all the children of display_ps
	lv_obj_clean(Displays_as[id_e].display_ps);
	lv_scr_load(Displays_as[id_e].display_ps);
	Displays_as[id_e].init_cb(Displays_as[id_e].display_ps);
}

/*
 * ===================
 * lv_display_next
 * -------------------
 */
void lv_display_next(void)
{
	int8_t id_i8=(uint8_t)Display_id;
	id_i8=(id_i8==(DISPLAY_NBR-1))?0:id_i8+1;
	Display_id=(Display_id_e)id_i8;
}

void lv_display_prev(void)
{
	int8_t id_i8=(uint8_t)Display_id;
	id_i8=(id_i8==0)?(DISPLAY_NBR-1):id_i8-1;
	Display_id=(Display_id_e)id_i8;
}


/*
 * ===================
 * displays_cmd_event_cb
 * -------------------
 *
 */
void displays_cmd_event_cb(lv_event_t *e)
{
	lv_obj_t *obj = lv_event_get_target(e);
	lv_event_code_t code = lv_event_get_code(e);
	Display_id_e disp_id_bk_e=Display_id;
	bool refresh_b=false;

	// Prevent LVGL sending further input-device-related events
	lv_indev_wait_release(lv_indev_get_act());

	if (code==LV_EVENT_PRESSED)
	{
		uint32_t id = lv_btnmatrix_get_selected_btn(obj);
		const char *txt = lv_btnmatrix_get_btn_text(obj, id);

		if (txt != NULL)
		{
			LOG_D("displays_cmd_event_cb: %s",txt);
			if (strcmp("TGT",txt)==0)
			{
				Display_id=DISPLAY_TARGET_ID;
				refresh_b=true;
			} else if (strcmp(LV_SYMBOL_LEFT,txt)==0)
			{
				lv_display_prev();
				refresh_b=true;
			} else if (strcmp(LV_SYMBOL_RIGHT,txt) == 0)
			{
				lv_display_next();
				refresh_b=true;
			} else
			{
				LOG_W("displays_cmd_event_cb: wrong cmd(%s)",txt);
			}
			if (refresh_b)
			{
				LOG_D("displays_cmd_event_cb: Display_id(%d)",(uint8_t)Display_id);
				// Clean current display
				lv_obj_clean(Displays_as[disp_id_bk_e].display_ps);
				// load next display
				lv_display_load(Display_id);
			}
		}
	}
}

/*
 * ===================
 * lv_display_cmd
 * -------------------
 * Displays button for
 * shifting screens.
 *
 */
void lv_display_cmd(lv_obj_t *parent)
{
	Target_display_btn = lv_btnmatrix_create(parent);
	lv_btnmatrix_set_map(Target_display_btn,Target_display_btnm_map);
	lv_obj_add_style(Target_display_btn,&Style_bg, 0);
	lv_obj_add_event_cb(Target_display_btn,displays_cmd_event_cb, LV_EVENT_PRESSED, NULL);
	lv_obj_set_size(Target_display_btn,DISPLAY_CMD_BTN_WITH,DISPLAY_CMD_BTN_HIGH);
	lv_obj_align(Target_display_btn,LV_ALIGN_BOTTOM_LEFT,ALIGN_LEFT,ALIGN_BOTTOM);
	lv_obj_add_style(Target_display_btn, &Style_btn, LV_PART_ITEMS);
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
 * lv_target_cmd
 * -------------------
 *
 */
void lv_target_cmd(lv_obj_t *parent)
{
	Target_btn = lv_btnmatrix_create(parent);
	lv_btnmatrix_set_map(Target_btn,Target_btnm_map);
	lv_obj_add_style(Target_btn,&Style_bg,0);
	lv_obj_add_event_cb(Target_btn,target_cmd_event_cb, LV_EVENT_PRESSED,NULL);
	lv_obj_set_size(Target_btn,TARGET_CMD_BTN_WITH,TARGET_CMD_BTN_HIGH);
	// From top : 60 - from left: 4
	lv_obj_set_pos(Target_btn,4,60);
	lv_obj_add_style(Target_btn,&Style_btn,LV_PART_ITEMS);
}



/*
 * ===================
 * autopilot_event_cb
 * -------------------
 *
 */
void target_audio_event_cb(lv_event_t *e)
{
	lv_event_code_t code=lv_event_get_code(e);
	if (code==LV_EVENT_PRESSED)
	{
		LOG_D("target_audio_event_cb");
	}
}

/*
 * ===================
 * lv_audio_cmd
 * -------------------
 *
 */
void lv_audio_cmd(lv_obj_t *parent)
{
	Audio_btn=lv_btn_create(parent);
	lv_obj_add_style(Audio_btn,&Style_bg,0);
	lv_obj_add_event_cb(Audio_btn,target_audio_event_cb,LV_EVENT_PRESSED,NULL);;
	lv_obj_add_style(Audio_btn,&Style_btn,LV_PART_ITEMS);
	lv_obj_set_size(Audio_btn,TARGET_AUDIO_BTN_WITH,TARGET_AUDIO_BTN_HIGH);
	lv_obj_align(Audio_btn,LV_ALIGN_BOTTOM_RIGHT,ALIGN_RIGHT,ALIGN_BOTTOM);
	Audio_l=lv_label_create(Audio_btn);
	lv_obj_set_style_text_color(Audio_l,BUTTON_TEXT_COLOR,0);
	lv_label_set_text(Audio_l,LV_SYMBOL_VOLUME_MID);
	lv_obj_center(Audio_l);
}


/*
 * ===========================
 * lv_display_header_status
 * ---------------------------
 *   Audio
 *     - LV_SYMBOL_VOLUME_MID / LV_SYMBOL_MUTE
 *   Wifi
 *   	- LV_SYMBOL_WIFI
 *   Signal
 *      - LV_SYMBOL_SIGNAL
 * ---------------------------
 *
 */
void lv_display_status(lv_obj_t *parent)
{
	Status_header_l=lv_label_create(parent);
	lv_obj_set_style_text_font(Status_header_l,&lv_font_doais_status,0);
	lv_obj_set_style_text_color(Status_header_l,lv_color_hex(DISPLAY_GREEN_RGB),0);
	//lv_label_set_text(Status_header_l,LV_SYMBOL_SIGNAL LV_SYMBOL_WIFI LV_SYMBOL_VOLUME_MID );
	lv_label_set_text(Status_header_l,Status_str);
	//lv_label_set_text(Status_header_l,LV_SYMBOL_SIGNAL"    "LV_SYMBOL_VOLUME_MID );
	lv_obj_align(Status_header_l, LV_ALIGN_TOP_RIGHT,ALIGN_RIGHT,3);
}








/*
 * ===============================
 *           TARGET FUNCTIONS
 * ===============================
 */



/**
 * ===================
 * lv_target_display
 * -------------------
 *
 */
void lv_target_display(lv_obj_t *parent)
{
	Target_display_s=lv_img_create(parent);
	lv_img_set_src(Target_display_s,&Img_target_s);
	lv_obj_align(Target_display_s,LV_ALIGN_CENTER, 0, 0);

	Speed_Heading_l = lv_label_create(parent);
	lv_obj_set_style_text_font(Speed_Heading_l,&lv_font_doais_speed,0);
	lv_obj_set_style_text_color(Speed_Heading_l,lv_color_hex(DISPLAY_WHITE_RGB),0);

	lv_label_set_text_fmt(Speed_Heading_l,"%04.1fk%03d" LV_SYMBOL_DEGREE,(float)6.7,270);

	lv_obj_set_width(Speed_Heading_l,SPEED_HEADING_LABEL_WITH);
	lv_obj_set_style_text_align(Speed_Heading_l, LV_TEXT_ALIGN_LEFT, 0);
	lv_obj_align(Speed_Heading_l, LV_ALIGN_TOP_LEFT,ALIGN_LEFT,4);

	lv_display_cmd(parent);
	lv_display_status(parent);
	lv_target_cmd(parent);
	lv_audio_cmd(parent);
	lv_target_bell(parent);
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
#define BELL_ANIM_DURATION_MS 500
static void set_angle(void * img, int32_t v)
{
	lv_img_set_angle(img, v);
}

void lv_target_bell(lv_obj_t *parent)
{
	lv_anim_t a;
	Bell_display_s=lv_img_create(parent);
	lv_img_set_src(Bell_display_s,&Img_bell_s);
	lv_obj_set_pos(Bell_display_s,BELL_CORNER_RL_POSX,BELL_CORNER_RL_POSY);

	/*
	 * Animation
	 */
/*
	lv_img_set_pivot(Bell_display_s,BELL_CORNER_RL_POSX,BELL_CORNER_RL_POSY);
	lv_anim_init(&a);
	lv_anim_set_var(&a,Bell_display_s);
	lv_anim_set_exec_cb(&a, set_angle);
	lv_anim_set_values(&a,-45,45);
	lv_anim_set_time(&a,BELL_ANIM_DURATION_MS);
	lv_anim_set_repeat_count(&a,LV_ANIM_REPEAT_INFINITE);
	lv_anim_start(&a);
	//lv_obj_add_flag(Bell_display_s,LV_OBJ_FLAG_HIDDEN);
*/
}
