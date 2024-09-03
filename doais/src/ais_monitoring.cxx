/**
 * =====================================
 * ais_monitoring.cpp
 * -------------------------------------
 *  AIS DoNovae
 *  www.DoNovae.com
 *  Herve Bailly
 * =====================================
 */
#include <errno.h>
#include <string.h>
//#include <driver/gpio.h>
//#include <driver/dac.h>
#include "ais.h"
#include "utilities.h"
#include "ais_monitoring.h"
#include "ais_utils.h"
#include "wifi_.h"
#include "ais_messages.h"


/*
 * Pages
 */
Gui_page Page_target(GUI_TARGET_BKGND,GUI_TOUCH_AREAS_TGT,GUI_TOUCH_AREA_TARGET_NB);
Gui_page Page_vessels(GUI_VESSELS_BKGND,GUI_TOUCH_AREAS_VSL,GUI_TOUCH_AREA_VESSELS_NB);
Gui_page Page_settings(GUI_SETTINGS_BKGND,GUI_TOUCH_AREAS_SET,GUI_TOUCH_AREA_SETTINGS_NB);
Gui_animation Animation_wifi(&GUI_ANIMATION_WIFI_XY,&GUI_ANIMATION_WIFI_SZ,GUI_ANIMATION_WIFI_A,GUI_ANIMATION_WIFI_NB);
Gui_animation Animation_spk(&GUI_ANIMATION_SPK_XY,&GUI_ANIMATION_SPK_SZ,GUI_ANIMATION_SPK_A,GUI_ANIMATION_SPK_NB);
Gui_animation Animation_bell(&GUI_ANIMATION_BELL_XY,&GUI_ANIMATION_BELL_SZ,GUI_ANIMATION_BELL_A,GUI_ANIMATION_BELL_NB);
Gui_animation Animation_gps(&GUI_ANIMATION_GPS_XY,&GUI_ANIMATION_GPS_SZ,GUI_ANIMATION_GPS_A,GUI_ANIMATION_GPS_NB);
Gui_slider Slider(&GUI_SLIDER_XY,GUI_SLIDER_HEIGHT,&GUI_SLIDER_CURSOR_SZ,GUI_BRIGHT_CURSOR,GUI_BRIGHT_MASK);
Ais_display Ais_dsp;




/*
 * -----------------------
 * cpa
 * -----------------------
 * Calculate Closest Point
 * of Approach, and the time
 * to go to the CPA.
 * 		T : target
 * 		M : Me
 * 		TM : distance M to T
 * 		CPA : distance CPA
 * 		cross : if collision detection
 *
 * Update
 * 	 range_nm
 * 	 time_to_collision_u32
 * 	 status_e
 * -----------------------
 */
