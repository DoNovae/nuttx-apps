/**
 * =====================================
 * gui.cpp
 * -------------------------------------
 *  AIS DoNovae
 *  www.DoNovae.com
 *  Herve Bailly
 * =====================================
 */
#include "gui.h"

#include <SPI.h>
#include <esp_log.h>

#include "LiberationMono_Bold12pt7b.h"
#include "LiberationMono_Bold24pt7b.h"
#define FONT12PT LiberationMono_Bold12pt7b
#define FONT24PT LiberationMono_Bold24pt7b


/*
#include "POE_Vetica_Monospace_Bold__attempt_12pt7b.h"
#include "POE_Vetica_Monospace_Bold__attempt_24pt7b.h"
#define FONT12PT POE_Vetica_Monospace_Bold__attempt_12pt7b
#define FONT24PT POE_Vetica_Monospace_Bold__attempt_24pt7b
 */


/*
 * ==========================================
 * Defines
 * ==========================================
 */
#define GUI_TOUCH_MARGIN_X 2
#define GUI_TOUCH_MARGIN_Y 0




/*
 * Speaker
 */
Gui_spk Speaker;

/*
 * -----------------------
 * State machine
 * -----------------------
 */
gui_state_machine_e Gui_state_s=GUI_SM_START;

/*
 * ----------------
 * Target
 * ----------------
 * 	GUI_TGT_VSL=0
 * 	GUI_TGT_SET=1
 * 	GUI_TGT_SPK=2
 * 	GUI_TGT_CROSS=3
 * 	GUI_TGT_ZIN=4
 * 	GUI_TGT_ZOUT=5
 */
const gui_touch_area_t GUI_TOUCH_AREAS_TGT[GUI_TOUCH_AREA_TARGET_NB]=
{
		{{39,209},{78,239}}, // GUI_TGT_VSL
		{{78,209},{133,239}},// GUI_TGT_SET
		{{0,157},{40,187}},  // GUI_TGT_SPK
		{{40,157},{95,187}},  // GUI_TGT_CROSS
		{{0,56},{39,86}},    // GUI_TGT_ZIN
		{{39,56},{94,86}}    // GUI_TGT_ZOUT
};


/*
 * ----------------
 * VESSELS
 * ----------------
 *	GUI_VSL_TGT=0
 *	GUI_VSL_SET=1
 *	GUI_VSL_SPK=2
 */
const gui_touch_area_t GUI_TOUCH_AREAS_VSL[GUI_TOUCH_AREA_VESSELS_NB]=
{
		{{0,209},{39,239}},  // GUI_VSL_TGT
		{{78,209},{133,239}},// GUI_TGT_SET
		{{160,209},{215,239}},  // GUI_TGT_SPK
};


/*
 * ----------------
 * SETTINGS
 * ----------------
 * 	GUI_SET_TGT=0
 * 	GUI_SET_VSL=1
 * 	GUI_SET_SPK=2
 * 	GUI_SET_WIFI=3
 */
const gui_touch_area_t GUI_TOUCH_AREAS_SET[GUI_TOUCH_AREA_SETTINGS_NB]=
{
		{{0,209},{39,239}},  // GUI_VSL_TGT
		{{39,209},{78,239}}, // GUI_TGT_VSL
		{{0,160},{39,190}},  // GUI_TGT_SPK
		{{39,160},{94,190}}  // GUI_SET_WIFI=3
};

/*
 * ----------------
 * Animation wifi
 * ----------------
 *
 */
const uint16_t *GUI_ANIMATION_WIFI_A[GUI_ANIMATION_WIFI_NB]={GUI_WIFI_ON,GUI_SPEAKER_WIFI_OFF,GUI_WIFI_HS};
const gui_point_t GUI_ANIMATION_WIFI_XY={274,4};
const gui_point_t GUI_ANIMATION_WIFI_SZ={20,20};

/*
 * ----------------
 * Animation speaker
 * ----------------
 *
 */
const uint16_t *GUI_ANIMATION_SPK_A[GUI_ANIMATION_SPK_NB]={GUI_SPEAKER_ON,GUI_SPEAKER_WIFI_OFF};
const gui_point_t GUI_ANIMATION_SPK_XY={296,4};
const gui_point_t GUI_ANIMATION_SPK_SZ={20,20};

