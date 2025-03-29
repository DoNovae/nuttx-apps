#include <stdio.h>
#include  "types.h"
#include  "display.h"







/*
 * ===================
 * Defines
 * -------------------
 */
#define VESSELS_PANEL_WIDTH  153
#define VESSELS_PANEL_HEIGHT 60
#define VESSELS_PANEL_STR_LEN 100

#define VESSELS_PANEL1_POSX 4
#define VESSELS_PANEL1_POSY 60

#define VESSELS_PANEL2_POSX 163
#define VESSELS_PANEL2_POSY 60


typedef enum
{
	VESSELS_CANNVAS_N0NE=0,
	VESSELS_CANVAS1=1,
	VESSELS_CANVAS2=2
} vessels_canvas_e;


/*
 * ===================
 * Prototypes
 * -------------------
 */
void vessels_list_one_vessel(vessels_canvas_e canvas_e,int16_t id_i16,char label,uint32_t speed_kt,uint16_t color_u16,uint16_t color_spd_u16,bool old_b);

/*
 * ===================
 * Globals
 * -------------------
 */
static char Panel1_str[VESSELS_PANEL_STR_LEN];
static char Panel2_str[VESSELS_PANEL_STR_LEN];
static lv_obj_t *Vessels_display_ps;
static lv_obj_t *Vessels_panel1_ps, *Vessels_panel2_ps;

/*
 * ===================
 * Extern
 * -------------------
 */
LV_IMG_DECLARE(Img_vessels_s);


/**
 * -------------------
 * lv_vessels_display
 * -------------------
 */
void lv_vessels_display(lv_obj_t *parent)
{
	Vessels_display_ps=lv_img_create(parent);
	lv_img_set_src(Vessels_display_ps,&Img_vessels_s);

	Vessels_panel1_ps = lv_label_create(parent);
	lv_obj_set_style_text_font(Vessels_panel1_ps,&lv_font_doais_16,0);
	lv_obj_set_style_text_color(Vessels_panel1_ps,lv_color_hex(BLACK_RGB),0);

	lv_obj_set_width(Vessels_panel1_ps,VESSELS_PANEL_WIDTH);
	lv_obj_set_style_text_align(Vessels_panel1_ps, LV_TEXT_ALIGN_LEFT, 0);
	lv_obj_align(Vessels_panel1_ps, LV_ALIGN_TOP_LEFT,VESSELS_PANEL1_POSX,VESSELS_PANEL1_POSY);

	lv_display_cmd(parent);
	lv_display_status(parent);
	lv_audio_cmd(parent);
}





/*
 * ====================
 * list_vessels_one_alert
 * --------------------
 *
 */
#define DISPLAY_ALERT_SZ 5
static uint32_t time_to_cpa_mn_au32[DISPLAY_ALERT_SZ];

void vessels_list_one_alert(int16_t id_i16,char label,uint32_t time_to_cpa_mn_u32,char * name_pc,uint16_t color_u16)
{
	if (id_i16<DISPLAY_ALERT_SZ)
	{
		time_to_cpa_mn_au32[id_i16]=time_to_cpa_mn_u32;
	} else
	{
		/*
		 * Get alert of max time_to_cpa_mn_u32
		 */
		uint32_t max_mn_u32=time_to_cpa_mn_u32;
		id_i16=-1;
		for (int16_t i=0;i<DISPLAY_ALERT_SZ;i++)
		{
			if (max_mn_u32<time_to_cpa_mn_au32[i])
			{
				id_i16=i;
				max_mn_u32=time_to_cpa_mn_au32[i];
			}
			if (id_i16==-1)
			{
				LOG_D("Drop alert");
				return;
			}
		}
		/*
		 * Erase old id_i16 of max time_to_cpa_mn_u32
		 */
/*		alert.setCursor(1,(7+26*id_i16));
		alert.setTextColor(TFT_WHITE,TFT_WHITE);
		alert.printf("%-20.20s","                             ");*/
	}
	/*
	 * Uppercase to lowercase
	 */
	uint8_t i=0;
	for(i=0;i<strlen(name_pc);i++){
		if((name_pc[i]>=65)&&(name_pc[i]<=90))
			name_pc[i]=name_pc[i]+32;
	}
	/*
	 * Suppress space from end
	 */
#define ASCII_SPACE 32
	i=strlen(name_pc)-1;
	while ((i>=0)&&(name_pc[i]==(char)ASCII_SPACE))
	{
		name_pc[i]=0;
		i--;
	}
/*
	alert.setCursor(1,(7+26*id_i16));
	alert.setTextColor(color_u16,TFT_WHITE);
	alert.printf("%c-",label);
	alert.setTextColor(DISPLAY_RED,TFT_WHITE);
	alert.printf("%2d\'",time_to_cpa_mn_u32);
	alert.setTextColor(DISPLAY_DARKGREY,TFT_WHITE);
	alert.printf("%-12.12s",name_pc);// left alignment 14 char min - truncated 14 char
*/
	memset((void*)Panel1_str,0,VESSELS_PANEL_STR_LEN);
	//sprintf(Panel1_str,"%03dk%03d" LV_SYMBOL_DEGREE,spd_u16,heading_u16);
	sprintf(Panel1_str,"%c-%2d%-12.12s",label,time_to_cpa_mn_u32,name_pc);
	lv_label_set_text_static(Vessels_panel1_ps,Panel1_str);
}





/*
 * ====================
 * list_vessels
 * --------------------
 *
 */
void vessels_list_vessel(int16_t id_u16,char label,uint32_t speed_kt,uint16_t color_u16,uint16_t color_spd_u16, bool old_b)
{
    vessels_canvas_e canvas_e;
	if (id_u16<(DISPLAY_VESSELS_NBR>>1))
	{
		canvas_e=VESSELS_CANVAS1;
		vessels_list_one_vessel(canvas_e,id_u16,label,speed_kt,color_u16,color_spd_u16,old_b);
	} else if (id_u16<DISPLAY_VESSELS_NBR)
	{
		canvas_e=VESSELS_CANVAS2;
		vessels_list_one_vessel(canvas_e,id_u16-5,label,speed_kt,color_u16,color_spd_u16,old_b);
	}
}



/*
 * ====================
 * draw_vessels_one_vessel
 * --------------------
 */
void vessels_list_one_vessel(vessels_canvas_e canvas_e,int16_t id_i16,char label,uint32_t speed_kt,uint16_t color_u16,uint16_t color_spd_u16,bool old_b)
{
	/*if (canvas_e==VESSELS_CANVAS1){
		vessels1.setTextColor(color_u16,TFT_WHITE);
		vessels1.setCursor(6,(7+26*id_i16));
		if (old_b) vessels1.printf("%c! ",label); else vessels1.printf("%c- ",label);
		vessels1.setTextColor(color_spd_u16,TFT_WHITE);


		 * %2.2d: 2 digits in 2 char length XX

		vessels1.printf("%2.2d",(speed_kt+5)/10);
	} else {
		vessels2.setTextColor(color_u16,TFT_WHITE);
		vessels2.setCursor(6,(7+26*id_i16));
		if (old_b) vessels2.printf("%c! ",label); else vessels2.printf("%c- ",label);
		vessels2.setTextColor(color_spd_u16,TFT_WHITE);
		vessels2.printf("%2.2d",(speed_kt+5)/10);
	}*/
}