void Monitor_data::cpa()
{
	nmeaPOS my_pos,target_pos;
	nmeaPOS my_speed_kt,target_speed_kt;
	nmeaPOS rel_pos,rel_speed_kt;
	double azimut_d,TM_d64,cpa_d64;
	double n_d64;
	bool cross=false;
	time_to_cpa_mn_u32=MONITORING_MIN_TIME_CPA_RST;

	/*
	 * Init pos
	 */
	my_pos.lat_d=(float)Gps_info_s.lat_d/LAT_LONG_SCALE;
	my_pos.lon_d=(float)Gps_info_s.lon_d/LAT_LONG_SCALE;

	target_pos.lat_d=AISMessage::lat_d2double(lat_d);
	target_pos.lon_d=AISMessage::lon_d2double(lon_d);

	my_pos.lat_nm=my_pos.lat_d*(double)MILES_PER_DEGREE_EQUATOR;
	my_pos.lon_nm=my_pos.lon_d*cos(my_pos.lat_d*DEG_TO_RAD)*(double)MILES_PER_DEGREE_EQUATOR;

	target_pos.lat_nm=target_pos.lat_d*(double)MILES_PER_DEGREE_EQUATOR;
	target_pos.lon_nm=target_pos.lon_d*cos(target_pos.lat_d*DEG_TO_RAD)*(double)MILES_PER_DEGREE_EQUATOR;

	/*
	 * rel_pos=TM
	 */
	rel_pos.lat_nm=target_pos.lat_nm-my_pos.lat_nm;
	rel_pos.lon_nm=target_pos.lon_nm-my_pos.lon_nm;

	/*
	 * Test TM
	 */
	TM_d64=Utils::distance_nm(my_pos.lat_d,my_pos.lon_d,target_pos.lat_d,target_pos.lon_d);
	LOG_D("AB_d64(%.1fNM)",TM_d64);

	/*
	 * Init speed
	 */
	azimut_d=0.0;
	if (cog_d!=COG_NOT_AVAILABLE){
		azimut_d=(double)cog_d/(double)10.0;
	} else if (heading_d!=HEADING_NOT_AVAILABLE){
		LOG_D("COG_NOT_AVAILABLE");
		azimut_d=(double)heading_d;
	} else{
		LOG_D("HEADING_NOT_AVAILABLE nor");
		status_e=MONITORING_STATUS_ERROR;
		return;
	}
	if ((cos(my_pos.lat_d*DEG_TO_RAD)<ERR_ZERO)&&((cos(my_pos.lat_d*DEG_TO_RAD)>-ERR_ZERO))){
		LOG_E("cos(my_pos.lat_d) %.3f too small",cos(my_pos.lat_d*DEG_TO_RAD));
		status_e=MONITORING_STATUS_ERROR;
		return;
	}
	LOG_D("My heading_d(%.1f) - target azimut_d(%.1f)",Gps_info_s.heading_d,azimut_d);

	my_speed_kt.lat_nm=Gps_info_s.speed_kt*cos(Gps_info_s.heading_d*DEG_TO_RAD);
	my_speed_kt.lon_nm=Gps_info_s.speed_kt*sin(Gps_info_s.heading_d*DEG_TO_RAD);

	target_speed_kt.lat_nm=(double)speed_kt/(double)10.0*cos(azimut_d*DEG_TO_RAD);
	target_speed_kt.lon_nm=(double)speed_kt/(double)10.0*sin(azimut_d*DEG_TO_RAD);

	LOG_D("my_speed_kt(%.1f) - target_speed_kt(%.1f)",Gps_info_s.speed_kt,(double)speed_kt/(double)10.0);

	rel_speed_kt.lat_nm=target_speed_kt.lat_nm-my_speed_kt.lat_nm;
	rel_speed_kt.lon_nm=target_speed_kt.lon_nm-my_speed_kt.lon_nm;

	if (rel_speed_kt.lat_nm==(float)0.0){
		rel_heading_d=(rel_speed_kt.lon_nm<0)?270:90;
	} else {
		rel_heading_d=atan(rel_speed_kt.lon_nm/rel_speed_kt.lat_nm)*RAD_TO_DEG;
	}
	if (rel_speed_kt.lat_nm<0) rel_heading_d+=180;

	/*
	 * Calculation CPA
	 *   For my boat
	 *   	Pos M, speed U
	 *   For target vessel
	 *      Pos T, speed V
	 *   CPA=<TM,Vrelative_normalized_otho>
	 *
	 */
	n_d64=sqrt(rel_speed_kt.lat_nm*rel_speed_kt.lat_nm+rel_speed_kt.lon_nm*rel_speed_kt.lon_nm);
	LOG_D("relat_speed_kt(%.1f) - rel_heading_d(%.1f)",n_d64,rel_heading_d);
	status_e=MONITORING_STATUS_NONE;

	if (n_d64>ERR_ZERO)
	{
		LOG_D("rel_pos(%.3f;%.3fNM) - rel_speed_kt(%.3f;%.3fKt)",rel_pos.lon_nm,rel_pos.lat_nm,rel_speed_kt.lon_nm,rel_speed_kt.lat_nm);
		cpa_d64=(rel_pos.lat_nm*rel_speed_kt.lon_nm-rel_pos.lon_nm*rel_speed_kt.lat_nm)/n_d64;

		/*
		 * <TM,Vrel> positive stricly means collision
		 */
		cross=(rel_pos.lat_nm*rel_speed_kt.lat_nm+rel_pos.lon_nm*rel_speed_kt.lon_nm)<0;

		if (cpa_d64<0){
			cpa_d64=-cpa_d64;
		}

		if (!cross){
			LOG_D("%s\n","No crossing tracks possible");
		}

		LOG_D("cpa_d64(%.1fNM) - cross(%d)",cpa_d64,cross);
	} else {
		LOG_E("Relative speeds too near or parallel route");
		//status_e=MONITORING_STATUS_ERROR;
		status_e=MONITORING_STATUS_NONE;
		return;
	}

	/*
	 * Status
	 */
	if (cross && (cpa_d64<(double)Monitoring.settings_s.cpa_warn_10thnm_u32/(double)10.0))
	{
		/*
		 * time_to_cpa_u32
		 * AH/U-V
		 */
		if (n_d64>ERR_ZERO) {
			time_to_cpa_mn_u32=(uint32_t)(0.5+sqrt(TM_d64*TM_d64-cpa_d64*cpa_d64)/n_d64*(double)MINUTES_PER_HOUR);
			LOG_D("time_to_cpa_mn_u32(%d)",time_to_cpa_mn_u32);

			if (time_to_cpa_mn_u32<Monitoring.settings_s.tcpa_max_mn_u32)
			{
				status_e=MONITORING_STATUS_ALERT;
				LOG_D("MONITORING_STATUS_ALERT");
			}
		} else {
			LOG_D("Relative speeds too near");
			status_e=MONITORING_STATUS_ERROR;
			return;
		}
	}
}



/*
 * =======================
 * AIS monitoring
 * -----------------------
 *
 * -----------------------
 */
Ais_monitoring::settings_t Ais_monitoring::settings_s={0,0};

monitoriring_display_status_e Ais_monitoring::display_status=MONITORING_DISPLAY_STATUS_NONE;
uint8_t Ais_monitoring::sys_status_u8=MONITORING_SYS_STATUS_NONE;
uint32_t Ais_monitoring::min_time_to_cpa_mn_u32=MONITORING_MIN_TIME_CPA_RST;
monitoring_refresh_e Ais_monitoring::refresh_b=MONOTORING_REFRESH_ASAP;
bool Ais_monitoring::is_pkt_b=false;
bool Ais_monitoring::is_cross_b=false;
ais_monitoring_zoom_e Ais_monitoring::zoom_s=AIS_MONITORING_Z0;

Ais_monitoring::Ais_monitoring(uint16_t mx_u16,uint16_t mx_label_u16) : Chained_list<Monitor_data>(mx_u16),labels(mx_label_u16,'?')
						{
	// Less than AIS_CHAINED_LABEL_MAX_SZ
	labels.enqueue('2');
	labels.enqueue('5');
	labels.enqueue('7');
	labels.enqueue('A');
	labels.enqueue('B');
	labels.enqueue('C');
	labels.enqueue('D');
	labels.enqueue('F');
	labels.enqueue('G');
	labels.enqueue('K');
	labels.enqueue('J');
	labels.enqueue('P');
	labels.enqueue('T');
	labels.enqueue('V');
	labels.enqueue('Y');

/*
	labels.enqueue('1');
	labels.enqueue('2');
	labels.enqueue('3');
	*/


	LOG_D("labels.size(%d)",labels.size());
						}

/*
 * -----------------------
 * update_from_msg
 * -----------------------
 */