/*
 * ----------------
 * Animation gps
 * ----------------
 *
 */
const uint16_t *GUI_ANIMATION_GPS_A[GUI_ANIMATION_GPS_NB]={GUI_GPS0,GUI_GPS1,GUI_GPS2,GUI_GPS3};
const gui_point_t GUI_ANIMATION_GPS_XY={252,4};
const gui_point_t GUI_ANIMATION_GPS_SZ={20,20};

/*
 * ----------------
 * Slider
 * ----------------
 *
 */
const gui_point_t GUI_SLIDER_XY={191,45};
const gui_point_t GUI_SLIDER_CURSOR_SZ={30,30};
#define  SLIDER_MARGIN_Y 2


/*
 * ----------------
 * Animation bell
 * ----------------
 *
 */
const uint16_t *GUI_ANIMATION_BELL_A[GUI_ANIMATION_BELL_NB]={GUI_BELL_ON,GUI_BELL_OFF};
const gui_point_t GUI_ANIMATION_BELL_XY={22,96};
const gui_point_t GUI_ANIMATION_BELL_SZ={50,50};






/*
 * ================
 * Class Gui
 * ----------------
 */
gui_touch_e Gui::touch_update(void)
{
	gui_touch_e touch_e=GUI_TOUCH_NONE;
	if (touch_points_u8)
	{
		memcpy(prev_tp,tp,sizeof(m5gfx::touch_point_t)*touch_points_u8);
	}
	prev_touch_points_u8=touch_points_u8;
	touch_points_u8=M5.Lcd.getTouch(tp,2);
	touch_e=GUI_TOUCH_NONE;
	if ((prev_touch_points_u8==0)&&(touch_points_u8!=0)){
		touch_e=GUI_JUST_TOUCH;
	}
	if (prev_touch_points_u8){
		touch_e=GUI_TOUCH_HOLD;
	}
	return (touch_e);
}

m5gfx::touch_point_t Gui::prev_tp[2];
m5gfx::touch_point_t Gui::tp[2];
uint8_t Gui::touch_points_u8=0;
uint8_t Gui::prev_touch_points_u8=0;

/*
 * ================
 * Class Gui_page
 * ----------------
 */
Gui_page::Gui_page(const uint16_t *img_pu16,const gui_touch_area_t*tch_a_p,const uint8_t a_nb_u8)
{
	bck_gnd_u16=img_pu16;
	touch_area_p=tch_a_p;
	areas_nb_u8=a_nb_u8;
};

Gui_page::~Gui_page(){};



/*
 * -----------------------
 * draw_background
 * -----------------------
 */
void Gui_page::draw_background()
{
	M5.Lcd.drawBitmap(0,0,320,240,(m5gfx::rgb565_t*)bck_gnd_u16);
}

void Gui_page::begin()
{
	M5.Lcd.setBrightness(127);
}


/*
 * -----------------------
 * area_selection
 * -----------------------
 */
int8_t Gui_page::area_selection()
{
	for (uint8_t i=0;i<areas_nb_u8;i++){
		if ((touch_area_p[i].p0.x+GUI_TOUCH_MARGIN_X<Gui::tp->x)&&(Gui::tp->x<touch_area_p[i].p1.x-GUI_TOUCH_MARGIN_X)&&
				(touch_area_p[i].p0.y+GUI_TOUCH_MARGIN_Y<Gui::tp->y)&&(Gui::tp->y<touch_area_p[i].p1.y-GUI_TOUCH_MARGIN_Y)){
			Speaker.play_click();
			return i;
		}
	}
	return -1;
}


/*
 * =======================
 * Class Gui_animation
 * -----------------------
 */
Gui_animation::Gui_animation(const gui_point_t *posxy_ps, const gui_point_t *sizesxy_ps, const uint16_t **img_pu16,const uint8_t img_nb_u8){
	image_pu16=img_pu16;
	image_nb_u8=img_nb_u8;
	Posxy_s.x=posxy_ps->x;
	Posxy_s.y=posxy_ps->y;
	Sizexy_s.x=sizesxy_ps->x;
	Sizexy_s.y=sizesxy_ps->y;
}

Gui_animation::~Gui_animation(){}

