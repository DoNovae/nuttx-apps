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

#define TARGET_CENTER_POSX 112
#define TARGET_CENTER_POSY 112

#define TARGET_LABEL_WIDTH 14
#define TARGET_LABEL_HEIGHT 18


/*
 * ===================
 * Defines
 * -------------------
 */
#define LABEL_MG_NB 16



/*
 * ===================
 * Types
 * -------------------
 */
typedef struct
{
	FAR lv_img_dsc_t * img_p;
	char ltr;
} label_img_t;



/*
 * Target labels
 */
LV_IMG_DECLARE(Img0_s);

#define TARGET_LABEL_NBR 15
LV_IMG_DECLARE(Img2_s);
LV_IMG_DECLARE(Img4_s);
LV_IMG_DECLARE(Img5_s);
LV_IMG_DECLARE(Img7_s);
LV_IMG_DECLARE(ImgA_s);
LV_IMG_DECLARE(ImgB_s);
LV_IMG_DECLARE(ImgF_s);
LV_IMG_DECLARE(ImgG_s);
LV_IMG_DECLARE(ImgJ_s);
LV_IMG_DECLARE(ImgP_s);
LV_IMG_DECLARE(ImgT_s);
LV_IMG_DECLARE(ImgV_s);
LV_IMG_DECLARE(ImgY_s);
LV_IMG_DECLARE(ImgQ_s);
LV_IMG_DECLARE(ImgR_s);


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

ais_monitoring_zoom_e Zoom_s=AIS_MONITORING_Z0;
bool Is_cross_b=false;

static label_img_t Label_img_a[LABEL_MG_NB];

/*
 * ===================
 * Extern
 * -------------------
 */
LV_IMG_DECLARE(Img_target_s);
LV_IMG_DECLARE(Img_bell_s);
LV_IMG_DECLARE(Target_canavas_s);


/*
 * ===================
 * Prototypes
 * -------------------
 */
void lv_display_target(lv_obj_t *parent);
void lv_target_bell(lv_obj_t *parent);
lv_img_dsc_t* label_ing_get(char c);


/*
 * ================================================================
 *           TARGET FUNCTIONS
 */

/*
 * ======================
 * label_ing_init
 * ----------------------
 */
void label_ing_init()
{
	Label_img_a[0].ltr='2';Label_img_a[0].img_p=&Img2_s;
	Label_img_a[1].ltr='4';Label_img_a[1].img_p=&Img4_s;
	Label_img_a[2].ltr='5';Label_img_a[2].img_p=&Img5_s;
	Label_img_a[3].ltr='7';Label_img_a[3].img_p=&Img7_s;

	Label_img_a[4].ltr='A';Label_img_a[4].img_p=&ImgA_s;
	Label_img_a[5].ltr='B';Label_img_a[5].img_p=&ImgB_s;
	Label_img_a[6].ltr='F';Label_img_a[6].img_p=&ImgF_s;
	Label_img_a[7].ltr='G';Label_img_a[7].img_p=&ImgG_s;
	Label_img_a[8].ltr='J';Label_img_a[8].img_p=&ImgJ_s;

	Label_img_a[9].ltr='P';Label_img_a[9].img_p=&ImgP_s;
	Label_img_a[10].ltr='T';Label_img_a[10].img_p=&ImgT_s;

	Label_img_a[11].ltr='V';Label_img_a[11].img_p=&ImgV_s;
	Label_img_a[12].ltr='Y';Label_img_a[12].img_p=&ImgY_s;

	Label_img_a[13].ltr='Q';Label_img_a[13].img_p=&ImgQ_s;
	Label_img_a[14].ltr='R';Label_img_a[14].img_p=&ImgR_s;
	Label_img_a[15].ltr='?';Label_img_a[15].img_p=&Img0_s;
}


/*
 * ======================
 * label_ing_gets
 * ----------------------
 */
lv_img_dsc_t* label_ing_get(char c)
{
	uint8_t u8;
	for (u8=0;u8<TARGET_LABEL_NBR;u8++)
	{
		if (Label_img_a[u8].ltr==c)
		{
			//LOG_D("label_ing_get: get(%c)",c);
			return Label_img_a[u8].img_p;
		}
	}
	return 0;
}


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
 * Zoom factor.v
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
	lv_obj_add_flag(Bell_display_ps,LV_OBJ_FLAG_HIDDEN);
}

/*
 * ===================
 * lv_target_bell_off
 * -------------------
 */
void lv_target_bell_off(void)
{
	lv_obj_add_flag(Bell_display_ps,LV_OBJ_FLAG_HIDDEN);
}

/*
 * ===================
 * lv_target_bell_on
 * -------------------
 */
void lv_target_bell_on(void)
{
	lv_obj_clear_flag(Bell_display_ps,LV_OBJ_FLAG_HIDDEN);
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
				if (Zoom_s==AIS_MONITORING_Z0) Zoom_s=AIS_MONITORING_Z1;
				else Zoom_s=AIS_MONITORING_Z2;
			} else if (strcmp("-",txt)==0)
			{
				if (Zoom_s==AIS_MONITORING_Z2) Zoom_s=AIS_MONITORING_Z1;
				else Zoom_s=AIS_MONITORING_Z0;
			} else if (strcmp("X",txt) == 0)
			{
				if (Is_cross_b) Is_cross_b=false; else Is_cross_b=true;
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
	lv_obj_set_pos(Target_canvas_ps,TARGET_CORNER_RL_POSX,TARGET_CORNER_RL_POSY);

	lv_update_status();
}


/*
 * ==========================
 * lv_update_target
 * --------------------------
 *
 * --------------------------
 */

