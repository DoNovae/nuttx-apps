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
#include <nuttx/lcd/lcd_dev.h>
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
#define DISPLAY_POSITION_POSX (14)
#define DISPLAY_POSITION_POSY (53+10)
#define DISPLAY_DATE_POSX (14)
#define DISPLAY_DATE_POSY (133+10)
#define STR_LEN 16

#define DISPLAY_CMD_BTN_WIDTH 148
#define DISPLAY_CMD_BTN_HIGH 24
#define TARGET_CMD_BTN_WIDTH 99
#define TARGET_CMD_BTN_HIGH 48
#define TARGET_AUDIO_BTN_WIDTH 49
#define TARGET_AUDIO_BTN_HIGH 24
#define BUTTON_BG_COLOR lv_palette_lighten(LV_PALETTE_GREY,2)
#define BUTTON_TEXT_COLOR lv_palette_darken(LV_PALETTE_GREY,3)

#define CANVAS_HD_WIDTH  60
#define CANVAS_HD_HEIGHT  20
#define CANVAS_ICONE_WIDTH  20
#define CANVAS_ICONE_HEIGHT  20
#define CANVAS_BUF_SIZE (CANVAS_HD_WIDTH*CANVAS_HD_HEIGHT*LV_COLOR_SIZE/8)

#define LCD_DEVPATH "/dev/lcd0"
/*
 * cf CONFIG_LCD_MAXPOWER
 *
 */
#define LCD_LEVEL_MAX 15
#define LCD_LEVEL_MIN 8

/*
 * ===================
 * Extern
 * -------------------
 */
LV_IMG_DECLARE(Img_wifi_bk_s);
LV_IMG_DECLARE(Img_wifi_on_s);
LV_IMG_DECLARE(Img_white_sq_s);
LV_IMG_DECLARE(Img_spk_on_s);

LV_IMG_DECLARE(Img_gps0_s);
LV_IMG_DECLARE(Img_gps1_s);
LV_IMG_DECLARE(Img_gps2_s);
LV_IMG_DECLARE(Img_gps3_s);
LV_IMG_DECLARE(Img_settings_s);

extern gui_animation_t Gui_animation_s;// Cf doais_gui.c



/*
 * ===================
 * Static prototypes
 * -------------------
 */
void displays_cmd_event_cb(lv_event_t *e);
void target_cmd_event_cb(lv_event_t *e);
void target_audio_event_cb(lv_event_t *e);
void lv_target_bell(lv_obj_t *parent);
void lv_display_speed(lv_obj_t *parent);
void lv_display_status(lv_obj_t *parent);
void lv_update_status(void);
void lv_target_cmd(lv_obj_t *parent);
void lv_settings_data(lv_obj_t *parent);
void lv_light_cmd(lv_obj_t *parent);

/*
 * ===================
 * Globals
 * -------------------
 */
static lv_obj_t *Status_header_ps;
static lv_obj_t *Target_display_bt_ps;
static lv_obj_t *Target_bt_ps;
static lv_obj_t *Audio_bt_ps, *Wifi_bt_ps;
static lv_style_t Style_btn, Style_bg;
static lv_obj_t *Canvas_head_ps;
static uint8_t Canvas_header_au8[CANVAS_BUF_SIZE];

static lv_obj_t *Settings_display_ps, *Light_slider_ps;
static lv_obj_t *Time_ps,*Pos_ps;
static char Pos_str[STR_LEN];
static char Time_str[STR_LEN];
static int Lcd_fd=-1;
static uint8_t Lcd_level_u8=0;

static const char *Target_display_bt_psm_map[]=
{
		"TGT",LV_SYMBOL_LEFT,LV_SYMBOL_RIGHT,""
};

static const char *Target_bt_psm_map[]=
{
		"+","-","\n",
		" ","X",""
};

static const char *Audio_bt_psm_map[]=
{
		LV_SYMBOL_VOLUME_MID,""
};

static const char *Wifi_bt_psm_map[]=
{
		LV_SYMBOL_WIFI,""
};



/*
 * ===================
 * Prototypes
 * -------------------
 */
uint8_t lcd_get_power();
int lcd_set_power(uint8_t level_u8);


/*
 * =========================================================================
 *           FUNCTIONS
 * ===============================
 */

/*
 * ====================
 * lcd_get_power
 * --------------------
 *
 */