void Gui_animation::draw(gui_animation_t id_s){
	if (id_s.u8>=image_nb_u8){
		LOG_E("Gui_animation.draw: ERROR id_s.u8(%d)>=image_nb_u8(%d)",id_s.u8,image_nb_u8);
		return;
	}
	M5.Lcd.drawBitmap(Posxy_s.x,Posxy_s.y,Sizexy_s.x,Sizexy_s.y,(void*)image_pu16[id_s.u8]);
}

/*
 * =======================
 * Class Gui_slider
 * -----------------------
 */
#define SLIDER_GUIDE_W 5
Gui_slider::Gui_slider(const gui_point_t *posxy_ps,uint16_t hgt_u16,const gui_point_t *sizesxy_ps,const uint16_t *cur_pu16,const uint16_t *msk_pu16)
{
	cursor_pu16=cur_pu16;
	mask_pu16=msk_pu16;
	Sizexy_s.x=sizesxy_ps->x;
	Sizexy_s.y=sizesxy_ps->y;
	height_u16=hgt_u16-(SLIDER_MARGIN_Y<<1)-sizesxy_ps->y;
	Origxy_s.x=posxy_ps->x;
	Origxy_s.y=posxy_ps->y+hgt_u16-SLIDER_MARGIN_Y-sizesxy_ps->y;
	Posxy_s.x=Origxy_s.x-(GUI_SLIDER_CURSOR_SZ.x>>1)+SLIDER_GUIDE_W;
	Posxy_s.y=Origxy_s.y;
}


Gui_slider::~Gui_slider(){}


void Gui_slider::draw(uint16_t h_u16)
{
	M5.Lcd.pushImage(Posxy_s.x,Posxy_s.y,Sizexy_s.x,Sizexy_s.y,(m5gfx::rgb565_t*)mask_pu16);
	if (h_u16>height_u16) h_u16=height_u16;
	Posxy_s.y=Origxy_s.y-h_u16;
	M5.Lcd.pushImage(Posxy_s.x,Posxy_s.y,Sizexy_s.x,Sizexy_s.y,(m5gfx::rgb565_t*)cursor_pu16);
}

void Gui_slider::draw()
{
	Posxy_s.y=Origxy_s.y-brightness2pos();
	M5.Lcd.pushImage(Posxy_s.x,Posxy_s.y,Sizexy_s.x,Sizexy_s.y,(m5gfx::rgb565_t*)cursor_pu16);
}

#define GUI_BRIGHTNESS_MAX 255
#define GUI_BRIGHTNESS_MIN 64
/*
 * -----------------
 * brightness2pos
 * -----------------
 * return brightness into [0,255]
 * -----------------
 */
int16_t Gui_slider::brightness2pos()
{
	int16_t pos_i16;
	pos_i16=((int16_t)M5.Lcd.getBrightness()-(int16_t)GUI_BRIGHTNESS_MIN)*(int16_t)height_u16/(int16_t)(GUI_BRIGHTNESS_MAX-GUI_BRIGHTNESS_MIN);
	if (pos_i16<0)pos_i16=0;
	if (pos_i16>height_u16)pos_i16=height_u16;
	return pos_i16;
}


void Gui_slider::pos2brightness(int16_t posy_i16)
{
	uint8_t br_u8;
	if (posy_i16<0)posy_i16=0;
	if (posy_i16>height_u16) posy_i16=height_u16;
	br_u8=posy_i16*(GUI_BRIGHTNESS_MAX-GUI_BRIGHTNESS_MIN)/height_u16+GUI_BRIGHTNESS_MIN;
	M5.Lcd.setBrightness(br_u8);
}


void Gui_slider::begin()
{
	uint16_t level_u16=0;
	//Get brigthness and convert into level
	level_u16=brightness2pos();
	if (level_u16>height_u16) level_u16=height_u16;
	Posxy_s.y=Origxy_s.y-level_u16;
}


bool Gui_slider::select_cursor(){
	bool rtn_b=false;
	if ((Posxy_s.x+GUI_TOUCH_MARGIN_X<Gui::tp->x)&&(Gui::tp->x<Posxy_s.x-GUI_TOUCH_MARGIN_X+Sizexy_s.x)&&
			(Posxy_s.y+GUI_TOUCH_MARGIN_Y<Gui::tp->y)&&(Gui::tp->y<Posxy_s.y-GUI_TOUCH_MARGIN_Y+Sizexy_s.y)){
		rtn_b=true;
	}
	return rtn_b;
}

