/**
 * =====================================
 * doais_gui.h
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
//#include <lvgl/src/hal/lv_hal_tick.h>
#include <lvgl/src/misc/lv_color.h>
#include "ais.h"


#ifndef DOIAS_GUI_H
#define DOIAS_GUI_H


/**
 * ==================
 * Defines
 * -----------------
 */
#define DISPLAY_GREEN 0x298E
#define DISPLAY_GOLD 0x2F9C
#define DISPLAY_NAVY 0x1F00
#define DISPLAY_RED 0x00F8
#define DISPLAY_DARKGREY 0x494A



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
  lv_update_screen_data_cb_t update_cb;
} lv_updatable_display_t;









/**
 * ================================================
 * Prototypes
 */
void lv_displays_update();
void lv_display_init();


/**
 * ==================
 * Extern
 * -----------------
 */
extern lv_updatable_display_t Displays_as[DISPLAY_NBR];
extern Display_id_e Display_id;



#endif //DOIAS_GUI_H
