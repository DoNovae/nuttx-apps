/**
 * =====================================
 * ui_target.c
 * -------------------------------------
 *  AIS DoNovae
 *  www.DoNovae.com
 *  Herve Bailly
 * =====================================
 *
 */

#include <stdio.h>
#include <lvgl/src/misc/lv_anim.h>
#include <lvgl/src/misc/lv_types.h>
#include <lvgl/src/draw/lv_draw.h>
#include <lvgl/src/draw/lv_draw_rect.h>
#include <lvgl/src/draw/lv_draw_arc.h>
#include <lvgl/src/draw/lv_draw_label.h>
#include <lvgl/src/core/lv_obj_draw.h>


#include  "display.h"
#include  "types.h"

/*
 * ===================
 * Defines
 * -------------------
 */
#define TARGET_CMD_BTN_WIDTH 99
#define TARGET_CMD_BTN_HIGH 48

#define SPEED_STR_LEN 16
#define TARGET_SCALE_STR_LEN 8
#define SPEED_HEADING_LABEL_WITH 230

#define TARGET_CANVAS_WIDTH  224
#define TARGET_CANVAS_HEIGHT 224
#define CANVAS_BUF_SIZE (TARGET_CANVAS_WIDTH*TARGET_CANVAS_HEIGHT*LV_IMG_PX_SIZE_ALPHA_BYTE)



/*
 * ===================
 * Extern
 * -------------------
 */
LV_IMG_DECLARE(Img_target_s);
LV_IMG_DECLARE(Img_bell_s);
LV_IMG_DECLARE(Target_canavas_s);
extern lv_style_t Style_btn, Style_bg;// cf ui_settings.c

/*
 * ===================
 * Globals
 * -------------------
 */
static lv_obj_t *Target_display_ps,*Bell_display_ps;
static lv_obj_t *Target_bt_ps;
static char Speed_str[SPEED_STR_LEN];
static char Target_scale_str[TARGET_SCALE_STR_LEN];
static lv_obj_t *Speed_Heading_ps, *Target_canvas_ps;

static uint8_t Target_canavas_au8[CANVAS_BUF_SIZE];

static const char *Target_bt_psm_map[]=
{
		"+","-","\n",
		" ","X",""
};

/*
 * ===================
 * Prototypes
 * -------------------
 */
void lv_display_target(lv_obj_t *parent);
void lv_target_bell(lv_obj_t *parent);


/*
 * ================================================================
 *           TARGET FUNCTIONS
 */

/**
 * ===================
 * lv_display_speed
 * -------------------
 *
 */
void lv_display_speed(lv_obj_t *parent)
{
	Speed_Heading_ps = lv_label_create(parent);
	lv_obj_set_style_text_font(Speed_Heading_ps,&lv_font_doais_speed,0);
	lv_obj_set_style_text_color(Speed_Heading_ps,lv_color_hex(DISPLAY_WHITE_RGB),0);
	lv_obj_set_width(Speed_Heading_ps,SPEED_HEADING_LABEL_WITH);
	lv_obj_set_style_text_align(Speed_Heading_ps, LV_TEXT_ALIGN_LEFT, 0);
	lv_obj_align(Speed_Heading_ps, LV_ALIGN_TOP_LEFT,0,4);
	lv_update_speed((float)0.0,0);
}

/*
 * ===================
 * lv_update_speed
 * -------------------
 */
void lv_update_speed(float speed_f, uint16_t heading_u16)
{
	uint16_t spd_u16;

	spd_u16=(uint16_t)(speed_f*10+(float)0.5);
	memset((void*)Speed_str,0,SPEED_STR_LEN);
	sprintf(Speed_str,"%03dk%03d" LV_SYMBOL_DEGREE,spd_u16,heading_u16);
	lv_label_set_text_static(Speed_Heading_ps,Speed_str);
}

/**
 * ===================
 * lv_target_display
 * -------------------
 *
 */
void lv_target_display(lv_obj_t *parent)
{
	/*
	 * Display bkgd image
	 */
	Target_display_ps=lv_img_create(parent);
	lv_img_set_src(Target_display_ps,&Img_target_s);
	lv_obj_align(Target_display_ps,LV_ALIGN_CENTER, 0, 0);

	lv_display_speed(parent);
	lv_display_speed(parent);

	lv_display_cmd(parent);
	lv_target_cmd(parent);
	lv_audio_cmd(parent);
	lv_target_bell(parent);
	lv_display_status(parent);

	lv_display_target(parent);
}



/*
 * ===================
 * lv_target_bell
 * -------------------
 *
 */