bool Gui_slider::select_rail(){
	bool rtn_b=false;
	if ((Posxy_s.x+GUI_TOUCH_MARGIN_X<Gui::tp->x)&&(Gui::tp->x<Posxy_s.x-GUI_TOUCH_MARGIN_X+Sizexy_s.x)){
		rtn_b=true;
	}
	return rtn_b;
}


void Gui_slider::get_touch(gui_touch_e gui_touch_e)
{
	if (gui_touch_e>=GUI_JUST_TOUCH){
		if (select_rail()&&(gui_touch_e>=GUI_JUST_TOUCH)){
			int16_t posy_i16;
			Speaker.play_click();
			posy_i16=Origxy_s.y-Gui::tp->y;
			if (posy_i16<0) posy_i16=0;
			if (posy_i16>height_u16) posy_i16=height_u16;
			draw(posy_i16);
			pos2brightness(posy_i16);
		}
	}
}

/*
 * ===========================
 * Ais_display
 * ---------------------------
 * cf #include "lgfx_fonts.hpp" for dreaw font
 * ---------------------------
 */
M5Canvas2 Ais_display::gps;
M5Canvas2 Ais_display::position;
M5Canvas2 Ais_display::date;
M5Canvas2 Ais_display::target;
M5Canvas2 Ais_display::vessels1;
M5Canvas2 Ais_display::vessels2;
M5Canvas2 Ais_display::alert;

Ais_display::Ais_display(){}
void Ais_display::begin(){
	gps.setPsram(true);
	gps.createSprite(DISPLAY_GPS_SX,DISPLAY_GPS_SY);
	gps.clear(DISPLAY_GPS_BK_COLOR);
	gps.setFont(&FONT12PT);
	gps.setTextSize(1.05,1.6);
	gps.setTextColor(TFT_WHITE,DISPLAY_GPS_BK_COLOR);
	gps.setTextWrap(false,false);

	position.setPsram(true);
	position.createSprite(DISPLAY_POSITION_SX,DISPLAY_POSITION_SY);
	position.clear(DISPLAY_POSITION_BK_COLOR);
	position.setFont(&FONT12PT);
	position.setTextSize(0.7,1.0);
	position.setTextColor(TFT_WHITE,DISPLAY_POSITION_BK_COLOR);
	position.setTextWrap(false,false);

	date.setPsram(true);
	date.createSprite(DISPLAY_DATE_SX,DISPLAY_DATE_SY);
	date.clear(DISPLAY_DATE_BK_COLOR);
	date.setFont(&FONT12PT);
	date.setTextSize(0.7,1.0);
	date.setTextColor(TFT_WHITE,DISPLAY_DATE_BK_COLOR);
	date.setTextWrap(false,false);

	target.setPsram(true);
	target.createSprite(DISPLAY_TARGET_SX,DISPLAY_TARGET_SY);
	target.clear(TFT_WHITE);
	target.setFont(&FONT12PT);
	target.setTextSize(0.9,0.9);
	target.setTextColor(TFT_WHITE,DISPLAY_GOLD);
	target.setTextWrap(false,false);

	vessels1.setPsram(true);
	vessels1.createSprite(DISPLAY_VESSELS1_SX,DISPLAY_VESSELS1_SY);
	vessels1.clear(TFT_WHITE);
	//vessels1.setFont(&FreeSansBold12pt7bmod);
	vessels1.setFont(&FONT12PT);
	vessels1.setTextSize(0.9,1);
	vessels1.setTextWrap(false,false);

	vessels2.setPsram(true);
	vessels2.createSprite(DISPLAY_VESSELS2_SX,DISPLAY_VESSELS2_SY);
	vessels2.clear(TFT_WHITE);
	//vessels2.setFont(&FreeSansBold12pt7bmod);
	vessels2.setFont(&FONT12PT);
	vessels2.setTextSize(0.9,1);
	vessels2.setTextWrap(false,false);

	alert.setPsram(true);
	alert.createSprite(DISPLAY_ALERT_SX,DISPLAY_ALERT_SY);
	alert.clear(TFT_WHITE);
	//alert.setFont(&FreeSansBold12pt7bmod);
	alert.setFont(&FONT12PT);
	//alert.setTextSize(0.6,1);
	alert.setTextSize(0.7,1);
	alert.setTextWrap(false,false);
}

