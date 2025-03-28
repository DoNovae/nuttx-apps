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
#include "utilities.h"
#include "ais_channels.h"
#include "ais_monitoring.h"
#include "doais_db_update.h"

/*
 * Global
 */
static ORB_DEFINE(ais_db_update,struct orb_ais_db_update_s,0); // const struct orb_metadata g_orb_ais_db_update=...



/*
 * -----------------------
 * AIS test
 * -----------------------
 * 3 boats in the surroundings
 * sending AIS msg18.
 * Pos
 * -----------------------
 */
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


void Ais_test_bench::ais_ready_to_send()
{
	vessel1.ais_ready_to_send();
	usleep(1000*1000);
	vessel2.ais_ready_to_send();
	usleep(1000*1000);
	vessel3.ais_ready_to_send();
}

/*
 * -------------------------
 * Ais_test_vessel
 * -------------------------
 *
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

static TXPacket tx_packet_s(MAX_AIS_RX_PACKET_SIZE);
static struct orb_ais_db_update_s ais_s;

void Ais_test_vessel::ais_ready_to_send()
{
	int ptopic_ais;

	ptopic_ais=orb_advertise_queue(ORB_ID(ais_db_update),&ais_s,ORB_AIS_DB_UPDATE_QUEUE_SIZE);
	if (ptopic_ais<0)
	{
		LOG_E("ais_ready_to_send: orb_advertise_queue advertise failed: %d",errno);
	}

	tx_packet_s.reset();
	msg18.encode(station_data_s,gps_i_s,tx_packet_s);
	tx_packet_s.ais_finalize();
	memcpy(ais_s.packet_au8,tx_packet_s.mPacket,ORB_AIS_PACKET);
	ais_s.id_u8=0;
	LOG_D("ais_ready_to_send: publish msg18");
	orb_publish(ORB_ID(ais_db_update),ptopic_ais,&ais_s);

	tx_packet_s.reset();
	msg24A.encode(station_data_s,gps_i_s,tx_packet_s);
	tx_packet_s.ais_finalize();
	memcpy(ais_s.packet_au8,tx_packet_s.mPacket,ORB_AIS_PACKET);
	ais_s.id_u8=1;
	LOG_D("ais_ready_to_send: publish msg24A");
	orb_publish(ORB_ID(ais_db_update),ptopic_ais,&ais_s);

	tx_packet_s.reset();
	msg24B.encode(station_data_s,gps_i_s,tx_packet_s);
	tx_packet_s.ais_finalize();
	memcpy(ais_s.packet_au8,tx_packet_s.mPacket,ORB_AIS_PACKET);
	ais_s.id_u8=2;
	LOG_D("ais_ready_to_send: publish msg24B");
	orb_publish(ORB_ID(ais_db_update),ptopic_ais,&ais_s);

	//orb_unadvertise(ptopic_ais);
}