void Ais_monitoring::update_from_msg(const AISMessage *msg_p,Monitor_data *data_p){
	switch (msg_p->type)
	{
	case MSG_1:
	case MSG_2:
	case MSG_3:
	case MSG_18:
	{
		/*
		 * Get data from message
		 */
		data_p->lon_d=msg_p->lon_d;
		data_p->lat_d=msg_p->lat_d;
		data_p->speed_kt=msg_p->speed_kt;
		data_p->cog_d=msg_p->cog_d;
		data_p->heading_d=msg_p->heading_d;
		/*
		 * Calculate range
		 */
		/*
		double lon_r=AISMessage::lon_d2double(data_p->lon_d)*DEG_TO_RAD;
		double lat_r=AISMessage::lat_d2double(data_p->lat_d)*DEG_TO_RAD;
		data_p->range_nm=Utils::distance_m(nmea_ndeg2radian(Gps_info_s.lat_d),nmea_ndeg2radian(Gps_info_s.lon_d),lat_r,lon_r);
		data_p->range_nm/=METER_PER_MILE;
		 */
		data_p->range_nm=Ais_monitoring::range2_nm(data_p->lon_d,data_p->lat_d);
		LOG_V("MSG_18: Vessel %d at range=%.1fNM - label('%c')",data_p->mmsi,data_p->range_nm,data_p->label);

		/*
		 * Reset date
		 */
		data_p->ticks_u32=0;
		break;
	}
	case MSG_24:
	{
		switch(msg_p->partno) {
		case PARTNO_24A:
		{
			LOG_D("PARTNO_24A");
			strncpy(data_p->shipname,msg_p->shipname,STATION_SHIP_NAME_SZ);
			for (uint8_t i=0;i<STATION_SHIP_NAME_SZ;i++){
				if (data_p->shipname[i]=='@') {
					data_p->shipname[i]=0;
				}
			}
			data_p->shipname[STATION_SHIP_NAME_SZ-1]=0;
			LOG_D("PARTNO_24A: label('%c') - shipname(%s)\n",data_p->label,data_p->shipname);
			break;
		}
		case PARTNO_24B:
		{
			LOG_D("PARTNO_24B");
			data_p->shiptype=msg_p->shiptype;
			LOG_D("PARTNO_24B: label('%c') - shiptype(%d)",data_p->label,data_p->shiptype);
			break;
		}
		default:
		{
			LOG_W("AISMessage24A mmsi(%d) - error partno(%d)",msg_p->mmsi,msg_p->partno);
			return;
		}
		}
		break;
	}
	default:
		LOG_W("mmsi(%d) - error type(%d)",msg_p->mmsi,msg_p->type);
	}
	data_p->print();
}


/*
 * -----------------------
 * del_mmsi
 * -----------------------
 * 24/07/22
 */
void Ais_monitoring::del_mmsi(Monitor_data * data_p,List<Monitor_data> **prev_p)
{
	// def_T='?' default label not to be enqueued
	if (data_p->label!=labels.def_T)
	{
		// Search default label to be replaced
		if (!replace_default_label(data_p->label))
		{
			LOG_D("labels.enqueue(%c)- count_u8(%d)",data_p->label,labels.count_u8);
			/*
			 * Test for debug.
			 * Normally there should be no similar label in the queue.
			 */
			if (labels.is_present(data_p->label)){
				LOG_E("Label(%c) duplicated!",data_p->label);
				Ais_monitoring::sys_status_u8|=(uint8_t)MONITORING_GPS_STATUS_LABELS_ALERT;
			} else
			{
				LOG_D("Label(%c) enqueued",data_p->label);
				//labels.enqueue(data_p->label);
				Ais_monitoring::sys_status_u8&=~(uint8_t)MONITORING_GPS_STATUS_LABELS_ALERT;
			}
			labels.enqueue(data_p->label);
		}
	}
	del_next(prev_p);
}


/*
 * -----------------------
 * update_dates
 * -----------------------
 */
bool Ais_monitoring::update(const AISMessage *msg_p,Monitor_data** monit_p,uint8_t ch_u8)
{
	double range_nm_d64=Ais_monitoring::range2_nm(msg_p->lon_d,msg_p->lat_d);
	bool newmsg_ok=false;
	/*
	 * Register new or update old mmsi if range and speed match
	 */
	if ((range_nm_d64<(double)settings_s.display_target_step_nm_u32*(double)MONITORING_DISPLAY_STEPS_NB+0.1)&&(msg_p->speed_kt>=settings_s.speed_min_kt_u32*10))
	{
		/*
		 * Register new mmsi,
		 * do nothing else.
		 */
		if (!new_mmsi(msg_p->mmsi,monit_p,range_nm_d64))
		{
			return false;
		}
		/*
		 * Update data of the previous registred mmsi
		 */
		update_from_msg(msg_p,*monit_p);
		(*monit_p)->cpa();
		LOG_D("msg: mmsi(%d) - label(%c) - range(%.1f) - repeat(%d) - cog_d(%d) - heading_d(%d) - ch_u8(%d)\n",(*monit_p)->mmsi,(*monit_p)->label,(*monit_p)->range_nm,msg_p->repeat,(*monit_p)->cog_d/10,(*monit_p)->heading_d,ch_u8);
		newmsg_ok=true;
	} else
	{
		/*
		 * Delete out of range mmsi if already registered
		 */
		LOG_D("msg delete: mmsi(%d) - range(%.1f) - repeat(%d) - cog_d(%d) - heading_d(%d) - ch_u8(%d)\n",msg_p->mmsi,range_nm_d64,msg_p->repeat,msg_p->cog_d/10,msg_p->heading_d,ch_u8);
		List<Monitor_data> *cur_p=(List<Monitor_data>*)0;
		List<Monitor_data> *prev_p=(List<Monitor_data>*)0;
		if (is_mmsi(msg_p->mmsi,monit_p,prev_p,cur_p))
		{
			// Delete mmsi
			del_mmsi(*monit_p,&prev_p);
		}
	}
	return newmsg_ok;
}

/*
 * -----------------------
 * update_range
 * -----------------------
 *   Calculate range of vessel
 *   Evaluate MONITORING_STATUS_ALERT of vessel
 * -----------------------
 */
