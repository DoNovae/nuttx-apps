#include <inttypes.h>
#include "ais_channels.h"
#include "gps.h"
#include "ais_messages.h"
#include "circular_queue.h"
#include "gui.h"
#include "gui_spk.h"
#include "gmath.h"

#ifndef __AIS_MONITORING_H__
#define __AIS_MONITORING_H__



/*
 * -----------------------
 * AIS monitoring
 * -----------------------
 *
 * -----------------------
 */



#define MONITORING_DISPLAY_STEPS_NB 3
#define MONITORING_DISPLAY_SPEED_MIN_KT 0

typedef enum {
	MONITORING_MIN_TIME_CPA_RST=1000,
	MONITORING_MIN_TIME_CPA_5=5,
	MONITORING_MIN_TIME_CPA_10=10,
	MONITORING_MIN_TIME_CPA_15=15,
	MONITORING_MIN_TIME_CPA_20=20,
	MONITORING_MIN_TIME_CPA_30=30
} monitoring_min_time_cpa_e;


typedef enum {
	MONOTORING_CPA_WARN_10THNM=3,
	MONOTORING_DISPLAY_TARGET_STEP_NM=2
}monitoring_range_e;

typedef enum {
	MONOTORING_REFRESH_NONE=0,
	MONOTORING_REFRESH_PERIODIC=1,
	MONOTORING_REFRESH_PKT=2,
	MONOTORING_REFRESH_ASAP=3
}monitoring_refresh_e;


/*
 * MONOTORING_TIMER_8MN 96
 * MONOTORING_TIMER_10MN 120
 * MONOTORING_TIMER_30MN 360
 */

typedef enum {
	MONOTORING_TIMER_MS=1000,
	MONOTORING_LOST_TARGET_MN=8,
	MONOTORING_MAX_TCPA_MN=30,
	MONOTORING_TICKS_1MN=12,
	MONOTORING_TICKS_10MN=120,
	MONOTORING_TICKS_30MN=360
} monitoring_timer_e;

typedef enum {
	MONITORING_STATUS_NONE=0,
	MONITORING_STATUS_ALERT=1,
	MONITORING_STATUS_ERROR=2
} monitoriring_status_e;

typedef enum {
	MONITORING_DISPLAY_STATUS_NONE=0,
	MONITORING_DISPLAY_STATUS_ALERT=1
}monitoriring_display_status_e;

typedef enum {
	MONITORING_SYS_STATUS_NONE=0,
	MONITORING_SYS_STATUS_GPS_ALERT=2,
	MONITORING_GPS_STATUS_LABELS_ALERT=4
}monitoriring_sys_status_e;

/*
 * AIS_CHAINED_LIST_MAX_SZ
 */
//#define AIS_CHAINED_LIST_MAX_SZ 4
#define AIS_CHAINED_LIST_MAX_SZ 32
//#define AIS_CHAINED_LABEL_MAX_SZ 3
#define AIS_CHAINED_LABEL_MAX_SZ 15

class Monitor_data
{
public:
	char shipname[STATION_SHIP_NAME_SZ];
	double range_nm; // Calculated
	double cpa_nm; // Calculated
	nmeaPOS cpa_pos_nm;
	float rel_heading_d;
	uint32_t time_to_cpa_mn_u32; // Calculated in minutes
	uint32_t mmsi;
	uint32_t lat_d; // Minutes/10000 Cf AISMessage
	uint32_t lon_d; // Minutes/10000 Cf AISMessage
	uint32_t cog_d; // 1/10 degree Cf AISMessage
	uint32_t speed_kt; // 1/10 knot Cf AISMessage
	uint8_t shiptype;
	uint32_t ticks_u32; // Date from creation
	uint32_t heading_d; // Cap compass - Cf AISMessage
	monitoriring_status_e status_e;
	char label; //in [0-9]

	/*
	 * Reset except label and mmsi
	 */
	inline void reset(){
		shiptype=SHIPTYPE_NONE;
		//strncpy(shipname,"@@@@@@@@@@@@@@@@@@@@@@@@@@@",STATION_SHIP_NAME_SZ);
		//shipname[STATION_SHIP_NAME_SZ-1]=0;
		shipname[0]=0;
		lat_d=lon_d=0;
		cog_d=speed_kt=ticks_u32=heading_d=0;
		time_to_cpa_mn_u32=0;
		status_e=MONITORING_STATUS_NONE;
		range_nm=(double)0.0;
		cpa_nm=(double)0.0;
	}

