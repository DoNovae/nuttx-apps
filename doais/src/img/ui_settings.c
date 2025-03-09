#include <stdio.h>
#include  "display.h"

/*
 * ===================
 * Extern
 * -------------------
 */
extern const lv_img_dsc_t Img_settings_s; //LV_IMG_DECLARE(Img_settings_s)

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
lv_obj_t *Settings_display_s;


/**
 * ===================
 * lv_settings_display
 * -------------------
 */
void lv_settings_display(lv_obj_t *parent)
{
	Settings_display_s=lv_img_create(parent);
    lv_img_set_src(Settings_display_s,&Img_settings_s);
}


/**
 * ===================
 * lv_settings_update_cb
 * -------------------
 */
void lv_settings_update_cb()
{
}


