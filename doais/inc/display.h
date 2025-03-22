/**
 * =====================================
 * display.h
 * -------------------------------------
 *  AIS DoNovae
 *  www.DoNovae.com
 *  Herve Bailly
 * =====================================
 *
 */
#include <lvgl/lvgl.h>
#include <port/lv_port.h>
#include <port/lv_port_tick.h>
#include <lvgl/src/misc/lv_color.h>



#ifndef DISPLAY_H
#define DISPLAY_H


/**
 * ==================
 * Defines
 * -----------------
 */
#define ALIGN_LEFT 1
#define ALIGN_RIGHT -1
#define ALIGN_TOP 7
#define ALIGN_BOTTOM -1



/*
 * -----------------
 * LV_SYMBOL
 * -----------------
 */
#define LV_SYMBOL_DEGREE "\xC2\xB0"

#define DISPLAY_TARGET_R1 37
#define DISPLAY_TARGET_R2 75
#define DISPLAY_TARGET_R3 112
#define DISPLAY_TARGET_R32 56
#define MONITORING_DISPLAY_STEPS_NB 3
#define MONITORING_DISPLAY_SPEED_MIN_KT 0



/*
 * -----------------
 * Colors
 * lv_palette_darken(LV_PALETTE_BLUE, 2)
 * lv_palette_lighten(LV_PALETTE_GREY,2)
 * lv_palette_main(LV_PALETTE_GREY)
 * lv_color_white()
 * lv_color_black()
 * lv_color_hex(DISPLAY_WHITE_RGB)
 * -----------------
 */
#define DISPLAY_GREEN_RGB 0x8fc748
#define DISPLAY_GOLD_RGB 0x998579
#define DISPLAY_RED_RGB 0xff0000
#define DISPLAY_DARKGREY_RGB 0x4b4b4b
#define DISPLAY_BLACK_RGB 0x0
#define DISPLAY_WHITE_RGB 0xffffff


/**
 * ================================================
 * Types
 */
#define DISPLAY_NBR 3

typedef enum
{
	DISPLAY_TARGET_ID=0,
	DISPLAY_VESSELS_ID=1,
	DISPLAY_SETTINGS_ID=2,
	DISPLAY_CHARTS_ID=3
} Display_id_e;

typedef struct
{
	uint8_t Hours;
	uint8_t Minutes;
	uint8_t Seconds;
} RTC_TimeTypeDef;


typedef struct
{
	uint8_t WeekDay;
	uint8_t Month;
	uint8_t Date;
	uint16_t Year;
} RTC_DateTypeDef;


/*
 * Displays
 */
typedef void (*lv_update_screen_data_cb_t)(void);
typedef void (*lv_init_screen_cb_t)(lv_obj_t* parent);

typedef struct
{
	lv_obj_t* display_ps;
	lv_init_screen_cb_t init_cb;
} lv_updatable_display_t;



/*
 * ----------------
 * point_t
 * ----------------
 */
typedef struct
{
	uint16_t x;
	uint16_t y;
} gui_point_t;

typedef struct
{
	gui_point_t p0;
	gui_point_t p1;
} gui_touch_area_t;


/*
 * ----------------
 * Target
 * ----------------
 */
typedef enum
{
	GUI_TGT_NONE=-1,
	GUI_TGT_VSL=0,
	GUI_TGT_SET=1,
	GUI_TGT_SPK=2,
	GUI_TGT_CROSS=3,
	GUI_TGT_ZIN=4,
	GUI_TGT_ZOUT=5
} gui_target_touch_e;



/*
 * ----------------
 * VESSELS
 * ----------------
 */
typedef enum
{
	GUI_VSL_NONE=-1,
	GUI_VSL_TGT=0,
	GUI_VSL_SET=1,
	GUI_VSL_SPK=2
} gui_vessels_touch_e;


/*
 * ----------------
 * SETTINGS
 * ----------------
 */
typedef enum
{
	GUI_SET_NONE=-1,
	GUI_SET_TGT=0,
	GUI_SET_VSL=1,
	GUI_SET_SPK=2,
	GUI_SET_WIFI=3
} gui_settings_touch_e;