	inline void print(){
		LOG_D("label('%c') - shipname(%s) - mmsi(%d) - shiptype(%d) - ticks_u32(%d) - lon_d(%.1f) - lat_d(%.1f) - cog_d(%d) - heading_d(%d) - speed_kt(%d) - range_nm(%.1f) - status_e(%d) - cpa_nm(%.1f) - time_to_cpa_mn_u32(%d)",
				label,shipname,mmsi,shiptype,ticks_u32,AISMessage::lon_d2double(lon_d),AISMessage::lat_d2double(lat_d),cog_d/10,heading_d,speed_kt/10,range_nm,(uint8_t)status_e,cpa_nm,time_to_cpa_mn_u32);
	}
	/*
	 * Closest point of approach
	 */
	void cpa();
};

/*
 * ----------------------
 * Ais_chained_list
 * ----------------------
 */

typedef enum {
	CHAINED_LIST_TEST_MMSI_EQ=0,
	CHAINED_LIST_TEST_LABEL_EQ=1,
	CHAINED_LIST_TEST_LABEL_GT=2,
	CHAINED_LIST_TEST_LABEL_LT=3,
	CHAINED_LIST_TEST_RANGE_GT=4,
	CHAINED_LIST_TEST_RANGE_LT=5
}chained_list_test_e;


typedef enum {
	AIS_MONITORING_Z0=0,
	AIS_MONITORING_Z1=1,
	AIS_MONITORING_Z2=2
}ais_monitoring_zoom_e;


/*
 * =========================
 *
 * -------------------------
 *   Max vessels number: AIS_CHAINED_LIST_MAX_SZ
 *   	If max, replace the vessel of highest range
 *   Vessels
 *   	Display first vessels in alert
 *   	Then display the first remaining vessels
 *   Label ?
 *   	When release a label different from ?, If queue is empty, re-name
 *   		Then an vessel labeled '?'
 *   		TODO : First vessel in alert labeled '?'
 * -------------------------
 */
class Ais_monitoring : public Chained_list<Monitor_data> {
public:
	struct settings_t {
		uint32_t cpa_warn_10thnm_u32;
		uint32_t lost_target_mn_u32;
		uint32_t tcpa_max_mn_u32;
		uint32_t lost_target_ticks_u32;
		uint32_t display_target_step_nm_u32;
		uint32_t speed_min_kt_u32;
		uint32_t gps_bauds_u32;
	};
	static settings_t settings_s;
	static monitoriring_display_status_e display_status;
	static uint8_t sys_status_u8;
	static uint32_t min_time_to_cpa_mn_u32;
	static monitoring_refresh_e refresh_b;
	static bool is_pkt_b;
	static bool is_cross_b;

	Ais_monitoring(uint16_t mx_u16, uint16_t mx_label_u16);
	bool test(const uint8_t type_u8,const Monitor_data* T1_p,const Monitor_data* T2_p)override;

	void update_from_msg(const AISMessage *msg_p,Monitor_data *data_p);
	bool update_dates_range(uint32_t ticks_u32);
	bool update(const AISMessage *msg_p,Monitor_data** monit_p,uint8_t ch_u8);
	bool update_range(Monitor_data *monit_p);
	void print();
	bool is_mmsi(const uint32_t mmsi_u32,Monitor_data** _p);
	bool is_mmsi(const uint32_t mmsi_u32,Monitor_data** _p,List<Monitor_data> *prev_p,List<Monitor_data> *cur_p);
	bool new_mmsi(const uint32_t mmsi_u32,Monitor_data** _p,double range_d64);
	void max_range(Monitor_data** _p,double range0_d64);
	void min_range(Monitor_data** _p,double *range0_d64_p,uint32_t * mmsi_p);
	bool replace_default_label(char label);
	void del_mmsi(Monitor_data * data_p,List<Monitor_data> **prev_p);
	static double range_nm(uint32_t lon_d,uint32_t lat_d);
	static double range2_nm(uint32_t lon_d,uint32_t lat_d);
	static double range3_nm(double lon_d,double lat_d);

	/*
	 * Display
	 */
	void start();
	void display_target_ais();
	void display_alerts();
	void status_alerts();
	void display_gps();
	void display_pos();
	void display_date();
	void beep();
	void voice();
	static uint16_t draw_vessels_color(uint8_t shiptype_u8);
	void display_vessels();

	static void zoom(int8_t p_i8);
private:
	Circular_queue_simple<char> labels;
	static ais_monitoring_zoom_e zoom_s;
	void display_target_ais_filtering(float max_nm_d32,float scale_px_nm_d32,monitoriring_status_e filter_e);
};

/*
 * ======================
 * Externals
 * ======================
 */

extern Ais_monitoring Monitoring;
extern Gui_page Page_target;
extern Gui_page Page_vessels;
extern Gui_page Page_settings;
extern Gui_animation Animation_wifi;
extern Gui_animation Animation_spk;
extern Gui_animation Animation_gps;
extern Gui_animation Animation_bell;
extern Gui_slider Slider;


#endif //__AIS_MONITORING_H__
