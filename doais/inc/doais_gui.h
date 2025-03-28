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
#include <lvgl/src/misc/lv_color.h>
#include "ais.h"
#include "display.h"

#ifndef DOIAS_GUI_H
#define DOIAS_GUI_H


/**
 * ==================
 * Defines
 * -----------------
 */


/**
 * ================================================
 * Prototypes
 */
void lv_displays_update();
void lv_display_init();




#endif //DOIAS_GUI_H
