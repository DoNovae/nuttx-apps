#include <stdio.h>
#include  "display.h"

/*
 * ===================
 * Extern
 * -------------------
 */
extern const lv_img_dsc_t Img_charts_s; //LV_IMG_DECLARE(Img_target_s)

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
lv_obj_t *Charts_display_s;


/**
 * -------------------
 * lv_parent
 * -------------------
 */
void lv_charts_display(lv_obj_t *parent)
{
	Charts_display_s=lv_img_create(parent);
    lv_obj_set_style_bg_color(parent,lv_color_white(),LV_PART_MAIN);
    lv_img_set_src(Charts_display_s,&Img_charts_s);
    lv_obj_align(Charts_display_s,LV_ALIGN_CENTER, 0, 0);
}

void lv_charts_update(void)
{
}