uint8_t lcd_get_power()
{
	uint8_t level_u8;
	int ret;

	if (Lcd_fd<0)
	{
		LOG_E("display_get_power: ERR LCD not opened");
		return 0;
	}

	ret=ioctl(Lcd_fd,LCDDEVIO_GETPOWER,&level_u8);
	if (ret< 0)
	{
		int errcode = errno;
		LOG_E("lcd_get_power: ERR - ioctl(LCDDEVIO_GETPOWER) failed: %d\n", errcode);
		return 0;
	}
	return level_u8;
}


/*
 * ====================
 * lcd_set_power
 * --------------------
 *   - 0: full off
 *   - CONFIG_LCD_MAXPOWER: max
 * --------------------
 */
int lcd_set_power(uint8_t level_u8)
{
	int ret;

	if (Lcd_fd<0)
	{
		LOG_E("display_set_power: ERR LCD not opened");
		return 0;
	}

	ret=ioctl(Lcd_fd,LCDDEVIO_SETPOWER,level_u8);
	if (ret<0)
	{
		int errcode=errno;
		LOG_E("lcd_set_power: ERR - ioctl(LCDDEVIO_SETPOWER) failed: %d\n", errcode);
	}
	return ret;
}


void lv_update(){};

/*
 * ===================
 * lv_display_init
 * -------------------
 */