#define BELL_CORNER_RL_POSX 23
#define BELL_CORNER_RL_POSY 129
#define BELL_CORNER_WIDTH 50
#define BELL_CORNER_HEIGHT 50
#define BELL_ANIM_DURATION_MS 250

/*
 * Set the zoom factor of the image.
 * Zoom factor.
 *    - 256 or LV_ZOOM_IMG_NONE for no zoom
 *    - <256: scale down
 *    - >256 scale up
 *    - 128 half size
 *    - 512 double size
 */
static void set_zoom(void * img, int32_t v)
{
	lv_img_set_zoom(img, v);
}


/*
 * ===================
 * lv_target_bell
 * -------------------
 *
 */
void lv_target_bell(lv_obj_t *parent)
{
	lv_anim_t a;
	Bell_display_ps=lv_img_create(parent);
	lv_img_set_src(Bell_display_ps,&Img_bell_s);
	lv_obj_set_pos(Bell_display_ps,BELL_CORNER_RL_POSX,BELL_CORNER_RL_POSY);
	lv_img_set_size_mode(Bell_display_ps,LV_IMG_SIZE_MODE_VIRTUAL);

	/*
	 * Animation
	 */
	lv_anim_init(&a);
	lv_anim_set_var(&a,Bell_display_ps);
	lv_anim_set_exec_cb(&a, set_zoom);
	lv_anim_set_values(&a,256,300);
	lv_anim_set_time(&a,BELL_ANIM_DURATION_MS);
	lv_anim_set_repeat_count(&a,LV_ANIM_REPEAT_INFINITE);

	lv_anim_start(&a);
	//lv_obj_add_flag(Bell_display_ps,LV_OBJ_FLAG_HIDDEN);
}


/*
 * ===================
 * target_cmd_event_cb
 * -------------------
 *
 */
void target_cmd_event_cb(lv_event_t *e)
{
	lv_obj_t *obj = lv_event_get_target(e);
	lv_event_code_t code = lv_event_get_code(e);

	LOG_D("target_cmd_event_cb: code(%d)",code);

	// Prevent LVGL sending further input-device-related events
	lv_indev_wait_release(lv_indev_get_act());

	if (code==LV_EVENT_PRESSED)
	{
		uint32_t id = lv_btnmatrix_get_selected_btn(obj);
		const char *txt = lv_btnmatrix_get_btn_text(obj, id);

		if (txt != NULL)
		{
			LOG_D("target_cmd_event_cb: %s",txt);
			if (strcmp("+",txt)==0)
			{
			} else if (strcmp("-",txt)==0)
			{
			} else if (strcmp("X",txt) == 0)
			{
			} else
			{
				LOG_W("target_cmd_event_cb: wrong cmd(%s)",txt);
			}
		}
	}
}


/* ===================
 * lv_target_cmd
 * -------------------
 *
 */
void lv_target_cmd(lv_obj_t *parent)
{
	Target_bt_ps = lv_btnmatrix_create(parent);
	lv_btnmatrix_set_map(Target_bt_ps,Target_bt_psm_map);
	lv_obj_add_event_cb(Target_bt_ps,target_cmd_event_cb, LV_EVENT_PRESSED,NULL);
	lv_obj_set_size(Target_bt_ps,TARGET_CMD_BTN_WIDTH,TARGET_CMD_BTN_HIGH);
	// From top : 60 - from left: 4
	lv_obj_set_pos(Target_bt_ps,4,60);
	lv_obj_add_style(Target_bt_ps,&Style_bg,0);
	lv_obj_add_style(Target_bt_ps,&Style_btn,LV_PART_ITEMS);
}


/*
 * ===================
 * lv_display_target
 * -------------------
 *
 */
#define TARGET_CORNER_RL_POSX 96
#define TARGET_CORNER_RL_POSY 16
void lv_display_target(lv_obj_t *parent)
{
	Target_canvas_ps=lv_canvas_create(parent);
	memset(Target_canavas_au8,0,CANVAS_BUF_SIZE);
	lv_canvas_set_buffer(Target_canvas_ps,Target_canavas_au8,TARGET_CANVAS_WIDTH,TARGET_CANVAS_HEIGHT,LV_IMG_CF_TRUE_COLOR_ALPHA);

	//lv_canvas_fill_bg(Target_canvas_ps,lv_color_white(),LV_OPA_COVER);
	lv_obj_set_pos(Target_canvas_ps,TARGET_CORNER_RL_POSX,TARGET_CORNER_RL_POSY);

	lv_update_status();
}


/*
 * ===================
 * lv_update_target
 * -------------------
 *
 */
