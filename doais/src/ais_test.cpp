/**
 * =====================================
 * ais_test.cpp
 * -------------------------------------
 *  AIS DoNovae
 *  www.DoNovae.com
 *  Herve Bailly
 * =====================================
 */
#include <string.h>
#include "ais_test.h"
#include "_assert.h"
#include "utilities.h"
#include "wifi_.h"
#include "ais_channels.h"
#include "ais_monitoring.h"





/*
 * -----------------------
 * AIS test
 * -----------------------
 * 3 boats in the surroundings
 * sending AIS msg18.
 * Pos
 * -----------------------
 */
SI4463::Si446x Ais_test_bench::si446x_tx=SI4463::si446x2_;
SI4362::Si446x Ais_test_bench::si446x_rx=SI4362::si446x1_;
Ais_test_vessel Ais_test_bench::vessel1=Ais_test_vessel();
Ais_test_vessel Ais_test_bench::vessel2=Ais_test_vessel();
Ais_test_vessel Ais_test_bench::vessel3=Ais_test_vessel();
StationData Ais_test_bench::my_station_s;
int8_t Ais_test_bench::on_off=0;

void Ais_test_bench::init() {
	StationData station_s;
	gps_data_t gps_s;
	on_off=0;
	/*
	 * Set my vessel
	 */
	my_station_s.set_shipname((char*)"TETE D ARTICHAUT");
	my_station_s.mmsi=1234;
	/*
	 * Global Gps_info_s
	 */
	//Gps_info_s.lat_d=nmea_degree2ndeg((double)47.22143353);
	//Gps_info_s.lon_d=nmea_degree2ndeg((double)-1.58430576);
	//Gps_info_s.lat_d=nmea_degree2ndeg((const double)48.6472222);
	//Gps_info_s.lon_d=nmea_degree2ndeg((const double)-2.0088889);
	Gps_info_s.speed_kt=7;
	Gps_info_s.heading_d=0.0;
	Gps_info_s.fix=1;
	/*
	 * Vessels
	 */
	station_s.set_shipname((char*)"FISH");
	station_s.mmsi=1111;
	station_s.shiptype=FISH;
	vessel1.init(station_s,Gps_info_s);
	//vessel1.new_postion(45,8,15,225);
	vessel1.new_postion(225,1,15,30);

	station_s.set_shipname((char*)"CARGO");
	station_s.mmsi=2222;
	station_s.shiptype=CARGO;
	vessel2.init(station_s,gps_s);
	//vessel2.new_postion(135,6,10,315);
	vessel2.new_postion(135,1.5,10,345);
	//vessel2.new_postion(90,6,10,90);

	station_s.set_shipname((char*)"SAILING");
	station_s.mmsi=3333;
	station_s.shiptype=SAILING;
	vessel3.init(station_s,gps_s);
	vessel3.new_postion(315,2.5,8,90);
}

void Ais_test_bench::new_mmsi()
{
	vessel1.new_mmsi(AIS_MSG_TYPE_18);
	vessel1.new_mmsi(AIS_MSG_TYPE_24A);
	vessel1.new_mmsi(AIS_MSG_TYPE_24B);
	vessel2.new_mmsi(AIS_MSG_TYPE_18);
	vessel2.new_mmsi(AIS_MSG_TYPE_24A);
	vessel1.new_mmsi(AIS_MSG_TYPE_24B);
	vessel3.new_mmsi(AIS_MSG_TYPE_18);
	vessel3.new_mmsi(AIS_MSG_TYPE_24A);
	vessel1.new_mmsi(AIS_MSG_TYPE_24B);
}

/*
 * Calculate new positions of vessel
 */
void Ais_test_bench::set_direction(int32_t direction_i32) {
	/*
	vessel1.turn(direction_i32);
	vessel2.turn(direction_i32);
	vessel3.turn(direction_i32);
	 */
	Gps_info_s.heading_d=(double)direction_i32;
	if (Gps_info_s.heading_d>360.0) Gps_info_s.heading_d=Gps_info_s.heading_d-360;
	if (Gps_info_s.heading_d<0) Gps_info_s.heading_d=Gps_info_s.heading_d+360;
}


