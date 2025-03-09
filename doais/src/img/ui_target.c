#include <stdio.h>
#include  "display.h"
#include  "types.h"


/*
 * ===================
 * Defines
 * -------------------
 */
#define SPEED_HEADING_LABEL_WITH 230
#define ALIGN_LEFT 1
#define ALIGN_TOP 7
#define ALIGN_BOTTOM -1
#define DISPLAY_CMD_BTN_WITH 148
#define DISPLAY_CMD_BTN_HIGH 25

/*
 * ===================
 * Extern
 * -------------------
 */
extern const lv_img_dsc_t Img_target_s; //LV_IMG_DECLARE(Img_target_s)


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
lv_obj_t *Target_display_s;
lv_obj_t *Speed_Heading_l;
lv_obj_t *Target_btn;



const char *Target_btnm_map[] =
{
		"TGT",LV_SYMBOL_LEFT, LV_SYMBOL_RIGHT,""
};



/*
 * -------------------
 * lv_display_cmd
 * -------------------
 */
void lv_display_cmd(lv_obj_t *parent)
{
static lv_style_t style_bg;
    lv_style_init(&style_bg);
    lv_style_set_pad_all(&style_bg, 0);
    lv_style_set_pad_gap(&style_bg, 0);
    lv_style_set_clip_corner(&style_bg, true);
    lv_style_set_radius(&style_bg, LV_RADIUS_CIRCLE);
    lv_style_set_border_width(&style_bg, 0);
    lv_style_set_border_width(&style_bg, 0);
    lv_style_set_border_color(&style_bg,lv_color_hex(DISPLAY_GOLD_RGB));

    static lv_style_t style_btn;
    lv_style_init(&style_btn);
    lv_style_set_radius(&style_btn, 0);
    lv_style_set_border_width(&style_btn, 1);
    lv_style_set_border_opa(&style_btn, LV_OPA_50);
    lv_style_set_border_color(&style_btn,lv_color_hex(DISPLAY_GOLD_RGB));
    lv_style_set_border_side(&style_btn, LV_BORDER_SIDE_INTERNAL);
    lv_style_set_radius(&style_btn, 0);

Target_btn = lv_btnmatrix_create(parent);
lv_btnmatrix_set_map(Target_btn, Target_btnm_map);
lv_obj_add_style(Target_btn, &style_bg, 0);
lv_obj_add_event_cb(Target_btn,displays_cmd_event_cb, LV_EVENT_PRESSED, NULL);
lv_obj_set_size(Target_btn,DISPLAY_CMD_BTN_WITH,DISPLAY_CMD_BTN_HIGH);
lv_obj_align(Target_btn,LV_ALIGN_BOTTOM_LEFT,ALIGN_LEFT,ALIGN_BOTTOM);
lv_obj_add_style(Target_btn, &style_btn, LV_PART_ITEMS);
}




/**
 * -------------------
 * lv_parent
 * -------------------
 */
void lv_target_display(lv_obj_t *parent)
{
	Target_display_s=lv_img_create(parent);
	lv_obj_set_style_bg_color(parent,lv_color_hex(DISPLAY_GOLD_RGB),LV_PART_MAIN);
	lv_img_set_src(Target_display_s,&Img_target_s);
	lv_obj_align(Target_display_s,LV_ALIGN_CENTER, 0, 0);

	Speed_Heading_l = lv_label_create(parent);
	lv_obj_set_style_text_font(Speed_Heading_l,&lv_font_montserrat_30,0);
	//lv_obj_set_style_text_font(Speed_Heading_l,&lv_font_montserrat_30b,0);
	//lv_obj_set_style_text_font(Speed_Heading_l,&LiberationMono_Bold30pt,0);
	lv_obj_set_style_text_color(Speed_Heading_l,lv_color_hex(DISPLAY_WHITE_RGB),0);

	//lv_label_set_text(Speed_Heading_l,LV_SYMBOL_SIGNAL);
	lv_label_set_text(Speed_Heading_l,"6.7kt270"LV_SYMBOL_DEGREE);

	//lv_label_set_text_fmt(Speed_Heading_l,"%04.1fkt%03d" LV_SYMBOL_DEGREE LV_SYMBOL_SIGNAL,(float)6.7,270);

	//lv_obj_set_style_text_font(Speed_Heading_l,&Signal30pt,0);
	//lv_label_set_text(Speed_Heading_l,LV_SYMBOL_SIGNAL);

	lv_obj_set_width(Speed_Heading_l,SPEED_HEADING_LABEL_WITH);
	lv_obj_set_style_text_align(Speed_Heading_l, LV_TEXT_ALIGN_LEFT, 0);
	lv_obj_align(Speed_Heading_l, LV_ALIGN_TOP_LEFT,ALIGN_LEFT,ALIGN_TOP);
	// elsewhere: lv_obj_set_pos(Speed_Heading_l, LV_ALIGN_TOP_LEFT,ALIGN_LEFT,ALIGN_TOP);

	lv_display_cmd(parent);
}

/*
 * -------------------
 * lv_target_update
 * -------------------
 */
void lv_target_update(void)
{
}

/*
 * -------------------
 * autopilot_event_cb
 * -------------------
 */
void displays_cmd_event_cb(lv_event_t *e)
{
	LOG_D("displays_cmd_event_cb");
}