void lv_button_init()
{
	/*
	 * Buttons styles
	 */
	lv_style_init(&Style_bg);
	lv_style_set_pad_all(&Style_bg,0);
	lv_style_set_pad_gap(&Style_bg,0);
	lv_style_set_clip_corner(&Style_bg,true);
	lv_style_set_radius(&Style_bg,LV_RADIUS_CIRCLE);
	lv_style_set_border_width(&Style_bg, 0);
	lv_style_set_border_color(&Style_bg,lv_color_hex(DISPLAY_DARKGREY_RGB));
	lv_style_set_bg_color(&Style_bg,BUTTON_BG_COLOR);

	lv_style_init(&Style_btn);
	lv_style_set_radius(&Style_btn, 0);
	lv_style_set_border_width(&Style_btn, 1);
	lv_style_set_border_opa(&Style_btn,LV_OPA_50);
	lv_style_set_border_side(&Style_btn,LV_BORDER_SIDE_INTERNAL);
	lv_style_set_text_font(&Style_btn,&lv_font_montserrat_18);
	lv_style_set_text_color(&Style_btn,BUTTON_TEXT_COLOR);

	Lcd_fd = open(LCD_DEVPATH,0);
	if (Lcd_fd<0)
	{
		int errcode = errno;
		LOG_E("lv_button_init: ERR opening lcd - errno(%d)",errcode);
	}
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
 * lv_displays_update
 * -------------------
 */
void lv_displays_update(void)
{
	switch(Display_id ){
	case DISPLAY_TARGET_ID:
		lv_target_update();
		break;
	case DISPLAY_VESSELS_ID:
		break;
	case DISPLAY_CHARTS_ID:
		break;
	default:
		LOG_E("lv_displays_update: ERR Display_id(%d) unknown",Display_id);
		break;
	}
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
			//LOG_D("displays_cmd_event_cb: %s",txt);
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
				//LOG_D("displays_cmd_event_cb: Display_id(%d)",(uint8_t)Display_id);
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
	Target_display_bt_ps = lv_btnmatrix_create(parent);
	lv_btnmatrix_set_map(Target_display_bt_ps,Target_display_bt_psm_map);
	lv_obj_add_event_cb(Target_display_bt_ps,displays_cmd_event_cb,LV_EVENT_PRESSED,NULL);
	lv_obj_set_size(Target_display_bt_ps,DISPLAY_CMD_BTN_WIDTH,DISPLAY_CMD_BTN_HIGH);
	lv_obj_align(Target_display_bt_ps,LV_ALIGN_BOTTOM_LEFT,ALIGN_LEFT,ALIGN_BOTTOM);
	lv_obj_add_style(Target_display_bt_ps,&Style_bg,0);
	lv_obj_add_style(Target_display_bt_ps, &Style_btn,LV_PART_ITEMS);
}




/*
 * ===================
 * target_audio_event_cb
 * -------------------
 *
 */
void target_audio_event_cb(lv_event_t *e)
{
	lv_event_code_t code=lv_event_get_code(e);
	//nxmutex_lock(&Gui_anim_Mutex_s);
	if (code==LV_EVENT_PRESSED)
	{
		if (Gui_animation_s.spk==GUI_ANIM_SPEAKER_ON)
		{
			Gui_animation_s.spk=GUI_ANIM_SPEAKER_OFF;
			lv_canvas_copy_buf(Canvas_head_ps,Img_white_sq_s.data,CANVAS_ICONE_WIDTH*2,0,CANVAS_ICONE_WIDTH,CANVAS_ICONE_HEIGHT);

		}else
		{
			Gui_animation_s.spk=GUI_ANIM_SPEAKER_ON;
			lv_canvas_copy_buf(Canvas_head_ps,Img_spk_on_s.data,CANVAS_ICONE_WIDTH*2,0,CANVAS_ICONE_WIDTH,CANVAS_ICONE_HEIGHT);
		}
		//		LOG_D("target_audio_event_cb: Gui_animation_s.spk(%d)",Gui_animation_s.spk);
		//		LOG_D("target_audio_event_cb: Gui_animation_s.wifi(%d)",Gui_animation_s.wifi);
		//		LOG_D("target_audio_event_cb: Gui_animation_s.gps(%d)",Gui_animation_s.gps);
	}
	lv_canvas_set_buffer(Canvas_head_ps,Canvas_header_au8,CANVAS_HD_WIDTH,CANVAS_HD_HEIGHT,LV_IMG_CF_TRUE_COLOR);
	//nxmutex_unlock(&Gui_anim_Mutex_s);
}



/*
 * ===================
 * lv_audio_cmd
 * -------------------
 *
 */
void lv_audio_cmd(lv_obj_t *parent)
{
	Audio_bt_ps=lv_btnmatrix_create(parent);
	lv_btnmatrix_set_map(Audio_bt_ps,Audio_bt_psm_map);
	lv_obj_add_style(Audio_bt_ps,&Style_bg,0);
	lv_obj_add_style(Audio_bt_ps, &Style_btn,LV_PART_ITEMS);
	lv_obj_add_event_cb(Audio_bt_ps,target_audio_event_cb, LV_EVENT_PRESSED,NULL);
	lv_obj_set_size(Audio_bt_ps,TARGET_AUDIO_BTN_WIDTH,TARGET_AUDIO_BTN_HIGH);
	lv_obj_align(Audio_bt_ps,LV_ALIGN_BOTTOM_RIGHT,ALIGN_RIGHT,ALIGN_BOTTOM);
}

/*
 * ===================
 * target_wifi_event_cb
 * -------------------
 *
 */
void target_wifi_event_cb(lv_event_t *e)
{
	lv_event_code_t code=lv_event_get_code(e);
	if (code==LV_EVENT_PRESSED)
	{
		if (Gui_animation_s.wifi==GUI_ANIM_WIFI_ON)
		{
			Gui_animation_s.wifi=GUI_ANIM_WIFI_OFF;
			lv_canvas_copy_buf(Canvas_head_ps,Img_white_sq_s.data,CANVAS_ICONE_WIDTH,0,CANVAS_ICONE_WIDTH,CANVAS_ICONE_HEIGHT);
		}else
		{
			Gui_animation_s.wifi=GUI_ANIM_WIFI_ON;
			lv_canvas_copy_buf(Canvas_head_ps,Img_wifi_on_s.data,CANVAS_ICONE_WIDTH,0,CANVAS_ICONE_WIDTH,CANVAS_ICONE_HEIGHT);
		}
		//		LOG_D("settings_wifi_event_cb: Gui_animation_s.spk(%d)",Gui_animation_s.spk);
		//		LOG_D("settings_wifi_event_cb: Gui_animation_s.wifi(%d)",Gui_animation_s.wifi);
		//		LOG_D("settings_wifi_event_cb: Gui_animation_s.gps(%d)",Gui_animation_s.gps);
	}
	lv_canvas_set_buffer(Canvas_head_ps,Canvas_header_au8,CANVAS_HD_WIDTH,CANVAS_HD_HEIGHT,LV_IMG_CF_TRUE_COLOR);
}



/*
 * ===================
 * lv_wifi_cmd
 * -------------------
 *
 */
void lv_wifi_cmd(lv_obj_t *parent)
{
	Wifi_bt_ps=lv_btnmatrix_create(parent);
	lv_btnmatrix_set_map(Wifi_bt_ps,Wifi_bt_psm_map);
	lv_obj_add_style(Wifi_bt_ps,&Style_bg,0);
	lv_obj_add_style(Wifi_bt_ps, &Style_btn,LV_PART_ITEMS);
	lv_obj_add_event_cb(Wifi_bt_ps,target_wifi_event_cb, LV_EVENT_PRESSED,NULL);
	lv_obj_set_size(Wifi_bt_ps,TARGET_AUDIO_BTN_WIDTH,TARGET_AUDIO_BTN_HIGH);
	lv_obj_align(Wifi_bt_ps,LV_ALIGN_BOTTOM_RIGHT,ALIGN_RIGHT,ALIGN_BOTTOM-TARGET_AUDIO_BTN_HIGH*2);
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
	Canvas_head_ps=lv_canvas_create(parent);
	memset(Canvas_header_au8,0,CANVAS_BUF_SIZE);
	lv_canvas_set_buffer(Canvas_head_ps,Canvas_header_au8,CANVAS_HD_WIDTH,CANVAS_HD_HEIGHT,LV_IMG_CF_TRUE_COLOR);
	lv_canvas_fill_bg(Canvas_head_ps,lv_color_white(),LV_OPA_COVER);
	lv_obj_align(Canvas_head_ps,LV_ALIGN_TOP_RIGHT,-4,6);

	lv_update_status();
}

/* ===========================
 * lv_update_status
 * ---------------------------
 */
void lv_update_status(void)
{
	//LOG_D("lv_update_status: Gui_animation_s.spk(%d)",Gui_animation_s.spk);
	switch (Gui_animation_s.spk)
	{
	case GUI_ANIM_SPEAKER_ON:
		lv_canvas_copy_buf(Canvas_head_ps,Img_spk_on_s.data,CANVAS_ICONE_WIDTH*2,0,CANVAS_ICONE_WIDTH,CANVAS_ICONE_HEIGHT);
		break;
	default:
		lv_canvas_copy_buf(Canvas_head_ps,Img_white_sq_s.data,CANVAS_ICONE_WIDTH*2,0,CANVAS_ICONE_WIDTH,CANVAS_ICONE_HEIGHT);
		break;
	}

	//LOG_D("lv_update_status: Gui_animation_s.wifi(%d)",Gui_animation_s.wifi);
	switch (Gui_animation_s.wifi)
	{
	case GUI_ANIM_WIFI_ON:
		lv_canvas_copy_buf(Canvas_head_ps,Img_wifi_on_s.data,CANVAS_ICONE_WIDTH,0,CANVAS_ICONE_WIDTH,CANVAS_ICONE_HEIGHT);
		break;
	case GUI_ANIM_WIFI_HS:
		lv_canvas_copy_buf(Canvas_head_ps,Img_wifi_bk_s.data,CANVAS_ICONE_WIDTH,0,CANVAS_ICONE_WIDTH,CANVAS_ICONE_HEIGHT);
		break;
	default:
		lv_canvas_copy_buf(Canvas_head_ps,Img_white_sq_s.data,CANVAS_ICONE_WIDTH,0,CANVAS_ICONE_WIDTH,CANVAS_ICONE_HEIGHT);
		break;
	}

	/*
	 * Display GPS status
	 */
	lv_update_status_gps();

	lv_canvas_set_buffer(Canvas_head_ps,Canvas_header_au8,CANVAS_HD_WIDTH,CANVAS_HD_HEIGHT,LV_IMG_CF_TRUE_COLOR);
}

/* ===========================
 * lv_update_status_gps
 * ---------------------------
 */
void lv_update_status_gps(void)
{
	//LOG_D("lv_update_status: Gui_animation_s.gps(%d)",Gui_animation_s.gps);
	switch (Gui_animation_s.gps)
	{
	case GUI_ANIM_GPS0:
		lv_canvas_copy_buf(Canvas_head_ps,Img_gps0_s.data,0,0,CANVAS_ICONE_WIDTH,CANVAS_ICONE_HEIGHT);
		break;
	case GUI_ANIM_GPS1:
		lv_canvas_copy_buf(Canvas_head_ps,Img_gps1_s.data,0,0,CANVAS_ICONE_WIDTH,CANVAS_ICONE_HEIGHT);
		break;
	case GUI_ANIM_GPS2:
		lv_canvas_copy_buf(Canvas_head_ps,Img_gps2_s.data,0,0,CANVAS_ICONE_WIDTH,CANVAS_ICONE_HEIGHT);
		break;
	case GUI_ANIM_GPS3:
	case GUI_ANIM_GPS4:
	case GUI_ANIM_GPS5:
	case GUI_ANIM_GPS6:
		lv_canvas_copy_buf(Canvas_head_ps,Img_gps3_s.data,0,0,CANVAS_ICONE_WIDTH,CANVAS_ICONE_HEIGHT);
		break;
	default:
		lv_canvas_copy_buf(Canvas_head_ps,Img_gps0_s.data,0,0,CANVAS_ICONE_WIDTH,CANVAS_ICONE_HEIGHT);
		break;
	}
	lv_canvas_set_buffer(Canvas_head_ps,Canvas_header_au8,CANVAS_HD_WIDTH,CANVAS_HD_HEIGHT,LV_IMG_CF_TRUE_COLOR);
}

/*
 * ===================
 * lv_target_cmd
 * -------------------
 *
 */
void lv_target_cmd(lv_obj_t *parent)
{
	Target_bt_ps = lv_btnmatrix_create(parent);
	lv_btnmatrix_set_map(Target_bt_ps,Target_bt_psm_map);
	lv_obj_add_event_cb(Target_bt_ps,target_cmd_event_cb, LV_EVENT_PRESSED,NULL);
	lv_obj_set_size(Target_bt_ps,TARGET_CMD_BTN_WIDTH,TARGET_CMD_BTN_HIGH);
	// From top : 60 - from left: 4
	lv_obj_set_pos(Target_bt_ps,4,60);
	lv_obj_add_style(Target_bt_ps,&Style_bg,0);
	lv_obj_add_style(Target_bt_ps,&Style_btn,LV_PART_ITEMS);
}










/*
 * =========================================================================
 *           Settings
 * ===============================
 */



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
 * light_slider_event_cb
 * -------------------
 *
 */
static void light_slider_event_cb(lv_event_t * e)
{
	lv_obj_t * slider = lv_event_get_target(e);
	Lcd_level_u8=(uint8_t)lv_slider_get_value(slider);
	LOG_D("lcd_get_power: value(%d)",lcd_get_power());
	LOG_D("light_slider_event_cb: value(%d)",Lcd_level_u8);
	lcd_set_power(Lcd_level_u8);
}

/*
 * ===================
 * lv_light_cmd
 * -------------------
 *
 */
#define LIGHT_SLIDER_HEIGHT 120
#define LIGHT_SLIDER_WIDTH 10
#define LIGHT_SLIDER_ALIGNX -70
#define LIGHT_SLIDER_ALIGNY -35

void lv_light_cmd(lv_obj_t *parent)
{
	Light_slider_ps=lv_slider_create(parent);

	lv_slider_set_range(Light_slider_ps,LCD_LEVEL_MIN,LCD_LEVEL_MAX);
	lv_slider_set_mode(Light_slider_ps, LV_SLIDER_MODE_NORMAL);
	lv_obj_set_size(Light_slider_ps, LIGHT_SLIDER_WIDTH, LIGHT_SLIDER_HEIGHT);
	lv_obj_set_style_bg_color(Light_slider_ps, lv_palette_darken(LV_PALETTE_GREY,4),LV_PART_MAIN);
	lv_obj_set_style_bg_color(Light_slider_ps, lv_palette_lighten(LV_PALETTE_GREY,2),LV_PART_KNOB);
	lv_obj_set_style_bg_color(Light_slider_ps,lv_color_hex(DISPLAY_DARKGREY_RGB),LV_PART_INDICATOR);
	lv_obj_add_event_cb(Light_slider_ps,light_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	//Slider_l_ps = lv_label_create(parent);
	//lv_label_set_text(Slider_l_ps, "0%");
	lv_obj_align(Light_slider_ps,LV_ALIGN_BOTTOM_RIGHT,LIGHT_SLIDER_ALIGNX,LIGHT_SLIDER_ALIGNY);
	Lcd_level_u8=lcd_get_power();
	LOG_D("light_slider_event_cb: value(%d)",Lcd_level_u8);
	lv_slider_set_value(Light_slider_ps,Lcd_level_u8,LV_ANIM_OFF);
}