void Ais_test_bench::send_nmea_ais()
{
	vessel1.ais_ready_to_send();
	delay(1000);
	vessel2.ais_ready_to_send();
	delay(1000);
	vessel3.ais_ready_to_send();
}

/*
 * -------------------------
 * Ais_test_vessel
 * -------------------------
 *
 * -------------------------
 */
void Ais_test_vessel::init(const StationData &station_s,const gps_data_t & gps_s)
{
	msg18=AISMessage18();
	msg24A=AISMessage24A();
	msg24B=AISMessage24B();
	station_data_s=station_s;
	memcpy((void*)&gps_i_s,(void*)&gps_s,sizeof(gps_data_t));
}

void Ais_test_vessel::new_postion(float azimuth_d,float dist_nm,float speed_kt,float direction_d)
{
	nmeaPOS start_pos,end_pos;
	start_pos.lat_r=nmea_degree2radian((float)Gps_info_s.lat_d/LAT_LONG_SCALE);
	start_pos.lon_r=nmea_degree2radian((float)Gps_info_s.lon_d/LAT_LONG_SCALE);
	nmea_move_horz(&start_pos,&end_pos,azimuth_d,dist_nm*KM_PER_MILE);
	gps_i_s.lat_d=(int32_t)((float)nmea_radian2degree(end_pos.lat_r)*LAT_LONG_SCALE);
	gps_i_s.lon_d=(int32_t)((float)nmea_radian2degree(end_pos.lon_r)*LAT_LONG_SCALE);
	gps_i_s.speed_kt=speed_kt;
	gps_i_s.heading_d=direction_d;
	LOG_W("start_pos(lon %.2f,lat %.2f) - end_pos(lon %.2f,lat %.2f)",(float)Gps_info_s.lon_d/LAT_LONG_SCALE,
			(float)Gps_info_s.lat_d/LAT_LONG_SCALE,(float)gps_i_s.lon_d/LAT_LONG_SCALE,(float)gps_i_s.lat_d/LAT_LONG_SCALE);
}

void Ais_test_vessel::turn(int32_t direction_i32)
{
	gps_i_s.heading_d=gps_i_s.heading_d+(double)direction_i32;
	if (gps_i_s.heading_d>360.0) gps_i_s.heading_d=gps_i_s.heading_d-360;
	if (gps_i_s.heading_d<0) gps_i_s.heading_d=gps_i_s.heading_d+360;
}

void Ais_test_vessel::set(const gps_data_t * gps_s)
{
	gps_i_s.lat_d=gps_s->lat_d;
	gps_i_s.lon_d=gps_s->lon_d;
	gps_i_s.speed_kt=gps_s->speed_kt;
	gps_i_s.heading_d=gps_s->heading_d;
}

void Ais_test_vessel::ais_ready_to_send()
{
	si446x_2.tx_packet_s.reset();
	msg18.encode(station_data_s,gps_i_s,si446x_2.tx_packet_s);
	si446x_2.tx_packet_s.ais_finalize();
	//si446x_2.rx_get_ais_test(si446x_2.tx_packet_s.get_pt(),si446x_2.tx_packet_s.bit_size());
	//si446x_2.tx_send_ais_packet();
	delay(100);

	si446x_2.tx_packet_s.reset();
	msg24A.encode(station_data_s,si446x_2.tx_packet_s);
	si446x_2.tx_packet_s.ais_finalize();
	//si446x_2.rx_get_ais_test(si446x_2.tx_packet_s.get_pt(),si446x_2.tx_packet_s.bit_size());
	//si446x_2.tx_send_ais_packet();
	delay(100);

	si446x_2.tx_packet_s.reset();
	msg24B.encode(station_data_s,si446x_2.tx_packet_s);
	si446x_2.tx_packet_s.ais_finalize();
	//si446x_2.rx_get_ais_test(si446x_2.tx_packet_s.get_pt(),si446x_2.tx_packet_s.bit_size());
	//si446x_2.tx_send_ais_packet();
	delay(100);
}