//void lv_update_target(void)
//{
//	/*
//	 * Edit Target_canavas_au8
//	 */
//	lv_canvas_copy_buf(Target_canvas_ps,Target_canavas_s.data,0,0,TARGET_CANVAS_WIDTH,TARGET_CANVAS_HEIGHT);
//
//	/*
//	 * Display target
//	 */
//	lv_canvas_set_buffer(Target_canvas_ps,Target_canavas_au8,TARGET_CANVAS_WIDTH,TARGET_CANVAS_HEIGHT,LV_IMG_CF_TRUE_COLOR_ALPHA);
//}



/*
 * ==========================
 * lv_update_target
 * --------------------------
 *    Cf void Ais_monitoring::display_target_ais()
 * --------------------------
 */

#define SCALE_POSX 196
#define SCALE_POSY 25
#define SCALE_WIDTH 33
#define ARC_CENTER_POSX 112
#define ARC_CENTER_POSY 112

void lv_target_update(float max_nm_d32)
{
	float alert_circ_d32;

	/*
	 * Edit Target_canavas_au8
	 */
	lv_canvas_copy_buf(Target_canvas_ps,Target_canavas_s.data,0,0,TARGET_CANVAS_WIDTH,TARGET_CANVAS_HEIGHT);


	/*
	 * Circles
	 */
	lv_draw_arc_dsc_t arc_dsc;
	lv_draw_arc_dsc_init(&arc_dsc);
	arc_dsc.color=lv_color_black();
	arc_dsc.width=1;
	lv_canvas_draw_arc(Target_canvas_ps,ARC_CENTER_POSX,ARC_CENTER_POSY,DISPLAY_TARGET_R2,0,360,&arc_dsc);
	lv_canvas_draw_arc(Target_canvas_ps,ARC_CENTER_POSX,ARC_CENTER_POSY,DISPLAY_TARGET_R1,0,360,&arc_dsc);

		//scale_px_nm_d32=(float)DISPLAY_TARGET_R3/Monitoring.settings_s.display_target_step_nm_u32/(float)MONITORING_DISPLAY_STEPS_NB;
	//	Ais_display::target.drawCircle(DISPLAY_TARGET_CENTER_X,DISPLAY_TARGET_CENTER_Y,DISPLAY_TARGET_R3,TFT_BLACK);
	//	max_nm_d32=(float)settings_s.display_target_step_nm_u32*(float)MONITORING_DISPLAY_STEPS_NB;
	//	alert_circ_d32=(float)DISPLAY_TARGET_R3*(float)settings_s.cpa_warn_10thnm_u32/(float)10.0/max_nm_d32;
	//	switch (zoom_s){
	//	case AIS_MONITORING_Z0:
	//		Ais_display::target.drawCircle(DISPLAY_TARGET_CENTER_X,DISPLAY_TARGET_CENTER_Y,DISPLAY_TARGET_R2,TFT_BLACK);
	//		Ais_display::target.drawCircle(DISPLAY_TARGET_CENTER_X,DISPLAY_TARGET_CENTER_Y,DISPLAY_TARGET_R1,TFT_BLACK);
	//		break;
	//	case AIS_MONITORING_Z1:
	//		Ais_display::target.drawCircle(DISPLAY_TARGET_CENTER_X,DISPLAY_TARGET_CENTER_Y,DISPLAY_TARGET_R32,TFT_BLACK);
	//		scale_px_nm_d32*=(float)3/2;
	//		max_nm_d32=(float)max_nm_d32*(float)2.0/(float)3.0;
	//		alert_circ_d32=alert_circ_d32*(float)3.0/(float)2.0;
	//		break;
	//	case AIS_MONITORING_Z2:
	//		scale_px_nm_d32*=(float)3;
	//		max_nm_d32=(float)max_nm_d32/(float)3;
	//		alert_circ_d32=alert_circ_d32*(float)3.0;
	//		break;
	//	}
	//	Ais_display::target.drawCircle(DISPLAY_TARGET_CENTER_X,DISPLAY_TARGET_CENTER_Y,(int16_t)(alert_circ_d32+(float)0.5),DISPLAY_RED);
	//
	//	/*
	//	 * Lines
	//	 */
	//	Ais_display::target.drawLine(0,DISPLAY_TARGET_CENTER_Y,DISPLAY_TARGET_SX,DISPLAY_TARGET_CENTER_Y,TFT_BLACK);
	//	Ais_display::target.drawLine(DISPLAY_TARGET_CENTER_X,0,DISPLAY_TARGET_CENTER_X,DISPLAY_TARGET_SY,TFT_BLACK);
	//
	//	/*
	//	 * Cross
	//	 */
	//	if (is_cross_b) {
	//		Ais_display::target.drawLine(DISPLAY_TARGET_CENTER_X-10,DISPLAY_TARGET_CENTER_Y-10,DISPLAY_TARGET_CENTER_X+10,DISPLAY_TARGET_CENTER_Y+10,DISPLAY_RED);
	//		Ais_display::target.drawLine(DISPLAY_TARGET_CENTER_X-10,DISPLAY_TARGET_CENTER_Y+10,DISPLAY_TARGET_CENTER_X+10,DISPLAY_TARGET_CENTER_Y-10,DISPLAY_RED);
	//	}
	//
	//	/*
	//	 * Display filtered vessels
	//	 */
	//	display_target_ais_filtering(max_nm_d32,scale_px_nm_d32,MONITORING_STATUS_NONE);
	//	display_target_ais_filtering(max_nm_d32,scale_px_nm_d32,MONITORING_STATUS_ALERT);
	//
	//	// Scale in NM
	//	Ais_display::draw_target_scale((int16_t)(max_nm_d32+(float)0.5));
	//	M5.Lcd.pushImage(DISPLAY_TARGET_POSX,DISPLAY_TARGET_POSY,DISPLAY_TARGET_SX,DISPLAY_TARGET_SY,(m5gfx::rgb565_t*)Ais_display::target.getBuffer());
	    lv_draw_label_dsc_t scale_dsc;
	    lv_draw_label_dsc_init(&scale_dsc);
	    scale_dsc.color=lv_color_white();
	    scale_dsc.font=&lv_font_doais_20;

		memset((void*)Target_scale_str,0,TARGET_SCALE_STR_LEN);
		sprintf(Target_scale_str,"%1dN",(int16_t)(max_nm_d32+(float)0.5));
	    lv_canvas_draw_text(Target_canvas_ps,SCALE_POSX,SCALE_POSY,SCALE_WIDTH,&scale_dsc,Target_scale_str);


	/*
	 * Display target
	 */
	lv_canvas_set_buffer(Target_canvas_ps,Target_canavas_au8,TARGET_CANVAS_WIDTH,TARGET_CANVAS_HEIGHT,LV_IMG_CF_TRUE_COLOR_ALPHA);
}


