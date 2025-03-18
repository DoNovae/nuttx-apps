/**
 * =====================================
 * doais_gui.cxx
 * -------------------------------------
 *  AIS DoNovae
 *  www.DoNovae.com
 *  Herve Bailly
 * =====================================
 *
 */

#include <stdio.h>
#include <lvgl/src/misc/lv_anim.h>
#include  "types.h"
#include  "gps.h"
#include "ais_channels.h"
#include "ais_monitoring.h"

extern "C" {
#include "display.h"
}


/*
 * ===================
 * Defines
 * -------------------
 */



/*
 * ===================
 * Extern
 * -------------------
 */
extern StationData Station_data_s; // Cf doais_db_update
extern Ais_monitoring Monitoring;// Cf doais_db_update
extern gps_data_t Gps_info_s; // Cf doais_db_update
extern FAR mutex_t Gps_data_mutex_s;// Cf ais_main.c




/*
 * ===================
 * Globals
 * -------------------
 */
gui_animation_t Gui_animation_s;

/*
 * ===================
 * Static prototypes
 * -------------------
 */
void lv_display_load(Display_id_e id_e);
void lv_display_init(void);
void lv_display_next(void);
void lv_display_prev(void);
void lv_displays_update(void);


/*
 * ===============================
 *           FUNCTIONS
 * ===============================
 */

/*
 * ===================
 * lv_display_init
 * -------------------
 */

void lv_display_init()
{
	/*
	 * Displays
	 */
	Displays_as[DISPLAY_TARGET_ID].display_ps=lv_obj_create(NULL);
	Displays_as[DISPLAY_TARGET_ID].init_cb=lv_target_display;
	Displays_as[DISPLAY_TARGET_ID].update_cb=lv_update;
	Displays_as[DISPLAY_VESSELS_ID].display_ps=lv_obj_create(NULL);
	Displays_as[DISPLAY_VESSELS_ID].init_cb=lv_vessels_display;
	Displays_as[DISPLAY_VESSELS_ID].update_cb=lv_update;
	Displays_as[DISPLAY_SETTINGS_ID].display_ps=lv_obj_create(NULL);
	Displays_as[DISPLAY_SETTINGS_ID].init_cb=lv_settings_display;
	Displays_as[DISPLAY_SETTINGS_ID].update_cb=lv_update;
	Display_id=DISPLAY_TARGET_ID;

	/*
	 * Buttons styles
	 */
	lv_button_init();

	/*
	 * Gui animation
	 */
	Gui_animation_s.wifi=GUI_ANIM_WIFI_ON; // configuration_store.c
	Gui_animation_s.spk=GUI_ANIM_SPEAKER_ON;
	Gui_animation_s.bell=GUI_ANIM_BELL_OFF;
	Gui_animation_s.gps=GUI_ANIM_GPS0;

	lv_display_load(Display_id);
}

/*
 * ===================
 * lv_display_inits
 * -------------------
 */
void lv_display_load(Display_id_e id_e)
{
	// Remove all the children of display_ps
	lv_obj_clean(Displays_as[id_e].display_ps);
	lv_scr_load(Displays_as[id_e].display_ps);
	Displays_as[id_e].init_cb(Displays_as[id_e].display_ps);
}

/*
 * ===================
 * lv_display_next
 * -------------------
 */
void lv_display_next(void)
{
	int8_t id_i8=(uint8_t)Display_id;
	id_i8=(id_i8==(DISPLAY_NBR-1))?0:id_i8+1;
	Display_id=(Display_id_e)id_i8;
}

void lv_display_prev(void)
{
	int8_t id_i8=(uint8_t)Display_id;
	id_i8=(id_i8==0)?(DISPLAY_NBR-1):id_i8-1;
	Display_id=(Display_id_e)id_i8;
}


/*
 * ===================
 * lv_displays_update
 * -------------------
 */
void lv_displays_update(void)
{
	nxmutex_lock(&Gps_data_mutex_s);
	/*
	 * Update gps status
	 */
	Gui_animation_s.gps=(gui_animation_gps_e)Gps_info_s.fix;
	lv_update_status_gps();

	switch(Display_id ){
	case DISPLAY_TARGET_ID:
		lv_update_speed((float)Gps_info_s.speed_kt,(uint16_t)Gps_info_s.heading_d);
		lv_target_update();
		break;
	case DISPLAY_VESSELS_ID:
		break;
	case DISPLAY_SETTINGS_ID:
		lv_settings_update_pos(Gps_info_s.lon_d,Gps_info_s.lat_d);
		lv_settings_update_time(Gps_info_s.utc_s.tm_hour,Gps_info_s.utc_s.tm_min,Gps_info_s.utc_s.tm_sec);
		//LOG_D("update_gps_info: tm_hour(%d) - tm_min(%d) - tm_sec(%d)",Gps_info_s.utc_s.tm_hour,Gps_info_s.utc_s.tm_min,Gps_info_s.utc_s.tm_sec);
		break;
	case DISPLAY_CHARTS_ID:
		break;
	default:
		LOG_E("lv_displays_update: ERR Display_id(%d) unknown",Display_id);
		break;
	}
	nxmutex_unlock(&Gps_data_mutex_s);
}

