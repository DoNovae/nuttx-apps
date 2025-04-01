#include <stdio.h>
#include  "types.h"
#include  "display.h"





/*
 * ===================
 * Defines
 * -------------------
 */
#define VESSELS_PANEL_WIDTH 153
#define VESSELS_PANEL_HEIGHT (134-5)

#define VESSELS_PANEL1_POSX (4+1)
#define VESSELS_PANEL1_POSY (60+5)

#define VESSELS_PANEL2A_POSX (163+5)
#define VESSELS_PANEL2A_POSY (60+5)

#define VESSELS_PANEL2B_POSX (163+VESSELS_PANEL_WIDTH/2+5)
#define VESSELS_PANEL2B_POSY (60+5)

#define VESSELS_ALERT_LINE_NBR 7

#define VESSELS_PANEL2_LINE_NBR (DISPLAY_VESSELS_NBR/2)


#define VESSELS_ALERT_CHR_PER_LINE 16
#define VESSELS_ALERT_STR_LEN (VESSELS_ALERT_CHR_PER_LINE*VESSELS_ALERT_LINE_NBR+1)

#define VESSELS_PANEL2_CHR_PER_LINE 5
#define VESSELS_PANEL2_STR_LEN (VESSELS_PANEL2_CHR_PER_LINE*VESSELS_ALERT_LINE_NBR+1)


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
static char Panel1_str[VESSELS_ALERT_STR_LEN];
static char Panel2a_str[VESSELS_PANEL2_STR_LEN];
static char Panel2b_str[VESSELS_PANEL2_STR_LEN];
static lv_obj_t *Vessels_display_ps;
static lv_obj_t *Vessels_panel1_ps, *Vessels_panel2a_ps, *Vessels_panel2b_ps;



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

	lv_display_cmd(parent);
	lv_display_status(parent);
	lv_audio_cmd(parent);

	/*
	 * Alarm panel
	 */
	Vessels_panel1_ps=lv_label_create(parent);
	lv_obj_set_style_text_font(Vessels_panel1_ps,&lv_font_doais_16,0);
	lv_obj_set_style_text_color(Vessels_panel1_ps,lv_color_hex(BLACK_RGB),0);

	lv_obj_set_width(Vessels_panel1_ps,VESSELS_PANEL_WIDTH);
	lv_obj_set_height(Vessels_panel1_ps,VESSELS_PANEL_HEIGHT);
	lv_obj_set_style_text_align(Vessels_panel1_ps,LV_TEXT_ALIGN_LEFT,0);
	lv_obj_align(Vessels_panel1_ps,LV_ALIGN_TOP_LEFT,VESSELS_PANEL1_POSX,VESSELS_PANEL1_POSY);

	memset((void*)Panel1_str,0,VESSELS_ALERT_STR_LEN);
	Panel1_str[0]=' ';
	lv_label_set_text_static(Vessels_panel1_ps,Panel1_str);

	/*
	 * Panel2
	 */
	Vessels_panel2a_ps=lv_label_create(parent);
	lv_obj_set_style_text_font(Vessels_panel2a_ps,&lv_font_doais_16,0);
	lv_obj_set_style_text_color(Vessels_panel2a_ps,lv_color_hex(BLACK_RGB),0);

	lv_obj_set_width(Vessels_panel2a_ps,(VESSELS_PANEL_WIDTH)/2);
	lv_obj_set_height(Vessels_panel2a_ps,VESSELS_PANEL_HEIGHT);
	lv_obj_set_style_text_align(Vessels_panel2a_ps,LV_TEXT_ALIGN_LEFT,0);
	lv_obj_align(Vessels_panel2a_ps,LV_ALIGN_TOP_LEFT,VESSELS_PANEL2A_POSX,VESSELS_PANEL2A_POSY);

	memset((void*)Panel2a_str,0,VESSELS_PANEL2_STR_LEN);
	Panel2a_str[0]=' ';
	lv_label_set_text_static(Vessels_panel2a_ps,Panel2a_str);

	Vessels_panel2b_ps=lv_label_create(parent);
	lv_obj_set_style_text_font(Vessels_panel2b_ps,&lv_font_doais_16,0);
	lv_obj_set_style_text_color(Vessels_panel2b_ps,lv_color_hex(BLACK_RGB),0);

	lv_obj_set_width(Vessels_panel2b_ps,(VESSELS_PANEL_WIDTH)/2);
	lv_obj_set_height(Vessels_panel2b_ps,VESSELS_PANEL_HEIGHT);
	lv_obj_set_style_text_align(Vessels_panel2b_ps,LV_TEXT_ALIGN_LEFT,0);
	lv_obj_align(Vessels_panel2b_ps,LV_ALIGN_TOP_LEFT,VESSELS_PANEL2B_POSX,VESSELS_PANEL2B_POSY);

	memset((void*)Panel2b_str,0,VESSELS_PANEL2_STR_LEN);
	Panel2b_str[0]=' ';
	lv_label_set_text_static(Vessels_panel2b_ps,Panel2b_str);
}




/*
 * ====================
 * list_vessels_one_alert
 * --------------------
 *
 */
static uint32_t time_to_cpa_mn_au32[VESSELS_ALERT_LINE_NBR];

