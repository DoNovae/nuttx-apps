#include "ais_utils.h"
#include <errno.h>
#include "ais_channels.h"
#include "_assert.h"
#include "utilities.h"
#include "gmath.h"


/* --------------------
 * mnea_parse_lat_d
 * --------------------
 *   Latitude=DDMM.NNNN
 * --------------------
 */
float Utils::mnea_parse_lat_d(const char* decimal,const char hemisphere)
{
	// Latitude always starts with 4 integers: 2 for degrees, 2 for minutes . N decimal minutes
	float deg_f=nmea_atof(decimal,2);
	float min_f=nmea_atof(decimal+2,2);
	return ((hemisphere=='N')? 1 : -1)*(deg_f+min_f/60.0f);
}


/* --------------------
 * mnea_parse_long_d
 * --------------------
 *   Longitude=DDDMM.NNNN
 * --------------------
 */
float Utils::mnea_parse_long_d(const char* decimal,const char hemisphere)
{
	// Longitude always starts with 5 integers: 3 for degrees, 2 for minutes . N decimal minutes
	float deg_f=nmea_atof(decimal,3);
	float min_f=nmea_atof(decimal+3,3);
	return ((hemisphere=='E')?1:-1)*(deg_f+min_f/60.0f);
}

/*
 * -----------------------
 * coordinate_uint32
 * -----------------------
 */
uint32_t Utils::coordinate_uint32(const float val_d64,const uint8_t numBits)
{
	int32_t val_i32=(int32_t)(val_d64*(double)600000.0);
	LOG_V("val_i32(%d) - 0x%8x",val_i32,val_i32);
	return val_i32;
}


/*
 * -----------------------
 * uint32_coordinate
 * -----------------------
 */
float Utils::uint32_coordinate(const uint32_t val_u32, const uint8_t numBits)
{
	int32_t val_i32;
	LOG_V("val_u32(%d) - 0x%08x",val_u32,val_u32);
	val_i32=((int32_t)(val_u32<<(32-numBits)))>>(32-numBits);
	LOG_V("val_i32(%d) - 0x%8x",val_i32,val_i32);
	return (float)val_i32/(float)600000.0;
}


/*
 * --------------------------
 * arc_rad
 * --------------------------
 */
float Utils::arc_rad(const float lat1_r, const float lon1_r, const float lat2_r, const float lon2_r)
{
	float dLat    = (lat2_r - lat1_r);
	float dLon    = (lon2_r - lon1_r);
	float lat1Rad = lat1_r;
	float lat2Rad = lat2_r;

	float a = sin(dLat/2.0) * sin(dLat/2.0) + sin(dLon/2.0) * sin(dLon/2) * cos(lat1Rad) * cos(lat2Rad);
	float radians = 2 * asin(min(1.0, (double)sqrt(a)));
	return radians;
}

/*
 * --------------------------
 * distance_m
 * --------------------------
 * Distance in meters
 * --------------------------
 */
float Utils::distance_m(const float lat1_r,const float lon1_r,const float lat2_r,const float lon2_r)
{
	return EARTH_RADIUS_IN_METERS * Utils::arc_rad(lat1_r, lon1_r, lat2_r, lon2_r);
}

/*
 * --------------------------
 * distance_nm
 * --------------------------
 * Distance in nautical miles
 * --------------------------
 */
float Utils::distance_nm(const float lat1_d,const float lon1_d,const float lat2_d,const float lon2_d)
{
	float dLat=(lat2_d-lat1_d);
	float dLon=(lon2_d-lon1_d);
	float posx_d64,posy_d64;

	posx_d64=dLon*cosf(lat1_d*DEG_TO_RAD)*(float)MILES_PER_DEGREE_EQUATOR;
	posy_d64=dLat*(float)MILES_PER_DEGREE_EQUATOR;
	return sqrt((posx_d64)*(posx_d64)+(posy_d64)*(posy_d64));
}

/*
 * --------------------------
 * test_ascii
 * --------------------------
 */
bool Utils::test_ascii(const char *str_p)
{
	uint8_t byte;
	for (uint16_t c=0;c<strlen(str_p);c++){
		byte=(uint8_t)str_p[c];
		LOG_V("byte %d",byte);
		if ((byte>95)||((byte<32))) return false;
	}
	return true;
};


