/**
 * ********************
 *  gui.h
 *  www.DoNovae.com
 *  Herve Bailly
 * *******************
 */

#ifndef __GUI_H
#define __GUI_H

#include <stdint.h>
//#include <M5Unified.h>
//#include <M5GFX.h>
#include "gui_spk.h"
#include "ais.h"



/*
 * ==========================================
 * Extern image
 * ==========================================
 */
extern const uint16_t GUI_TARGET_BKGND[];
extern const uint16_t GUI_VESSELS_BKGND[];
extern const uint16_t GUI_SETTINGS_BKGND[];
extern const uint16_t GUI_BRIGHT_CURSOR[];
extern const uint16_t GUI_BRIGHT_MASK[];
extern const uint16_t GUI_WIFI_OFF[];
extern const uint16_t GUI_WIFI_ON[];
extern const uint16_t GUI_WIFI_HS[];
extern const uint16_t GUI_SPEAKER_WIFI_OFF[];
extern const uint16_t GUI_SPEAKER_ON[];
extern const uint16_t GUI_TARGET[];
extern const uint16_t GUI_BELL_ON[];
extern const uint16_t GUI_BELL_OFF[];
extern const uint16_t GUI_GPS0[];
extern const uint16_t GUI_GPS1[];
extern const uint16_t GUI_GPS2[];
extern const uint16_t GUI_GPS3[];

/*
 * ==========================================
 * Defines
 * ==========================================
 */
#define GUI_TOUCH_AREA_TARGET_NB 6
#define GUI_TOUCH_AREA_VESSELS_NB 3
#define GUI_TOUCH_AREA_SETTINGS_NB 4
#define GUI_SPK_POSXY {290,0}
#define GUI_WIFI_POSXY {260,0}
#define GUI_LOOP_START 1000
#define GUI_SLIDER_HEIGHT 178


/*
 * Colors
 */
#define DISPLAY_GREEN 0x298E
#define DISPLAY_GOLD 0x2F9C
#define DISPLAY_NAVY 0x1F00
#define DISPLAY_RED 0x00F8
#define DISPLAY_DARKGREY 0x494A


/*
 * GPS
 */
#define DISPLAY_GPS_POSX 8
#define DISPLAY_GPS_POSY 5
#define DISPLAY_GPS_SX 133
#define DISPLAY_GPS_SY 30
#define DISPLAY_GPS_BK_COLOR DISPLAY_DARKGREY

/*
 * Position
 */
#define DISPLAY_POSITION_POSX (8+4)
#define DISPLAY_POSITION_POSY (5+54)
#define DISPLAY_POSITION_SX 133
#define DISPLAY_POSITION_SY 30
#define DISPLAY_POSITION_BK_COLOR DISPLAY_DARKGREY


/*
 * Date
 */
#define DISPLAY_DATE_POSX (8+30)
#define DISPLAY_DATE_POSY (5+115)
#define DISPLAY_DATE_SX 100
#define DISPLAY_DATE_SY 30
#define DISPLAY_DATE_BK_COLOR DISPLAY_DARKGREY


/*
 * Target
 */
#define DISPLAY_TARGET_POSX 96
#define DISPLAY_TARGET_POSY 16
#define DISPLAY_TARGET_SX 224
#define DISPLAY_TARGET_SY 224
#define DISPLAY_TARGET_CENTER_X 112
#define DISPLAY_TARGET_CENTER_Y 112
#define DISPLAY_TARGET_R1 37
#define DISPLAY_TARGET_R2 75
#define DISPLAY_TARGET_R3 112
#define DISPLAY_TARGET_R32 56
#define GUI_TARGET_VSL_COLOR DISPLAY_NAVY
#define GUI_TARGET_ALERT_COLOR DISPLAY_RED


/*
 * Vessels
 */
#define DISPLAY_VESSELS2_POSX 245
#define DISPLAY_VESSELS2_POSY 60
#define DISPLAY_VESSELS2_SX 70
#define DISPLAY_VESSELS2_SY 136

#define DISPLAY_VESSELS1_POSX 168
#define DISPLAY_VESSELS1_POSY 60
#define DISPLAY_VESSELS1_SX 70
#define DISPLAY_VESSELS1_SY 136

#define DISPLAY_VESSELS_NBR 10

/*
 * Alert
 */
#define DISPLAY_ALERT_POSX 5
#define DISPLAY_ALERT_POSY 60
#define DISPLAY_ALERT_SX 155
#define DISPLAY_ALERT_SY 136

/*
 * ==========================================
 * Types
 * ==========================================
 */

/*
 * ----------------
 * point_t
 * ----------------
 */
typedef struct
{
	uint16_t x;
	uint16_t y;
} gui_point_t;

typedef struct
{
	gui_point_t p0;
	gui_point_t p1;
} gui_touch_area_t;


/*
 * ----------------
 * Target
 * ----------------
 */
typedef enum {
	GUI_TGT_NONE=-1,
	GUI_TGT_VSL=0,
	GUI_TGT_SET=1,
	GUI_TGT_SPK=2,
	GUI_TGT_CROSS=3,
	GUI_TGT_ZIN=4,
	GUI_TGT_ZOUT=5
}gui_target_touch_e;