void vessels_list_one_alert(int16_t id_i16,char label,uint32_t time_to_cpa_mn_u32,char * name_pc,uint16_t color_u16)
{
	if (id_i16<VESSELS_ALERT_LINE_NBR)
	{
		time_to_cpa_mn_au32[id_i16]=time_to_cpa_mn_u32;
	} else
	{
		/*
		 * Get alert of max time_to_cpa_mn_u32
		 */
		uint32_t max_mn_u32=time_to_cpa_mn_u32;
		id_i16=-1;
		for (int16_t i=0;i<VESSELS_ALERT_LINE_NBR;i++)
		{
			if (max_mn_u32<time_to_cpa_mn_au32[i])
			{
				id_i16=i;
				max_mn_u32=time_to_cpa_mn_au32[i];
			}
			if (id_i16==-1)
			{
				//LOG_D("Drop alert");
				return;
			}
		}
		/*
		 * Erase old id_i16 of max time_to_cpa_mn_u32
		 */
		int16_t pos_i16=id_i16*VESSELS_ALERT_CHR_PER_LINE;
		//LOG_D("vessels_list_one_alert: erase id_i16(%d) - pos_i16(%d)",id_i16,pos_i16);
		sprintf(Panel1_str+pos_i16,"%16.16s","                ");
		Panel1_str[pos_i16+VESSELS_ALERT_CHR_PER_LINE-1]='\n';
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

	int16_t pos_i16=id_i16*VESSELS_ALERT_CHR_PER_LINE;
	//LOG_D("vessels_list_one_alert: id_i16(%d) - pos_i16(%d)",id_i16,pos_i16);

	sprintf(Panel1_str+pos_i16,"%c-%2d\'%10.10s\n",label,time_to_cpa_mn_u32,name_pc);
}


/*
 * ====================
 * vessels_alert_push
 * --------------------
 */
void vessels_alert_push()
{
	lv_label_set_text_static(Vessels_panel1_ps,Panel1_str);
}

/*
 * ====================
 * vessels_alert_reset
 * --------------------
 */
void vessels_alert_reset()
{
	memset((void*)Panel1_str,0,VESSELS_ALERT_STR_LEN);
	Panel1_str[0]=' ';
	lv_label_set_text_static(Vessels_panel1_ps,Panel1_str);
}

/*
 * ====================
 * vessels_list_vessel
 * --------------------
 */
void vessels_list_vessel(int16_t id_i16,char label,uint32_t speed_kt,uint16_t color_u16,uint16_t color_spd_u16, bool old_b)
{
	//LOG_D("vessels_list_vessel: id_i16(%d)",id_i16);

	if (id_i16<(DISPLAY_VESSELS_NBR/2))
	{
		vessels_list_one_vessel(VESSELS_CANVAS1,id_i16,label,speed_kt,color_u16,color_spd_u16,old_b);
	} else if (id_i16<DISPLAY_VESSELS_NBR)
	{
		vessels_list_one_vessel(VESSELS_CANVAS2,id_i16-DISPLAY_VESSELS_NBR/2,label,speed_kt,color_u16,color_spd_u16,old_b);
	}
}



/*
 * ====================
 * draw_vessels_one_vessel
 * --------------------
 */
void vessels_list_one_vessel(vessels_canvas_e canvas_e,int16_t id_i16,char label,uint32_t speed_kt,uint16_t color_u16,uint16_t color_spd_u16,bool old_b)
{
	int16_t pos_i16=id_i16*VESSELS_PANEL2_CHR_PER_LINE;

	if (id_i16>VESSELS_PANEL2_LINE_NBR-1)
	{
		LOG_E("vessels_list_one_vessel: ERR too many lines (%d) > VESSELS_PANEL2_LINE_NBR-1",id_i16);
		return;
	}

	if (canvas_e==VESSELS_CANVAS1)
	{
		//LOG_D("vessels_list_one_vessel: VESSELS_CANVAS1 id_i16(%d) - pos_i16(%d)",id_i16,pos_i16);
		if (old_b)
		{
			sprintf(Panel2a_str+pos_i16,"%c!%2.2d\n",label,(speed_kt+5)/10);
		}else
		{
			sprintf(Panel2a_str+pos_i16,"%c-%2.2d\n",label,(speed_kt+5)/10);
		}

	} else
	{
		//LOG_D("vessels_list_one_vessel: VESSELS_CANVAS2 id_i16(%d) - pos_i16(%d)",id_i16,pos_i16);
		if (old_b)
		{
			sprintf(Panel2b_str+pos_i16,"%c!%2.2d\n",label,(speed_kt+5)/10);
		}else
		{
			sprintf(Panel2b_str+pos_i16,"%c-%2.2d\n",label,(speed_kt+5)/10);
		}
	}
}


/*
 * ====================
 * vessels_push
 * --------------------
 */
void vessels_push()
{
	lv_label_set_text_static(Vessels_panel2a_ps,Panel2a_str);
	lv_label_set_text_static(Vessels_panel2b_ps,Panel2b_str);
}

/*
 * ====================
 * vessels_reset
 * --------------------
 */
void vessels_reset()
{
	memset((void*)Panel2a_str,0,VESSELS_PANEL2_STR_LEN);
	Panel2a_str[0]=' ';
	lv_label_set_text_static(Vessels_panel2a_ps,Panel2a_str);
	memset((void*)Panel2b_str,0,VESSELS_PANEL2_STR_LEN);
	Panel2b_str[0]=' ';
	lv_label_set_text_static(Vessels_panel2b_ps,Panel2b_str);
}