bool Ais_monitoring::update_range(Monitor_data *monit_p)
{
	bool del=false;
	double range_nm_d64=Ais_monitoring::range2_nm(monit_p->lon_d,monit_p->lat_d);
	if ((range_nm_d64<(double)Monitoring.settings_s.display_target_step_nm_u32*(double)MONITORING_DISPLAY_STEPS_NB+0.1)&&(monit_p->speed_kt>=Monitoring.settings_s.speed_min_kt_u32*10))
	{
		monit_p->range_nm=range_nm_d64;
		monit_p->cpa();
		LOG_D("monit_p: mmsi(%d) - label(%c) - range(%.1f) - cog_d(%d) - heading_d(%d)\n",monit_p->mmsi,(monit_p)->label,(monit_p)->range_nm,(monit_p)->cog_d/10,(monit_p)->heading_d);
	} else {
		/*
		 * 24/07/22
		 * Delete mmsi if already registered
		 */
		LOG_D("monit_p delete: mmsi(%d) - range(%.1f) - cog_d(%d) - heading_d(%d)\n",monit_p->mmsi,range_nm_d64,monit_p->cog_d/10,monit_p->heading_d);
		//List<Monitor_data> *prev_p=(List<Monitor_data>*)0;
		// Delete mmsi
		del=true;
	}
	return del;
}


/*
 * -----------------------
 * update_dates_range
 * -----------------------
 *   Evaluate MONITORING_STATUS_ALERT of vessel in update_range()
 *   Return
 *   	-True: some vessels lost or out of range - refresh display needed
 */
bool Ais_monitoring::update_dates_range(uint32_t ticks_u32)
{
	List<Monitor_data> *cur_p=(List<Monitor_data>*)0;
	List<Monitor_data> *next_p=(List<Monitor_data>*)0;
	List<Monitor_data> *prev_p=(List<Monitor_data>*)0;
	Monitor_data *data_p;
	bool update_ok=false;
	cur_p=orig_p;
	prev_p=(List<Monitor_data>*)0;
	while (cur_p&&next_T(cur_p,&data_p,&next_p))
	{
		if (data_p==(Monitor_data*)0)
		{
			LOG_E("No data");
			break;
		}
		data_p->ticks_u32+=ticks_u32;
		LOG_D("mmsi(%d) - ticks_u32(%d)",data_p->mmsi,data_p->ticks_u32);
		/*
		 * Mononitoring lost vessel
		 */
		if ((data_p->ticks_u32>settings_s.lost_target_ticks_u32)||(data_p->speed_kt<settings_s.speed_min_kt_u32*10))
		{
			LOG_D("MONITORING_LOST_TICKS - mmsi(%d)",data_p->mmsi);
			update_ok=true;
		} else
		{
			/*
			 * 24/07/22
			 * Update range with new GPS position
			 */
			if (Gps_info_s.fix>0)
			{
				update_ok=update_range(data_p);
			}
		}
		if (update_ok)
		{
			del_mmsi(data_p,&prev_p);
			cur_p=next_p;
		} else {
			prev_p=cur_p;
			cur_p=next_p;
		}
	}
	return update_ok;
}

/*
 * -----------------------
 * print
 * -----------------------
 */
void Ais_monitoring::print()
{
	List<Monitor_data> *cur_p=(List<Monitor_data> *)0;
	List<Monitor_data> *next_p=(List<Monitor_data> *)0;
	Monitor_data *data_p;
	cur_p=orig_p;
	while (cur_p&&next_T(cur_p,&data_p,&next_p)){
		if (data_p==(Monitor_data*)0) {
			LOG_E("No data");
			break;
		}
		data_p->print();
		cur_p=next_p;
	}
}


/*
 * -----------------------
 * test
 * -----------------------
 */
bool Ais_monitoring::test(const uint8_t type_u8,const Monitor_data* T1_p,const Monitor_data* T2_p)
{
	LOG_V("type_u8(%d)",type_u8);
	switch(type_u8){
	case CHAINED_LIST_TEST_MMSI_EQ:
		LOG_V("CHAINED_LIST_TEST_MMSI_EQ - %d/%d",T1_p->mmsi,T2_p->mmsi);
		return (T1_p->mmsi==T2_p->mmsi);
		break;
	case CHAINED_LIST_TEST_LABEL_EQ:
		LOG_V("CHAINED_LIST_TEST_LABEL_EQ - '%c'/'%c'",T1_p->label,T2_p->label);
		return ((int)T1_p->label==(int)T2_p->label);
		break;
	case CHAINED_LIST_TEST_LABEL_GT:
		LOG_V("CHAINED_LIST_TEST_LABEL_GT - '%c'/'%c'",T1_p->label,T2_p->label);
		return ((int)T1_p->label>(int)T2_p->label);
		break;
	case CHAINED_LIST_TEST_LABEL_LT:
		LOG_V("CHAINED_LIST_TEST_LABEL_LT - '%c'/'%c'",T1_p->label,T2_p->label);
		return ((int)T1_p->label<(int)T2_p->label);
		break;
	case CHAINED_LIST_TEST_RANGE_GT:
		LOG_V("CHAINED_LIST_TEST_RANGE_GT - %.1f/%.1f",T1_p->range_nm,T2_p->range_nm);
		return (T1_p->range_nm>T2_p->range_nm);
		break;
	case CHAINED_LIST_TEST_RANGE_LT:
		LOG_V("CHAINED_LIST_TEST_RANGE_LT - %.1f/%.1f",T1_p->range_nm,T2_p->range_nm);
		return (T1_p->range_nm<T2_p->range_nm);
		break;
	default:
	{
		LOG_E("Unknown type(%d)",type_u8);
		return false;
		break;
	}
	}
}

/*
 * -----------------------
 * is_mmsi
 * -----------------------
 */
bool Ais_monitoring::is_mmsi(const uint32_t mmsi_u32,Monitor_data** _p,List<Monitor_data> *prev_p,List<Monitor_data> *cur_p)
{
	Monitor_data data;
	data.mmsi=mmsi_u32;
	return get_T(CHAINED_LIST_TEST_MMSI_EQ,&data,_p,&prev_p,&cur_p);
}

bool Ais_monitoring::is_mmsi(const uint32_t mmsi_u32,Monitor_data** _p)
{
	Monitor_data data;
	data.mmsi=mmsi_u32;
	return get_T(CHAINED_LIST_TEST_MMSI_EQ,&data,_p);
}

/*
 * -----------------------
 * new_mmsi
 * -----------------------
 */