/*
 * ----------------
 * VESSELS
 * ----------------
 */
typedef enum {
	GUI_VSL_NONE=-1,
	GUI_VSL_TGT=0,
	GUI_VSL_SET=1,
	GUI_VSL_SPK=2
}gui_vessels_touch_e;


/*
 * ----------------
 * SETTINGS
 * ----------------
 */
typedef enum {
	GUI_SET_NONE=-1,
	GUI_SET_TGT=0,
	GUI_SET_VSL=1,
	GUI_SET_SPK=2,
	GUI_SET_WIFI=3
}gui_settings_touch_e;


typedef union {
	gui_target_touch_e tgt;
	gui_vessels_touch_e vsl;
	gui_settings_touch_e set;
	int8_t i8;
}gui_touch_id_t;

typedef enum {
	GUI_TOUCH_NONE=0,
	GUI_JUST_TOUCH=1,
	GUI_TOUCH_HOLD=2
}gui_touch_e;


/*
 * -----------------------
 * State machine
 * -----------------------
 */
typedef enum {
	GUI_SM_START=0,
	GUI_SM_TARGET=1,
	GUI_SM_VESSELS=2,
	GUI_SM_SETTINGS=3
}gui_state_machine_e;

/*
 * -----------------------
 * Wifi animation
 * -----------------------
 */
typedef enum {
	GUI_ANIM_WIFI_ON=0,
	GUI_ANIM_WIFI_OFF=1,
	GUI_ANIM_WIFI_HS=2
}gui_animation_wifi_e;
#define GUI_ANIMATION_WIFI_NB 3

/*
 * -----------------------
 * Speaker animation
 * -----------------------
 */
typedef enum {
	GUI_ANIM_SPEAKER_ON=0,
	GUI_ANIM_SPEAKER_OFF=1
}gui_animation_spk_e;
#define GUI_ANIMATION_SPK_NB 2

/*
 * -----------------------
 * Bell animation
 * -----------------------
 */
typedef enum {
	GUI_ANIM_BELL_ON=0,
	GUI_ANIM_BELL_OFF=1
}gui_animation_bell_e;
#define GUI_ANIMATION_BELL_NB 2

/*
 * -----------------------
 * GPS animation
 * -----------------------
 */
typedef enum {
	GUI_ANIM_GPS0=0,
	GUI_ANIM_GPS1=1,
	GUI_ANIM_GPS2=2,
	GUI_ANIM_GPS3=3,
	GUI_ANIM_GPS4=4,
	GUI_ANIM_GPS5=5,
	GUI_ANIM_GPS6=6
}gui_animation_gps_e;
#define GUI_ANIMATION_GPS_NB 4


/*
 * -----------------------
 * Animation union
 * -----------------------
 */
typedef union {
	gui_animation_wifi_e wifi;
	gui_animation_spk_e spk;
	gui_animation_bell_e bell;
	gui_animation_gps_e gps;
	uint8_t u8;
}gui_animation_t;





/*
 * -----------------------
 * Vessels
 * -----------------------
 */
typedef enum {
	VESSELS_CANNVAS_N0NE=0,
	VESSELS_CANVAS1=1,
	VESSELS_CANVAS2=2
}vessels_canvas_e;



/*
 * ==========================================
 * Classes
 * ==========================================
 */

/*
 * =======================
 * Class Gui
 * -----------------------
 */
class Gui{
public:
	static gui_touch_e touch_update();

	//static m5gfx::touch_point_t prev_tp[2];
	//static m5gfx::touch_point_t tp[2];
private:
	static uint8_t touch_points_u8;
	static uint8_t prev_touch_points_u8;
};

/*
 * =======================
 * Class Gui_page
 * -----------------------
 */
class Gui_page : public Gui
{
public:
	Gui_page(const uint16_t *img_pu16,const gui_touch_area_t *tch_a_p, const uint8_t a_nb_u8);
	~Gui_page();
	static void begin();
	void draw_background();
	int8_t area_selection();

private:
	const uint16_t *bck_gnd_u16;
	const gui_touch_area_t *touch_area_p;
	uint8_t areas_nb_u8;
};

/*
 * =======================
 * Class Gui_animation
 * -----------------------
 */
class Gui_animation
{
public:
	Gui_animation(const gui_point_t *posxy_ps, const gui_point_t *sizesxy_ps, const uint16_t **img_pu16,const uint8_t img_nb_u8);
	~Gui_animation();
	void draw(gui_animation_t id);
private:
	const uint16_t **image_pu16;
	uint8_t image_nb_u8;
	gui_point_t Posxy_s;
	gui_point_t Sizexy_s;
};

/*
 * =======================
 * Class Gui_slider
 * -----------------------
 */
