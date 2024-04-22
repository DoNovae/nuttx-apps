#include <lvgl/lvgl.h>
#include <port/lv_port.h>
#include <port/lv_port_tick.h>
#include <lvgl/src/hal/lv_hal_tick.h>
#include <lvgl/src/misc/lv_color.h>

#ifndef UI_CLOCK_H
#define UI_CLOCK_H

#define LV_HOR_RES_MAX 320
#define LV_VER_RES_MAX 240

  typedef void (*lv_update_screen_data_cb_t)(void);
  typedef void (*lv_init_screen_cb_t)(lv_obj_t* obj);

  typedef struct _lv_updatable_screen_t {
    lv_obj_t* screen;
    lv_update_screen_data_cb_t update_cb;
    lv_init_screen_cb_t init_cb;
  } lv_updatable_screen_t;


  typedef struct
  {
    uint8_t Hours;
    uint8_t Minutes;
    uint8_t Seconds;
  }RTC_TimeTypeDef;


  typedef struct
  {
    uint8_t WeekDay;
    uint8_t Month;
    uint8_t Date;
    uint16_t Year;
  }RTC_DateTypeDef;

  /**
   * A clock display 
   */
  void lv_clock_display(lv_obj_t *parent);
   void clock_update_cb(void);

  extern lv_obj_t *clock_display;


#endif //UI_CLOCK_H