bool Ais_monitoring::new_mmsi(const uint32_t mmsi_u32,Monitor_data** _p,double range_nm_d64)
{
	bool rtn=false;
	char label=labels.def_T;
	LOG_V("mmsi_u32(%d)",mmsi_u32);
	/*
	 * If already registred, do nothing.
	 */
	if (is_mmsi(mmsi_u32,_p))
	{
		LOG_D("mmsi(%d) already",mmsi_u32);
		return true;
	}
	/*
	 * Register new mmsi data if place enough.
	 * If queue is full, replace max range mmsi.
	 */
	if (enqueue(_p))
	{
		LOG_D("count_u8(%d)",count_u8);
		LOG_D("labels.count_u8(%d)",labels.count_u8);
		(*_p)->reset();
		label=labels.dequeue();
		rtn=true;
	} else {
		/*
		 * Replace vessel of max range higher than range_nm_d64
		 */
		LOG_D("Replace vessel of max range");
		max_range(_p,range_nm_d64);
		if (*_p!=(Monitor_data*)0)
		{
			rtn=true;
			label=(*_p)->label;
			(*_p)->reset();
		}
	}

	if (rtn)
	{
		(*_p)->label=label;
		(*_p)->mmsi=mmsi_u32;
		(*_p)->range_nm=range_nm_d64;
		(*_p)->print();
	}
	return rtn;
}

/*
 * -----------------------
 * max_range
 * -----------------------
 * Output:
 * 	*_p=data_p of max
 */
void Ais_monitoring::max_range(Monitor_data** _p,double range0_d64)
{
	double maxrange_d64=range0_d64;
	List<Monitor_data> *cur_p=(List<Monitor_data> *)0;
	List<Monitor_data> *next_p=(List<Monitor_data> *)0;
	Monitor_data *data_p=(Monitor_data*)0;
	* _p=(Monitor_data*)0;
	cur_p=orig_p;
	while (cur_p&&next_T(cur_p,&data_p,&next_p)){
		if (data_p==(Monitor_data*)0) {
			LOG_E("No data");
			break;
		}

		if (data_p->range_nm>maxrange_d64)
			// DEBUG TO MEASURE MAX SENSIBILITY
			//if (data_p->range_nm<maxrange_d64)
		{
			* _p=data_p;
			maxrange_d64=data_p->range_nm;
		}
		cur_p=next_p;
	}
}

/*
 * -----------------------
 * min_range
 * -----------------------
 *  _p :
 *  	pointer on the min range
 *  	greater than *range0_d64_p at start
 *
 */
typedef enum {
	MONITORING_MIN_MMSI_RUN=0,
	MONITORING_MIN_MMSI_FOUND=1,
	MONITORING_MIN_MMSI_STOP=2
} monitoring_min_mmsi_e;

void Ais_monitoring::min_range(Monitor_data** _p,double *range0_d64_p,uint32_t *mmsi_p)
{
	double range0_d64=*range0_d64_p;
	uint32_t mmsi_u32=*mmsi_p;
	monitoring_min_mmsi_e min_mmsi_e=MONITORING_MIN_MMSI_RUN;
	*range0_d64_p=1000;
	List<Monitor_data> *cur_p=(List<Monitor_data> *)0;
	List<Monitor_data> *next_p=(List<Monitor_data> *)0;
	Monitor_data *data_p=(Monitor_data*)0;
	* _p=(Monitor_data*)0;
	cur_p=orig_p;
	while (cur_p&&next_T(cur_p,&data_p,&next_p)){
		if (data_p==(Monitor_data*)0) {
			LOG_E("No data");
			break;
		}
		/*
		 * Search up to MMSI position : min1 of range > MMSI range.
		 * Search after MMSI position : min2 of range >= MMSI range. Stop if range==MMSI range
		 * Take the min of min1,min2.
		 */
		switch(min_mmsi_e)
		{
		case MONITORING_MIN_MMSI_RUN:
			if (data_p->mmsi==mmsi_u32)
			{
				min_mmsi_e=MONITORING_MIN_MMSI_FOUND;
			}
			if ((data_p->range_nm>range0_d64)&&(data_p->range_nm<*range0_d64_p))
			{
				*_p=data_p;
				*range0_d64_p=data_p->range_nm;
				*mmsi_p=data_p->mmsi;
			}
			break;
		case MONITORING_MIN_MMSI_FOUND:
			if ((data_p->range_nm>=range0_d64)&&(data_p->range_nm<*range0_d64_p))
			{
				*_p=data_p;
				*range0_d64_p=data_p->range_nm;
				*mmsi_p=data_p->mmsi;
				if (data_p->range_nm==range0_d64) min_mmsi_e=MONITORING_MIN_MMSI_STOP;
			}
			break;
		default:
			break;
		}
		cur_p=next_p;
	}
}

/*
 * -----------------------
 * replace_default_label
 * -----------------------
 */
bool Ais_monitoring::replace_default_label(char label)
{
	bool rtn=false;
	List<Monitor_data> *cur_p=(List<Monitor_data> *)0;
	List<Monitor_data> *next_p=(List<Monitor_data> *)0;
	Monitor_data *data_p=(Monitor_data*)0;
	cur_p=orig_p;
	while (cur_p&&next_T(cur_p,&data_p,&next_p)){
		if (data_p==(Monitor_data*)0) {
			LOG_E("No data");
			break;
		}
		if (data_p->label==labels.def_T)
		{
			rtn=true;
			data_p->label=label;
			break;
		}
		cur_p=next_p;
	}
	return rtn;
}




/*
 * -----------------------
 * range_nm
 * -----------------------
 * In nautical miles
 * -----------------------
 */
double Ais_monitoring::range_nm(uint32_t lon_d,uint32_t lat_d)
{
	double range = Utils::distance_m(nmea_degree2radian((float)Gps_info_s.lat_d/LAT_LONG_SCALE),nmea_degree2radian((float)Gps_info_s.lon_d/LAT_LONG_SCALE),AISMessage::lat_d2double(lat_d)*DEG_TO_RAD,AISMessage::lon_d2double(lon_d)*DEG_TO_RAD);
	range=range/METER_PER_MILE;
	LOG_V("Gps_info_s(lon %.1f,lat %.1f) - monitoring(lon %.1f,lat %.1f)",(float)Gps_info_s.lon_d/LAT_LONG_SCALE,(float)Gps_info_s.lat_d/LAT_LONG_SCALE,
			AISMessage::lon_d2double(lon_d),AISMessage::lat_d2double(lat_d));
	LOG_D("range=%.1fNM",range);
	return range;
}


