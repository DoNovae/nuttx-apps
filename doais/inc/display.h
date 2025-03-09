#include <lvgl/lvgl.h>
#include <port/lv_port.h>
#include <port/lv_port_tick.h>
#include <lvgl/src/hal/lv_hal_tick.h>
#include <lvgl/src/misc/lv_color.h>


#ifndef DISPLAY_H
#define DISPLAY_H


/**
 * ==================
 * Defines
 * -----------------
 */
#define DISPLAY_NBR 3
#define ALIGN_LEFT 1
#define ALIGN_RIGHT -1
#define ALIGN_TOP 7
#define ALIGN_BOTTOM -1


/*
 * LV_SYMBOL
 */
#define LV_SYMBOL_DEGREE "\xC2\xB0"
#define LV_SYMBOL_SIGNAL "\xEF\x80\x92"






/*
 * Colors
 */
#define DISPLAY_GREEN_RGB 0x8fc748
#define DISPLAY_GOLD_RGB 0x998579
#define DISPLAY_RED_RGB 0xff0000
#define DISPLAY_DARKGREY_RGB 0x4b4b4b
#define DISPLAY_BLACK_RGB 0x0
#define DISPLAY_WHITE_RGB 0xffffff


/**
 * ==================
 * Types
 * -----------------
 */

typedef enum
{
	DISPLAY_TARGET_ID=0,
	DISPLAY_VESSELS_ID,
	DISPLAY_SETTINGS_ID,
	DISPLAY_CHARTS_ID
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
  lv_update_screen_data_cb_t update_cb;
} lv_updatable_display_t;



/**
 * ==================
 * Prototypes
 * -----------------
 */
void lv_display_init(Display_id_e id_e);

void lv_display_status(lv_obj_t *parent);
void lv_display_init(Display_id_e id_e);

void lv_target_display(lv_obj_t * parent);
void lv_target_update(void);

void lv_vessels_display(lv_obj_t *parent);
void lv_vessels_update(void);

void lv_settings_display(lv_obj_t *parent);
void lv_settings_update(void);

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
LV_FONT_DECLARE(lv_font_doais_12);
extern lv_updatable_display_t Displays_as[DISPLAY_NBR];
extern Display_id_e Display_id;



#endif //DISPLAY_H