/*
 * ======================
 * draw_circle
 * ----------------------
 */
void draw_circle(int32_t x, int32_t y, int32_t r)
{
//  if ( r <= 0 ) {
//    drawPixel(x, y);
//    return;
//  }
//
//  startWrite();
//  int32_t f = 1 - r;
//  int32_t ddF_y = - (r << 1);
//  int32_t ddF_x = 1;
//  int32_t i = 0;
//  int32_t j = -1;
//  do {
//    while (f < 0) {
//      ++i;
//      f += (ddF_x += 2);
//    }
//    f += (ddF_y += 2);
//
//    writeFastHLine(x - i    , y + r, i - j);
//    writeFastHLine(x - i    , y - r, i - j);
//    writeFastHLine(x + j + 1, y - r, i - j);
//    writeFastHLine(x + j + 1, y + r, i - j);
//
//    writeFastVLine(x + r, y + j + 1, i - j);
//    writeFastVLine(x + r, y - i    , i - j);
//    writeFastVLine(x - r, y - i    , i - j);
//    writeFastVLine(x - r, y + j + 1, i - j);
//    j = i;
//  } while (i < --r);
//  endWrite();
}




//#include "../../hal/lv_hal_disp.h"
#include <lvgl/src/misc/lv_math.h>
#include <lvgl/src/misc/lv_assert.h>
#include <lvgl/src/misc/lv_area.h>
#include <lvgl/src/misc/lv_style.h>
#include <lvgl/src/misc/lv_style.h>
#include <lvgl/src/core/lv_refr.h>
#include <lvgl/src/font/lv_font.h>