/*
 * -----------------------
 * range_nm
 * -----------------------
 * In nautical miles
 * -----------------------
 */
#define RANGE_DEF_NM 100000
double Ais_monitoring::range2_nm(uint32_t lon_d,uint32_t lat_d)
{
	double res=100000;
	if (Gps_info_s.fix>0){
		res=Utils::distance_nm((float)Gps_info_s.lat_d/LAT_LONG_SCALE,(float)Gps_info_s.lon_d/LAT_LONG_SCALE,
				AISMessage::lat_d2double(lat_d),AISMessage::lon_d2double(lon_d));
	}
	return res;
}

double Ais_monitoring::range3_nm(double lon_d,double lat_d)
{
	return  Utils::distance_nm((float)Gps_info_s.lat_d/LAT_LONG_SCALE,(float)Gps_info_s.lon_d/LAT_LONG_SCALE,lat_d,lon_d);
}

/*
 * -----------------------
 * beep
 * -----------------------
 */
void Ais_monitoring::beep()
{
	Speaker.play_tone2k();
}

/*
 * -----------------------
 * voice
 * -----------------------
 */
void Ais_monitoring::voice()
{
	if (min_time_to_cpa_mn_u32<(MONITORING_MIN_TIME_CPA_5+MONITORING_MIN_TIME_CPA_10+1)/2)
	{
		Speaker.play_five();
	} else if (min_time_to_cpa_mn_u32<(MONITORING_MIN_TIME_CPA_10+MONITORING_MIN_TIME_CPA_15+1)/2)
	{
		Speaker.play_ten();
	} else if (min_time_to_cpa_mn_u32<(MONITORING_MIN_TIME_CPA_15+MONITORING_MIN_TIME_CPA_20+1)/2)
	{
		Speaker.play_fifteen();
	} else if (min_time_to_cpa_mn_u32<(MONITORING_MIN_TIME_CPA_20+MONITORING_MIN_TIME_CPA_30+1)/2)
	{
		Speaker.play_twenty();
	} else if (min_time_to_cpa_mn_u32<MONITORING_MIN_TIME_CPA_30+1)
	{
		Speaker.play_thirty();
	}
	else
	{
		Speaker.play_tone2k();
	}
}



/*
 * -----------------------
 * start
 * -----------------------
 */
void Ais_monitoring::start()
{
	Speaker.begin();
	Speaker.play_tone2k();
	Slider.begin();
	Ais_display::begin();
	Gui_state_s=GUI_SM_TARGET;
	Page_target.draw_background();
}



/*
 * -----------------------
 * display_ais
 * -----------------------
 */

void Ais_monitoring::zoom(int8_t p_i8)
{
	int8_t val_i8;
	val_i8=(int8_t)zoom_s+p_i8;
	if (val_i8>AIS_MONITORING_Z2) val_i8=AIS_MONITORING_Z2;
	if (val_i8<AIS_MONITORING_Z0) val_i8=AIS_MONITORING_Z0;
	switch (val_i8){
	case AIS_MONITORING_Z0:
		zoom_s=AIS_MONITORING_Z0;
		break;
	case AIS_MONITORING_Z1:
		zoom_s=AIS_MONITORING_Z1;
		break;
	case AIS_MONITORING_Z2:
		zoom_s=AIS_MONITORING_Z2;
		break;
	default:
		zoom_s=AIS_MONITORING_Z0;
		break;
	}
}


void Ais_monitoring::display_target_ais()
{
	float scale_px_nm_d32;
	float max_nm_d32;
	float alert_circ_d32;

	/*
	 * Target
	 */
	Ais_display::target.pushImage(0,0,DISPLAY_TARGET_SX,DISPLAY_TARGET_SY,(m5gfx::rgb565_t*)GUI_TARGET);

	/*
	 * Circles
	 */
	Ais_display::target.drawCircle(DISPLAY_TARGET_CENTER_X,DISPLAY_TARGET_CENTER_Y,DISPLAY_TARGET_R3,TFT_BLACK);
	scale_px_nm_d32=(float)DISPLAY_TARGET_R3/Monitoring.settings_s.display_target_step_nm_u32/(float)MONITORING_DISPLAY_STEPS_NB;
	max_nm_d32=(float)settings_s.display_target_step_nm_u32*(float)MONITORING_DISPLAY_STEPS_NB;
	alert_circ_d32=(float)DISPLAY_TARGET_R3*(float)settings_s.cpa_warn_10thnm_u32/(float)10.0/max_nm_d32;
	switch (zoom_s){
	case AIS_MONITORING_Z0:
		Ais_display::target.drawCircle(DISPLAY_TARGET_CENTER_X,DISPLAY_TARGET_CENTER_Y,DISPLAY_TARGET_R2,TFT_BLACK);
		Ais_display::target.drawCircle(DISPLAY_TARGET_CENTER_X,DISPLAY_TARGET_CENTER_Y,DISPLAY_TARGET_R1,TFT_BLACK);
		break;
	case AIS_MONITORING_Z1:
		Ais_display::target.drawCircle(DISPLAY_TARGET_CENTER_X,DISPLAY_TARGET_CENTER_Y,DISPLAY_TARGET_R32,TFT_BLACK);
		scale_px_nm_d32*=(float)3/2;
		max_nm_d32=(float)max_nm_d32*(float)2.0/(float)3.0;
		alert_circ_d32=alert_circ_d32*(float)3.0/(float)2.0;
		break;
	case AIS_MONITORING_Z2:
		scale_px_nm_d32*=(float)3;
		max_nm_d32=(float)max_nm_d32/(float)3;
		alert_circ_d32=alert_circ_d32*(float)3.0;
		break;
	}
	Ais_display::target.drawCircle(DISPLAY_TARGET_CENTER_X,DISPLAY_TARGET_CENTER_Y,(int16_t)(alert_circ_d32+(float)0.5),DISPLAY_RED);

	/*
	 * Lines
	 */
	Ais_display::target.drawLine(0,DISPLAY_TARGET_CENTER_Y,DISPLAY_TARGET_SX,DISPLAY_TARGET_CENTER_Y,TFT_BLACK);
	Ais_display::target.drawLine(DISPLAY_TARGET_CENTER_X,0,DISPLAY_TARGET_CENTER_X,DISPLAY_TARGET_SY,TFT_BLACK);

	/*
	 * Cross
	 */
	if (is_cross_b) {
		Ais_display::target.drawLine(DISPLAY_TARGET_CENTER_X-10,DISPLAY_TARGET_CENTER_Y-10,DISPLAY_TARGET_CENTER_X+10,DISPLAY_TARGET_CENTER_Y+10,DISPLAY_RED);
		Ais_display::target.drawLine(DISPLAY_TARGET_CENTER_X-10,DISPLAY_TARGET_CENTER_Y+10,DISPLAY_TARGET_CENTER_X+10,DISPLAY_TARGET_CENTER_Y-10,DISPLAY_RED);
	}

	/*
	 * Display filtered vessels
	 */
	display_target_ais_filtering(max_nm_d32,scale_px_nm_d32,MONITORING_STATUS_NONE);
	display_target_ais_filtering(max_nm_d32,scale_px_nm_d32,MONITORING_STATUS_ALERT);

	// Scale in NM
	Ais_display::draw_target_scale((int16_t)(max_nm_d32+(float)0.5));
	M5.Lcd.pushImage(DISPLAY_TARGET_POSX,DISPLAY_TARGET_POSY,DISPLAY_TARGET_SX,DISPLAY_TARGET_SY,(m5gfx::rgb565_t*)Ais_display::target.getBuffer());
}


