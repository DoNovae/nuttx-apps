#include <stdio.h>
#include  "display.h"

/*
 * ===================
 * Extern
 * -------------------
 */
LV_IMG_DECLARE(Img_vessels_s);

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
static lv_obj_t *Vessels_display_s;


/**
 * -------------------
 * lv_vessels_display
 * -------------------
 */
void lv_vessels_display(lv_obj_t *parent)
{
	Vessels_display_s=lv_img_create(parent);
    lv_img_set_src(Vessels_display_s,&Img_vessels_s);

    lv_display_cmd(parent);
    lv_display_status(parent);
	lv_audio_cmd(parent);
}


/*
 * ===================
 * lv_vessels_update
 * -------------------
 *
 */
void lv_vessels_update(void)
{
}