typedef union
{
	gui_target_touch_e tgt;
	gui_vessels_touch_e vsl;
	gui_settings_touch_e set;
	int8_t i8;
} gui_touch_id_t;

typedef enum
{
	GUI_TOUCH_NONE=0,
	GUI_JUST_TOUCH=1,
	GUI_TOUCH_HOLD=2
} gui_touch_e;


/*
 * -----------------------
 * State machine
 * -----------------------
 */
typedef enum
{
	GUI_SM_START=0,
	GUI_SM_TARGET=1,
	GUI_SM_VESSELS=2,
	GUI_SM_SETTINGS=3
} gui_state_machine_e;

/*
 * -----------------------
 * Wifi animation
 * -----------------------
 */
typedef enum
{
	GUI_ANIM_WIFI_OFF=0,
	GUI_ANIM_WIFI_ON=1,
	GUI_ANIM_WIFI_HS=2
} gui_animation_wifi_e;
#define GUI_ANIMATION_WIFI_NB 3

/*
 * -----------------------
 * Speaker animation
 * -----------------------
 */
typedef enum
{
	GUI_ANIM_SPEAKER_OFF=0,
	GUI_ANIM_SPEAKER_ON=1
} gui_animation_spk_e;
#define GUI_ANIMATION_SPK_NB 2

/*
 * -----------------------
 * Bell animation
 * -----------------------
 */
typedef enum
{
	GUI_ANIM_BELL_ON=0,
	GUI_ANIM_BELL_OFF=1
} gui_animation_bell_e;
#define GUI_ANIMATION_BELL_NB 2

/*
 * -----------------------
 * GPS animation
 * -----------------------
 */
typedef enum
{
	GUI_ANIM_GPS0=0,
	GUI_ANIM_GPS1=1,
	GUI_ANIM_GPS2=2,
	GUI_ANIM_GPS3=3,
	GUI_ANIM_GPS4=4,
	GUI_ANIM_GPS5=5,
	GUI_ANIM_GPS6=6
} gui_animation_gps_e;
#define GUI_ANIMATION_GPS_NB 4


/*
 * -----------------------
 * Animation union
 * -----------------------
 */
typedef struct
{
	gui_animation_wifi_e wifi;
	gui_animation_spk_e spk;
	gui_animation_bell_e bell;
	gui_animation_gps_e gps;
} gui_animation_t;


/*
 * -----------------------
 * Vessels
 * -----------------------
 */
typedef enum
{
	VESSELS_CANNVAS_N0NE=0,
	VESSELS_CANVAS1=1,
	VESSELS_CANVAS2=2
} vessels_canvas_e;


/*
 * ===================
 * Globals
 * -------------------
 */



/**
 * ================================================
 * Prototypes
 */

void lv_button_init();
void lv_display_cmd(lv_obj_t *parent);
void lv_audio_cmd(lv_obj_t *parent);
void lv_update();

void lv_display_status(lv_obj_t *parent);

void lv_target_display(lv_obj_t * parent);
void lv_update_status_gps(void);

void lv_vessels_display(lv_obj_t *parent);

void lv_settings_display(lv_obj_t *parent);
void lv_settings_update_pos(int32_t lon_d,int32_t lat_d);
void lv_settings_update_time(int32_t hour_i32, int32_t mn_i32,int32_t s_i32);

void lv_update_speed(float speed_f, uint16_t heading_u16);
void lv_wifi_cmd(lv_obj_t *parent);
void lv_target_cmd(lv_obj_t *parent);
void lv_target_update(float max_nm_d32s);
void lv_update_status(void);

/*void lv_wind_display(lv_obj_t * parent);
void wind_update_cb(void);

void autopilot_update_cb(void);
void lv_autopilot_display(lv_obj_t *parent);*/

/**
 * ==================
 * Extern
 * -----------------
 */
LV_FONT_DECLARE(lv_font_doais_speed);
LV_FONT_DECLARE(lv_font_doais_status);
LV_FONT_DECLARE(lv_font_doais_24);
LV_FONT_DECLARE(lv_font_doais_20);
LV_FONT_DECLARE(lv_font_doais_12);

extern lv_updatable_display_t Displays_as[DISPLAY_NBR];
extern Display_id_e Display_id;

#endif //DISPLAY_H