void Ais_monitoring::display_target_ais_filtering(float max_nm_d32,float scale_px_nm_d32,monitoriring_status_e filter_e)
{
	uint16_t color_u16;
	List<Monitor_data> *cur_p=(List<Monitor_data>*)0;
	List<Monitor_data> *next_p=(List<Monitor_data>*)0;
	Monitor_data *data_p;
	float heading_d;
	cur_p=orig_p;
	while (cur_p&&next_T(cur_p,&data_p,&next_p))
	{
		if (data_p==(Monitor_data*)0)
		{
			LOG_E("No data");
			break;
		}
		if (data_p->status_e==filter_e)
		{
			/*
			 * Vessels
			 */
			float d_lat_d32=AISMessage::lat_d2double(data_p->lat_d)-(float)Gps_info_s.lat_d/LAT_LONG_SCALE;
			float d_lon_d32=(AISMessage::lon_d2double(data_p->lon_d)-(float)Gps_info_s.lon_d/LAT_LONG_SCALE)*cosf(AISMessage::lat_d2double(data_p->lat_d)*DEG_TO_RAD);
			d_lat_d32*=(float)MILES_PER_DEGREE_EQUATOR;
			d_lon_d32*=(float)MILES_PER_DEGREE_EQUATOR;

			/*
			 * Test range
			 */
			if (sqrt(d_lat_d32*d_lat_d32+d_lon_d32*d_lon_d32)<max_nm_d32)
			{
				d_lat_d32*=scale_px_nm_d32;
				d_lon_d32*=scale_px_nm_d32;

				/*
				 * Rotation +Gps_info_s.direction
				 * Inversion due to display orientation
				 * 	(-X)/Y and Y/X
				 */
				float lon_d32=d_lon_d32*cosf(Gps_info_s.heading_d*DEG_TO_RAD)-d_lat_d32*sinf(Gps_info_s.heading_d*DEG_TO_RAD);
				float lat_d32=d_lat_d32*cosf(Gps_info_s.heading_d*DEG_TO_RAD)+d_lon_d32*sinf(Gps_info_s.heading_d*DEG_TO_RAD);
				color_u16=draw_vessels_color(data_p->shiptype);
				if (data_p->status_e==MONITORING_STATUS_ALERT){
					color_u16=DISPLAY_RED;
				}

				heading_d=is_cross_b?data_p->rel_heading_d:(float)data_p->cog_d/(float)10.0;
				Ais_display::draw_target_vessels(data_p->label,(int16_t)(lon_d32+0.5),(int16_t)(lat_d32+0.5),heading_d-Gps_info_s.heading_d,color_u16);
			}
		}
		/*
		 * Next
		 */
		cur_p=next_p;
	}
}

/*
 * -----------------------
 * display_alerts
 * -----------------------
 */
void Ais_monitoring::display_alerts()
{
	List<Monitor_data> *cur_p=(List<Monitor_data>*)0;
	List<Monitor_data> *next_p=(List<Monitor_data>*)0;
	Monitor_data *data_p;
	cur_p=orig_p;
	uint8_t alert_u8=0;
	uint16_t color_u16;
	monitoriring_display_status_e status_s;

	//M5S Ais_display::clear(Ais_display::alert,DISPLAY_ALERT_X,DISPLAY_ALERT_Y,EPD_WHITE);
	status_s=MONITORING_DISPLAY_STATUS_NONE;
	Ais_monitoring::min_time_to_cpa_mn_u32=MONITORING_MIN_TIME_CPA_RST;

	Ais_display::draw_alerts_clear();
	while (cur_p&&next_T(cur_p,&data_p,&next_p)){
		if (data_p==(Monitor_data*)0) {
			LOG_E("No data");
			break;
		}
		if (data_p->status_e==MONITORING_STATUS_ALERT)
		{
			status_s=MONITORING_DISPLAY_STATUS_ALERT;

			color_u16=draw_vessels_color(data_p->shiptype);
			Ais_display::draw_vessels_one_alert(alert_u8,data_p->label,data_p->time_to_cpa_mn_u32,data_p->shipname,color_u16);
			alert_u8++;
			if (data_p->time_to_cpa_mn_u32 < min_time_to_cpa_mn_u32) min_time_to_cpa_mn_u32=data_p->time_to_cpa_mn_u32;
		}

		/*
		 * Next
		 */
		cur_p=next_p;
	}
	Ais_display::alerts_push();
	Ais_monitoring::display_status=status_s;
}