#define SCALE_POSX 196
#define SCALE_POSY 25
#define SCALE_WIDTH 33

void lv_target_update(float max_nm_d32,float *scale_px_nm_d32_p)
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

	*scale_px_nm_d32_p=(float)DISPLAY_TARGET_R3/Settings_s.display_target_step_nm_u32/(float)MONITORING_DISPLAY_STEPS_NB;
	lv_canvas_draw_arc(Target_canvas_ps,TARGET_CENTER_POSX,TARGET_CENTER_POSY,DISPLAY_TARGET_R3,0,360,&arc_dsc);

	max_nm_d32=(float)Settings_s.display_target_step_nm_u32*(float)MONITORING_DISPLAY_STEPS_NB;
	alert_circ_d32=(float)DISPLAY_TARGET_R3*(float)Settings_s.cpa_warn_10thnm_u32/(float)10.0/max_nm_d32;

	switch (Zoom_s){
	case AIS_MONITORING_Z0:
		lv_canvas_draw_arc(Target_canvas_ps,TARGET_CENTER_POSX,TARGET_CENTER_POSY,DISPLAY_TARGET_R2,0,360,&arc_dsc);
		lv_canvas_draw_arc(Target_canvas_ps,TARGET_CENTER_POSX,TARGET_CENTER_POSY,DISPLAY_TARGET_R1,0,360,&arc_dsc);
		break;
	case AIS_MONITORING_Z1:
		lv_canvas_draw_arc(Target_canvas_ps,TARGET_CENTER_POSX,TARGET_CENTER_POSY,DISPLAY_TARGET_R32,0,360,&arc_dsc);
		*scale_px_nm_d32_p*=(float)3/2;
		max_nm_d32=(float)max_nm_d32*(float)2.0/(float)3.0;
		alert_circ_d32=alert_circ_d32*(float)3.0/(float)2.0;
		break;
	case AIS_MONITORING_Z2:
		*scale_px_nm_d32_p*=(float)3;
		max_nm_d32=(float)max_nm_d32/(float)3;
		alert_circ_d32=alert_circ_d32*(float)3.0;
		break;
	}
	arc_dsc.color=lv_color_hex(DISPLAY_RED_RGB);
	lv_canvas_draw_arc(Target_canvas_ps,TARGET_CENTER_POSX,TARGET_CENTER_POSY,(int16_t)(alert_circ_d32+(float)0.5),0,360,&arc_dsc);

	/*
	 * Lines
	 */
	lv_draw_line_dsc_t line_dsc;
	lv_draw_line_dsc_init(&line_dsc);
	line_dsc.color=lv_color_black();
	line_dsc.width=1;
	lv_point_t line[2];

	line[0].x=0;line[0].y=TARGET_CENTER_POSY;
	line[1].x=TARGET_CANVAS_WIDTH;line[1].y=TARGET_CENTER_POSY;
	lv_canvas_draw_line(Target_canvas_ps,line,2,&line_dsc);
	line[0].x=TARGET_CENTER_POSX;line[0].y=0;
	line[1].x=TARGET_CENTER_POSX;line[1].y=TARGET_CANVAS_HEIGHT;
	lv_canvas_draw_line(Target_canvas_ps,line,2,&line_dsc);

	/*
	 * Cross
	 */
	if (Is_cross_b)
	{
		line_dsc.color=lv_color_hex(DISPLAY_RED_RGB);
		line[0].x=TARGET_CENTER_POSX-10;line[0].y=TARGET_CENTER_POSY-10;
		line[1].x=TARGET_CENTER_POSX+10;line[1].y=TARGET_CENTER_POSY+10;
		lv_canvas_draw_line(Target_canvas_ps,line,2,&line_dsc);
		line[0].x=TARGET_CENTER_POSX-10;line[0].y=TARGET_CENTER_POSY+10;
		line[1].x=TARGET_CENTER_POSX+10;line[1].y=TARGET_CENTER_POSY-10;
		lv_canvas_draw_line(Target_canvas_ps,line,2,&line_dsc);
	}

	// Scale in NM
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
	//lv_target_push();
}


/*
 * ====================
 * lv_target_push
 * --------------------
 */
void lv_target_push()
{
	/*
	 * Display target
	 */
	lv_canvas_set_buffer(Target_canvas_ps,Target_canavas_au8,TARGET_CANVAS_WIDTH,TARGET_CANVAS_HEIGHT,LV_IMG_CF_TRUE_COLOR_ALPHA);
}


/*
 * ====================
 * draw_target_vessel
 * --------------------
 */
void draw_target_vessel(char label,int16_t posx_i16,int16_t posy_i16,float head_d32,uint32_t color_u32, lv_opa_t opa_e)
{
	lv_img_dsc_t * img_p;
	img_p=label_ing_get(label);
	if (!img_p)
	{
		LOG_E("draw_target_vessels: label unknown %c",label);
		return;
	}

	lv_draw_img_dsc_t img_dsc;
	lv_draw_img_dsc_init(&img_dsc);
	img_dsc.opa=opa_e;
	img_dsc.recolor=lv_color_hex(color_u32);
	img_dsc.recolor_opa=LV_OPA_100;
	img_dsc.angle=head_d32*10;
	img_dsc.pivot.x=TARGET_LABEL_WIDTH/2;
	img_dsc.pivot.y=TARGET_LABEL_HEIGHT/2;
	lv_canvas_draw_img(Target_canvas_ps,TARGET_CENTER_POSX+posx_i16-TARGET_LABEL_WIDTH/2,TARGET_CENTER_POSY-posy_i16-TARGET_LABEL_HEIGHT/2,img_p,&img_dsc);
}