static void LV_ATTRIBUTE_FAST_MEM draw_letter_normal(lv_draw_ctx_t * draw_ctx, const lv_draw_label_dsc_t * dsc,const lv_point_t * pos, lv_font_glyph_dsc_t * g, const uint8_t * map_p)
{
    const uint8_t * bpp_opa_table_p;
    uint32_t bitmask_init;
    uint32_t bitmask;
    uint32_t bpp = g->bpp;
    lv_opa_t opa = dsc->opa;
    uint32_t shades;
    if(bpp == 3) bpp = 4;

//LV_LOG_WARN("HBL");

    if(bpp == LV_IMGFONT_BPP)
    { //is imgfont
        lv_area_t fill_area;
        fill_area.x1 = pos->x;
        fill_area.y1 = pos->y;
        fill_area.x2 = pos->x + g->box_w - 1;
        fill_area.y2 = pos->y + g->box_h - 1;
        lv_draw_img_dsc_t img_dsc;
        lv_draw_img_dsc_init(&img_dsc);
        img_dsc.angle = 45;
        img_dsc.zoom = LV_IMG_ZOOM_NONE;
        img_dsc.opa = dsc->opa;
        img_dsc.blend_mode = dsc->blend_mode;
        //lv_draw_img(draw_ctx, &img_dsc, &fill_area, map_p);
        lv_canvas_draw_img(Target_canvas_ps, pos->x, pos->y, (void *) map_p,&img_dsc);
        return;
    }
}


void lv_canavas_draw_letter(lv_draw_ctx_t * draw_ctx, const lv_draw_label_dsc_t * dsc,  const lv_point_t * pos_p,
                       uint32_t letter)
{
    lv_font_glyph_dsc_t g;
    bool g_ret = lv_font_get_glyph_dsc(dsc->font, &g, letter, '\0');
/*Don't draw anything if the character is empty. E.g. space*/
if((g.box_h == 0) || (g.box_w == 0)) return;

lv_point_t gpos;
gpos.x = pos_p->x + g.ofs_x;
gpos.y = pos_p->y + (dsc->font->line_height - dsc->font->base_line) - g.box_h - g.ofs_y;

/*If the letter is completely out of mask don't draw it*/
if(gpos.x + g.box_w < draw_ctx->clip_area->x1 ||
   gpos.x > draw_ctx->clip_area->x2 ||
   gpos.y + g.box_h < draw_ctx->clip_area->y1 ||
   gpos.y > draw_ctx->clip_area->y2)  {
    return;
}

const uint8_t * map_p = lv_font_get_glyph_bitmap(g.resolved_font, letter);
if(map_p == NULL) {
    LV_LOG_WARN("lv_draw_letter: character's bitmap not found");
    return;
}

    draw_letter_normal(draw_ctx, dsc, &gpos, &g, map_p);
}


//font->line_height
//font->base_line
//const uint8_t * map_p = lv_font_get_glyph_bitmap(font_p, letter);
//    if(map_p == NULL) {
//        LV_LOG_WARN("lv_draw_letter: character's bitmap not found");
//        return;
//    }

//void draw_char_rot_at(int16_t posx_i16,int16_t posy_i16,char c,const GFXfont *Font,float rot_d32,uint16_t color_u16)
//{
//	int16_t xshift_i16,yshift_i16;
//	//setTextColor(GUI_TARGET_VSL_COLOR);
//	//setFont(Font);
//	uint8_t first=Font->first;
//	GFXglyph *glyph=(Font->glyph)+(uint8_t)(c-first);
//	uint8_t gw=glyph->width;
//	uint8_t gh=glyph->height;
//	int8_t xo=glyph->xOffset;
//	int8_t yo=glyph->yOffset;
//	uint16_t bo=glyph->bitmapOffset;
//	uint8_t *bitmap=Font->bitmap;
//	int16_t xx, yy, bits = 0, bit = 0;
//	int16_t ic,jc;
//	int16_t px1,py1;
//	float deg_r=rot_d32*(float)M_PI/(float)180.0;
//	float res_d32;
//
//	xshift_i16=xo+gw/2;
//	yshift_i16=yo+gh/2;
//	log_v("xshift_i16(%d) - yshift_i16(%d)",xshift_i16,yshift_i16);
//
//	for (yy=0;yy<gh;yy++) {
//		for (xx=0;xx<gw;xx++) {
//			if (!(bit++ & 7)) {
//				bits = pgm_read_byte(&bitmap[bo++]);
//			}
//			if (bits & 0x80) {
//				ic=xo+xx-xshift_i16;
//				jc=yo+yy-yshift_i16;
//				/*
//				 * Matrix of rotation
//				 * 	-Reference at (0x)
//				 * 	-Rotation in direction of (Oy)
//				 *     cos(t) -sin(t)
//				 *     sin(t) costx)
//				 */
//				res_d32=((float)ic*cosf(deg_r)-(float)jc*sinf(deg_r))/2+(float)posx_i16;
//				px1=(int16_t)(res_d32);
//				res_d32=((float)ic*sinf(deg_r)+(float)jc*cosf(deg_r))/2+(float)posy_i16;
//				py1=(int16_t)(res_d32);
//				drawPixel(px1,py1,color_u16);
//			}
//			bits <<= 1;
//		}
//	}
//};