/*
 * -----------------------
 * status_alerts
 * -----------------------
 */
void Ais_monitoring::status_alerts()
{
	List<Monitor_data> *cur_p=(List<Monitor_data>*)0;
	List<Monitor_data> *next_p=(List<Monitor_data>*)0;
	Monitor_data *data_p;
	cur_p=orig_p;
	monitoriring_display_status_e status_s;

	//M5S Ais_display::clear(Ais_display::alert,DISPLAY_ALERT_X,DISPLAY_ALERT_Y,EPD_WHITE);
	status_s=MONITORING_DISPLAY_STATUS_NONE;
	Ais_monitoring::min_time_to_cpa_mn_u32=MONITORING_MIN_TIME_CPA_RST;

	while (cur_p&&next_T(cur_p,&data_p,&next_p)){
		if (data_p==(Monitor_data*)0) {
			LOG_E("No data");
			break;
		}
		if (data_p->status_e==MONITORING_STATUS_ALERT)
		{
			status_s=MONITORING_DISPLAY_STATUS_ALERT;
			if (data_p->time_to_cpa_mn_u32 < min_time_to_cpa_mn_u32) min_time_to_cpa_mn_u32=data_p->time_to_cpa_mn_u32;
		}

		/*
		 * Next
		 */
		cur_p=next_p;
	}
	Ais_monitoring::display_status=status_s;
}

/*
 * -----------------------
 * display_vessels
 * -----------------------
 */
void Ais_monitoring::display_vessels()
{
	List<Monitor_data> *cur_p=(List<Monitor_data>*)0;
	List<Monitor_data> *next_p=(List<Monitor_data>*)0;
	Monitor_data *data_p;
	uint8_t mmsi_u8=0;
	uint16_t color_u16,color_spd_u16;

	Ais_display::draw_vessels_clear();
	// First display vessels in alert
	cur_p=orig_p;
	next_p=(List<Monitor_data>*)0;
	while (cur_p&&next_T(cur_p,&data_p,&next_p)){
		if (data_p==(Monitor_data*)0) {
			LOG_E("No data");
			break;
		}
		//HBL260623
		if (mmsi_u8==DISPLAY_VESSELS_NBR){
			break;
		}
		/*
		 * Caption
		 */
		color_u16=draw_vessels_color(data_p->shiptype);
		color_spd_u16=DISPLAY_RED;
		if (data_p->status_e==MONITORING_STATUS_ALERT)
		{
			bool old_b=data_p->ticks_u32>(settings_s.lost_target_ticks_u32/2);
			Ais_display::draw_vessels(mmsi_u8,data_p->label,data_p->speed_kt,color_u16,color_spd_u16,old_b);
			mmsi_u8++;
		}
		/*
		 * Next
		 */
		cur_p=next_p;
	}
	// Then display first vessels nearest in the list not in alert
	cur_p=orig_p;
	data_p=&orig_p->data;
	double range_nm_d64=0;
	uint32_t mmsi_u32=0;
	while ((mmsi_u8<DISPLAY_VESSELS_NBR)&&data_p)
	{
		//HBL260623
		if (mmsi_u8==DISPLAY_VESSELS_NBR){
			break;
		}
		min_range(&data_p,&range_nm_d64,&mmsi_u32);
		if ((data_p!=(Monitor_data*)0)&&(data_p->status_e!=MONITORING_STATUS_ALERT))
		{
			bool old_b=data_p->ticks_u32>(settings_s.lost_target_ticks_u32/2);
			color_u16=draw_vessels_color(data_p->shiptype);
			color_spd_u16=DISPLAY_DARKGREY;
			Ais_display::draw_vessels(mmsi_u8,data_p->label,data_p->speed_kt,color_u16,color_spd_u16,old_b);
			mmsi_u8++;
		}
	}
	Ais_display::vessels_push();
}


/*
 * -----------------------
 * display_gps
 * -----------------------
 */
void Ais_monitoring::display_gps()
{
	if (Gps_info_s.fix>0)
	{
		Ais_display::draw_gps_values(Gps_info_s.speed_kt,Gps_info_s.heading_d);
	} else
	{
		Ais_display::draw_gps_values(0.0,0.0);
	}
}


void Ais_monitoring::display_pos()
{
	Ais_display::draw_pos_values(Gps_info_s.lon_d,Gps_info_s.lat_d);
}

void Ais_monitoring::display_date()
{
	Ais_display::draw_date_values(Gps_info_s.utc_s.tm_hour,Gps_info_s.utc_s.tm_min,Gps_info_s.utc_s.tm_sec);
}

/*
 * -----------------------
 * display_gps
 * -----------------------
 */
/*
void Ais_monitoring::display_wifi()
{
	LOG_V("Ais_monitoring::state(%d)",Ais_monitoring::state);
	switch (Ais_monitoring::state){
	case MONITORING_WIFI_ON:
		Ais_display::draw_wifi(true);
		break;
	case MONITORING_WIFI_OFF:
		//M5S Ais_display::clear(Ais_display::wifi,DISPLAY_WIFI_X,DISPLAY_WIFI_Y,EPD_WHITE);
		break;
	case MONITORING_WIFI_HS:
		Ais_display::draw_wifi(false);
		break;
	default:
		break;
	}
	Ais_monitoring::state=MONITORING_WIFI_ACK;
}
 */




/*
 * -----------------------
 * draw_vessels_alerts
 * -----------------------
 */
uint16_t Ais_monitoring::draw_vessels_color(uint8_t shiptype_u8)
{
	uint16_t color_u16;
	switch(shiptype_u8){
	case SAILING:
		color_u16=DISPLAY_GREEN;
		break;
	case CARGO:
	case TANKER:
	case PASSENGER:
		color_u16=DISPLAY_DARKGREY;
		break;
	case FISH:
	case PLEASSURE_CRAFT:
	case HIGH_SPEED_CRAFT:
	case PILOT:
	case TUG:
		color_u16=DISPLAY_NAVY;
		break;
	default:
		color_u16=DISPLAY_DARKGREY;
		break;
	}
	return color_u16;
}