void Ais_test_vessel::new_mmsi(uint8_t msg_type_u8)
{
	double range_nm_d64;
	range_nm_d64=Ais_monitoring::range3_nm((float)gps_i_s.lon_d/LAT_LONG_SCALE,(float)gps_i_s.lat_d/LAT_LONG_SCALE);
	LOG_W("range_nm_d64(%.1f)",range_nm_d64);
	new_mmsi(msg_type_u8,station_data_s.mmsi,range_nm_d64);
}

void Ais_test_vessel::new_mmsi(uint8_t msg_type_u8,uint32_t mmsi_u32,float range_nm_d64)
{
	AISMessage *msg;
	Monitor_data* monit_p;
	switch (msg_type_u8)
	{
	case MSG_1:
	case MSG_2:
	case MSG_3:
	{
		LOG_W("AISMessage123");
		msg=&msg123;
		msg->mmsi=mmsi_u32;
		msg->repeat=0;
		msg->accuracy=1;
		msg->status=5;
		msg->turn=124;
		msg->speed_kt=(uint32_t)(gps_i_s.speed_kt)*10;
		msg->lat_d=AISMessage::double2lat_d((float)gps_i_s.lat_d/LAT_LONG_SCALE);
		msg->lon_d=AISMessage::double2lon_d((float)gps_i_s.lon_d/LAT_LONG_SCALE);
		msg->heading_d=HEADING_NOT_AVAILABLE;
		msg->cog_d=gps_i_s.heading_d*10;
		msg->second=40;
		msg->raim=1;
		break;
	}
	case MSG_18:
	{
		LOG_W("AISMessage18");
		msg=&msg18;
		msg->mmsi=mmsi_u32;
		msg->repeat=0;
		msg->accuracy=1;
		msg->status=5;
		msg->cs=1;
		msg->display=1;
		msg->dsc=1;
		msg->band=1;
		msg->msg22=1;
		msg->assigned=1;
		msg->speed_kt=(uint32_t)(gps_i_s.speed_kt)*10;
		msg->lat_d=AISMessage::double2lat_d((float)gps_i_s.lat_d/LAT_LONG_SCALE);
		msg->lon_d=AISMessage::double2lon_d((float)gps_i_s.lon_d/LAT_LONG_SCALE);
		msg->heading_d=HEADING_NOT_AVAILABLE;
		msg->cog_d=gps_i_s.heading_d*10;
		msg->second=40;
		msg->raim=1;
		break;
	}
	case AIS_MSG_TYPE_24A:
	{
		LOG_W("AISMessage24A");
		msg=&msg24A;
		msg->mmsi=mmsi_u32;
		strncpy(msg->shipname,station_data_s.shipname,STATION_SHIP_NAME_SZ);
		msg->repeat=0;
		msg->partno=PARTNO_24A;
		break;
	}
	case AIS_MSG_TYPE_24B:
	{
		LOG_W("AISMessage24B");
		msg=&msg24B;
		msg->mmsi=mmsi_u32;
		msg->repeat=3;
		msg->shiptype=station_data_s.shiptype;
		msg->partno=PARTNO_24B;
		break;
	}
	default:
	{
		LOG_W("No message id");
		return;
	}
	}
	switch (msg->type){
	case MSG_1:
	case MSG_2:
	case MSG_3:
	case MSG_18:
		if(Monitoring.new_mmsi(msg->mmsi,&monit_p,range_nm_d64)){
			Monitoring.update_from_msg(msg,monit_p);
			monit_p->cpa();
			LOG_D("msg: mmsi(%d) - label(%c) - range(%.1f) - repeat(%d) - cog_d(%d) - heading_d(%d) - ch_u8(%d)\n",monit_p->mmsi,monit_p->label,monit_p->range_nm,msg123.repeat,monit_p->cog_d/10,monit_p->heading_d,0);

		} else{
			LOG_W("MSG_18 - new_mmsi error");
		}
		break;
	case MSG_24:
		if (Monitoring.is_mmsi(msg->mmsi,&monit_p)){
			Monitoring.update_from_msg(msg,monit_p);
		} else {
			LOG_W("MSG_24 - mmsi error");
		}
		break;
	default:
		LOG_W("mmsi(%d) - error type(%d)",msg->mmsi,msg->type);
	}
}


