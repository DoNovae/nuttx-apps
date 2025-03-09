#include <stdio.h>
#include  "display.h"

/*
 * ===================
 * Extern
 * -------------------
 */
extern const lv_img_dsc_t Img_vessels_s; //LV_IMG_DECLARE(Img_target_s)

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
lv_obj_t *Vessels_display_s;


/**
 * -------------------
 * lv_parent
 * -------------------
 */
void lv_vessels_display(lv_obj_t *parent)
{
	Vessels_display_s=lv_img_create(parent);
    lv_img_set_src(Vessels_display_s,&Img_vessels_s);
}