class Gui_slider : public Gui
{
public:
	Gui_slider(const gui_point_t *posxy_ps,uint16_t hgt_u16,const gui_point_t *sizesxy_ps,const uint16_t *cur_pu16,const uint16_t *msk_pu16);
	~Gui_slider();
	void draw(uint16_t h_u16);
	void draw();
	void begin();
	int16_t brightness2pos(int16_t brightness_i16);
	void get_touch(gui_touch_e gui_touch_e);

private:
	bool select_cursor();
	bool select_rail();
	void pos2brightness(int16_t posy_i16);
	int16_t brightness2pos();
	const uint16_t *cursor_pu16;
	const uint16_t *mask_pu16;
	gui_point_t Origxy_s;
	gui_point_t Sizexy_s;
	gui_point_t Posxy_s;
	uint16_t height_u16;
};



/*
 * =======================
 * Class Ais_display
 * -----------------------
 *
 */
#define DISPLAY_ALERT_SZ 5
typedef enum {
	GUI_TGT_ROTATE_0=0,
	GUI_TGT_ROTATE_90=1,
	GUI_TGT_ROTATE_180=2,
	GUI_TGT_ROTATE_270=3
}gui_target_rotation_e;

//class M5Canvas2 :public M5Canvas {
class M5Canvas2 {
public:
	//void draw_char_rot_at(int16_t posx_i16,int16_t posy_i16,char c,const GFXfont *Font,float head_d32,uint16_t color_u16);
	//void draw_char_rot90_at(int16_t posx_i16,int16_t posy_i16,char c,const GFXfont *Font,gui_target_rotation_e rot_e,uint16_t color_u16);
};



class Ais_display
{
public:
	Ais_display();
	static void begin();
	static M5Canvas2 gps;
	static M5Canvas2 position;
	static M5Canvas2 date;
	static M5Canvas2 target;
	static M5Canvas2 vessels1;
	static M5Canvas2 vessels2;
	static M5Canvas2 alert;

	/*
	 * Gps
	 */
	static void draw_gps_values(float speed_d32, float direction_d32);
	static void draw_pos_values(int32_t lon_d, int32_t lat_d);
	static void draw_date_values(int32_t lon_d, int32_t lat_d);
	static void draw_date_values(int32_t hour_i32, int32_t mn_i32,int32_t s_i32);

	/*
	 * Target
	 */
	static void draw_target_vessels(char label,int16_t posx_i16,int16_t posy_i16,float head_d32,uint16_t color_u16);
	static void draw_target_scale(uint16_t scale_u16);

	/*
	 * Vessels
	 */
	static void draw_vessels_one_alert(int16_t id_i16,char label,uint32_t time_to_cpa_mn_u32,char * name_pc,uint16_t color_u16);
	static void draw_vessels_one_vessel(vessels_canvas_e canvas_e,int16_t id_u16,char label,uint32_t speed_kt,uint16_t color_u16,uint16_t color_spd_u16,bool old_b);
	static void draw_vessels(int16_t id_u16,char label,uint32_t speed_kt,uint16_t color_u16, uint16_t color_spd_u16,bool old_b);
	static inline void draw_vessels_clear(){/* TODO vessels1.clear(TFT_WHITE);vessels2.clear(TFT_WHITE);*/};
	static inline void draw_alerts_clear(){/* TODO alert.clear(TFT_WHITE);*/};
	static void vessels_push();
	static void alerts_push();
	static uint8_t alerts_cpt;

private:
	static uint32_t time_to_cpa_mn_au32[DISPLAY_ALERT_SZ];
};






/*
 * ==========================================
 * Extern image
 * ==========================================
 */
extern const gui_touch_area_t GUI_TOUCH_AREAS_TGT[GUI_TOUCH_AREA_TARGET_NB];
extern const gui_touch_area_t GUI_TOUCH_AREAS_VSL[GUI_TOUCH_AREA_VESSELS_NB];
extern const gui_touch_area_t GUI_TOUCH_AREAS_SET[GUI_TOUCH_AREA_SETTINGS_NB];
extern gui_state_machine_e Gui_state_s;
extern Gui_spk Speaker;
extern const uint16_t *GUI_ANIMATION_WIFI_A[GUI_ANIMATION_WIFI_NB];
extern const gui_point_t GUI_ANIMATION_WIFI_XY;
extern const gui_point_t GUI_ANIMATION_WIFI_SZ;
extern const uint16_t *GUI_ANIMATION_SPK_A[GUI_ANIMATION_SPK_NB];
extern const gui_point_t GUI_ANIMATION_SPK_XY;
extern const gui_point_t GUI_ANIMATION_SPK_SZ;
extern const uint16_t *GUI_ANIMATION_GPS_A[GUI_ANIMATION_GPS_NB];
extern const gui_point_t GUI_ANIMATION_GPS_XY;
extern const gui_point_t GUI_ANIMATION_GPS_SZ;
extern const uint16_t *GUI_ANIMATION_BELL_A[GUI_ANIMATION_BELL_NB];
extern const gui_point_t GUI_ANIMATION_BELL_XY;
extern const gui_point_t GUI_ANIMATION_BELL_SZ;
extern const gui_point_t GUI_SLIDER_XY;
extern const gui_point_t GUI_SLIDER_CURSOR_SZ;

#endif //__GUI_H