void Ais_display::draw_gps_values(float speed_f32,float cog_f32)
{
	gps.clear(DISPLAY_GPS_BK_COLOR);
	gps.setCursor(4,1);
	gps.printf("%4.1fkt%03d",speed_f32,(uint16_t)cog_f32);// cog_f32 in [0;360]
	M5.Lcd.pushImage(DISPLAY_GPS_POSX,DISPLAY_GPS_POSY,DISPLAY_GPS_SX,DISPLAY_GPS_SY,(m5gfx::rgb565_t*)gps.getBuffer());
}

void Ais_display::draw_pos_values(int32_t lon_d, int32_t lat_d)
{
	position.clear(DISPLAY_GPS_BK_COLOR);
	position.setCursor(4,1);
	position.printf("%+07.2f%+06.2f",(float)lon_d/LAT_LONG_SCALE,(float)lat_d/LAT_LONG_SCALE);
	M5.Lcd.pushImage(DISPLAY_POSITION_POSX,DISPLAY_POSITION_POSY,DISPLAY_POSITION_SX,DISPLAY_POSITION_SY,(m5gfx::rgb565_t*)position.getBuffer());
}

void Ais_display::draw_date_values(int32_t hour_i32, int32_t mn_i32,int32_t s_i32)
{
	date.clear(DISPLAY_GPS_BK_COLOR);
	date.setCursor(4,1);
	date.printf("%02d:%02d:%02d",hour_i32,mn_i32,s_i32);
	M5.Lcd.pushImage(DISPLAY_DATE_POSX,DISPLAY_DATE_POSY,DISPLAY_DATE_SX,DISPLAY_DATE_SY,(m5gfx::rgb565_t*)date.getBuffer());
}

void Ais_display::draw_target_vessels(char label,int16_t posx_i16,int16_t posy_i16,float head_d32,uint16_t color_u16)
{
	//target.draw_char_rot_at(DISPLAY_TARGET_CENTER_X+posx_i16,DISPLAY_TARGET_CENTER_Y-posy_i16,label,&FreeSansBold12pt7bmod,head_d32,color_u16);
	target.draw_char_rot_at(DISPLAY_TARGET_CENTER_X+posx_i16,DISPLAY_TARGET_CENTER_Y-posy_i16,label,&FONT24PT,head_d32,color_u16);
}

void Ais_display::draw_target_scale(uint16_t scale_u16)
{
	target.setCursor(184,200);
	target.printf("%dN",scale_u16);
}


/*
 * -----------------------
 * draw_vessels_one_alert
 * -----------------------
 */
uint32_t Ais_display::time_to_cpa_mn_au32[DISPLAY_ALERT_SZ];

