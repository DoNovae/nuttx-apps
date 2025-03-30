

#include "ais_channels.h"
#include "ais_messages.h"
#include "ais.h"

#ifndef __AIS_TEST_H__
#define __AIS_TEST_H__



/*
 * -----------------------
 * AIS Ais_test_vessel
 * -----------------------
 *
 * -----------------------
 */

class Ais_test_vessel{
public:
	AISMessage123 msg123;
	AISMessage18 msg18;
	AISMessage24A msg24A;
	AISMessage24B msg24B;
	StationData station_data_s;
	gps_data_s gps_i_s;

    void init(const StationData &station_data_s,const gps_data_t & gps_s);
    void set(const gps_data_t * gps_info_s);
    void turn(const int32_t direction_i32);
    void ais_ready_to_send(int ptopic_ais);
	inline Ais_test_vessel(){;};
	void new_postion(float azimuth_d,float dist_nm,float speed_kt,float direction);
};


/*
 * -----------------------
 * AIS test
 * -----------------------
 * 3 boats in the surroundings
 * sending AIS msg18.
 * Pos
 * -----------------------
 */

class Ais_test_bench{
public:
	static StationData my_station_s;
	static Ais_test_vessel vessel1,vessel2,vessel3;
	static void init();
	static void set_direction(int32_t direction_i32);
	static void ais_ready_to_send(int ptopic_ais);

	static int8_t on_off;
};



#endif //__AIS_TEST_H__