void Ais_display::draw_vessels_one_alert(int16_t id_i16,char label,uint32_t time_to_cpa_mn_u32,char * name_pc,uint16_t color_u16)
{
	if (id_i16<DISPLAY_ALERT_SZ){
		time_to_cpa_mn_au32[id_i16]=time_to_cpa_mn_u32;
	}else {
		/*
		 * Get alert of max time_to_cpa_mn_u32
		 */
		uint32_t max_mn_u32=time_to_cpa_mn_u32;
		id_i16=-1;
		for (int16_t i=0;i<DISPLAY_ALERT_SZ;i++){
			if (max_mn_u32<time_to_cpa_mn_au32[i]){
				id_i16=i;
				max_mn_u32=time_to_cpa_mn_au32[i];
			}
			if (id_i16==-1){
				LOG_D("Drop alert");
				return;
			}
		}
		/*
		 * Erase old id_i16 of max time_to_cpa_mn_u32
		 */
		alert.setCursor(1,(7+26*id_i16));
		alert.setTextColor(TFT_WHITE,TFT_WHITE);
		alert.printf("%-20.20s","                             ");
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
	while ((i>=0)&&(name_pc[i]==(char)ASCII_SPACE)){
		name_pc[i]=0;
		i--;
	}
	alert.setCursor(1,(7+26*id_i16));
	alert.setTextColor(color_u16,TFT_WHITE);
	alert.printf("%c-",label);
	alert.setTextColor(DISPLAY_RED,TFT_WHITE);
	alert.printf("%2d\'",time_to_cpa_mn_u32);
	alert.setTextColor(DISPLAY_DARKGREY,TFT_WHITE);
	//alert.printf("%-20.20s",name_pc);
	alert.printf("%-12.12s",name_pc);// left alignment 14 char min - truncated 14 char

	//M5.Lcd.pushImage(DISPLAY_ALERT_POSX,DISPLAY_ALERT_POSY,DISPLAY_ALERT_SX,DISPLAY_ALERT_SY,(m5gfx::rgb565_t*)alert.getBuffer());
}

void Ais_display::vessels_push()
{
	M5.Lcd.pushImage(DISPLAY_VESSELS1_POSX,DISPLAY_VESSELS1_POSY,DISPLAY_VESSELS1_SX,DISPLAY_VESSELS1_SY,(m5gfx::rgb565_t*)vessels1.getBuffer());
	M5.Lcd.pushImage(DISPLAY_VESSELS2_POSX,DISPLAY_VESSELS2_POSY,DISPLAY_VESSELS2_SX,DISPLAY_VESSELS2_SY,(m5gfx::rgb565_t*)vessels2.getBuffer());
}

void Ais_display::alerts_push()
{
	M5.Lcd.pushImage(DISPLAY_ALERT_POSX,DISPLAY_ALERT_POSY,DISPLAY_ALERT_SX,DISPLAY_ALERT_SY,(m5gfx::rgb565_t*)alert.getBuffer());
}

/*
 * -----------------------
 * draw_vessels_captions
 * -----------------------
 *
 */
void Ais_display::draw_vessels_one_vessel(vessels_canvas_e canvas_e,int16_t id_i16,char label,uint32_t speed_kt,uint16_t color_u16,uint16_t color_spd_u16,bool old_b)
{
	if (canvas_e==VESSELS_CANVAS1){
		vessels1.setTextColor(color_u16,TFT_WHITE);
		vessels1.setCursor(6,(7+26*id_i16));
		if (old_b) vessels1.printf("%c! ",label); else vessels1.printf("%c- ",label);
		vessels1.setTextColor(color_spd_u16,TFT_WHITE);

		/*
		 * %2.2d: 2 digits in 2 char length XX
		 */
		vessels1.printf("%2.2d",(speed_kt+5)/10);
		//M5.Lcd.pushImage(DISPLAY_VESSELS1_POSX,DISPLAY_VESSELS1_POSY,DISPLAY_VESSELS1_SX,DISPLAY_VESSELS1_SY,(m5gfx::rgb565_t*)vessels1.getBuffer());
	}else {
		vessels2.setTextColor(color_u16,TFT_WHITE);
		vessels2.setCursor(6,(7+26*id_i16));
		if (old_b) vessels2.printf("%c! ",label); else vessels2.printf("%c- ",label);
		vessels2.setTextColor(color_spd_u16,TFT_WHITE);
		vessels2.printf("%2.2d",(speed_kt+5)/10);
		//M5.Lcd.pushImage(DISPLAY_VESSELS2_POSX,DISPLAY_VESSELS2_POSY,DISPLAY_VESSELS2_SX,DISPLAY_VESSELS2_SY,(m5gfx::rgb565_t*)vessels2.getBuffer());
	}
}


/*
 * -----------------------
 * display_caption
 * -----------------------
 */
void Ais_display::draw_vessels(int16_t id_u16,char label,uint32_t speed_kt,uint16_t color_u16,uint16_t color_spd_u16, bool old_b)
{
	vessels_canvas_e canvas_e;
	if (id_u16<(DISPLAY_VESSELS_NBR>>1)){
		canvas_e=VESSELS_CANVAS1;
		draw_vessels_one_vessel(canvas_e,id_u16,label,speed_kt,color_u16,color_spd_u16,old_b);
	}else if (id_u16<DISPLAY_VESSELS_NBR){
		canvas_e=VESSELS_CANVAS2;
		draw_vessels_one_vessel(canvas_e,id_u16-5,label,speed_kt,color_u16,color_spd_u16,old_b);
	}
}


/*
 * -----------------------
 * M5Canvas2
 * -----------------------
 */

/*
 * -----------------------
 * draw_char_rot_at
 * -----------------------
 * Draw char with any rotation,
 * center on COG
 * -----------------------
 */
void M5Canvas2::draw_char_rot_at(int16_t posx_i16,int16_t posy_i16,char c,const GFXfont *Font,float rot_d32,uint16_t color_u16)
{
	int16_t xshift_i16,yshift_i16;
	//setTextColor(GUI_TARGET_VSL_COLOR);
	//setFont(Font);
	uint8_t first=Font->first;
	GFXglyph *glyph=(Font->glyph)+(uint8_t)(c-first);
	uint8_t gw=glyph->width;
	uint8_t gh=glyph->height;
	int8_t xo=glyph->xOffset;
	int8_t yo=glyph->yOffset;
	uint16_t bo=glyph->bitmapOffset;
	uint8_t *bitmap=Font->bitmap;
	int16_t xx, yy, bits = 0, bit = 0;
	int16_t ic,jc;
	int16_t px1,py1;
	float deg_r=rot_d32*(float)M_PI/(float)180.0;
	float res_d32;

	xshift_i16=xo+gw/2;
	yshift_i16=yo+gh/2;
	LOG_V("xshift_i16(%d) - yshift_i16(%d)",xshift_i16,yshift_i16);

	for (yy=0;yy<gh;yy++) {
		for (xx=0;xx<gw;xx++) {
			if (!(bit++ & 7)) {
				bits = pgm_read_byte(&bitmap[bo++]);
			}
			if (bits & 0x80) {
				ic=xo+xx-xshift_i16;
				jc=yo+yy-yshift_i16;
				/*
				 * Matrix of rotation
				 * 	-Reference at (0x)
				 * 	-Rotation in direction of (Oy)
				 *     cos(t) -sin(t)
				 *     sin(t) costx)
				 */
				res_d32=((float)ic*cosf(deg_r)-(float)jc*sinf(deg_r))/2+(float)posx_i16;
				px1=(int16_t)(res_d32);
				res_d32=((float)ic*sinf(deg_r)+(float)jc*cosf(deg_r))/2+(float)posy_i16;
				py1=(int16_t)(res_d32);
				drawPixel(px1,py1,color_u16);
			}
			bits <<= 1;
		}
	}
};


/*
 * -----------------------
 * draw_char_rot90_at
 * -----------------------
 * Draw char with rotation of 0, 90, 180, 270°
 * by switching x and y.
 * -----------------------
 */
void M5Canvas2::draw_char_rot90_at(int16_t posx_i16,int16_t posy_i16,char c,const GFXfont *Font,gui_target_rotation_e rot_e,uint16_t color_u16)
{
	uint8_t first=Font->first;
	GFXglyph *glyph=(Font->glyph)+(uint8_t)(c-first);
	uint8_t gw=glyph->width;
	uint8_t gh=glyph->height;
	int8_t xo=glyph->xOffset;
	int8_t yo=glyph->yOffset;
	uint16_t bo=glyph->bitmapOffset;
	uint8_t *bitmap=Font->bitmap;
	int16_t xx,yy,bits=0,bit=0;
	int16_t ic,jc;
	int16_t px,py;

	for (yy=0;yy<gh;yy++) {
		for (xx=0;xx<gw;xx++) {
			if (!(bit++ & 7)) {
				bits = pgm_read_byte(&bitmap[bo++]);
			}
			if (bits & 0x80) {
				ic=xo+xx;
				jc=yo+yy;
				switch (rot_e){
				case GUI_TGT_ROTATE_0:
					px=ic;
					py=jc;
					break;
				case GUI_TGT_ROTATE_90:
					px=-jc;
					py=ic;
					break;
				case GUI_TGT_ROTATE_180:
					px=-ic;
					py=-jc;
					break;
				case GUI_TGT_ROTATE_270:
					px=jc;
					py=-ic;
					break;
				default:
					px=ic;
					py=jc;
				}
				drawPixel(posx_i16+px,posy_i16+py,color_u16);
			}
			bits <<= 1;
		}
	}
};



