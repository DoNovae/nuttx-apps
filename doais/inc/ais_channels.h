/**
 * =====================================
 *  ais_channels.h
 * -------------------------------------
 *  AIS DoNovae
 *  www.DoNovae.com
 *  Herve Bailly
 * =====================================
 *
 * =====================================
 */



#include <inttypes.h>
#include <string.h>
#include <math.h>
#include "types.h"

#ifndef AISCHANNELS_H_
#define AISCHANNELS_H_

/*
 * -------------------
 * AIS radio
 * -------------------
 */

//#define MAX_AIS_RX_PACKET_SIZE      512
#define MAX_AIS_RX_PACKET_SIZE      256

#define AIS_RAMP_DOWN 0x00
#define AIS_HDLC_FLAG 0x7E
//#define AIS_HDLC_FLAG_REVERSE 0x7E

#define NUM_AIS_CHANNELS 3

#define AIS_SHIP_NAME_BIT_SZ (120)
#define AIS_CALLSIGN_BIT_SZ (42)
#define AIS_VEDORID_BIT_SZ (42)
#define STATION_SHIP_NAME_SZ (AIS_SHIP_NAME_BIT_SZ/6+1)
#define STATION_CALLSIGN_SZ (AIS_CALLSIGN_BIT_SZ/6+1) // 7 X 6 bit ASCII characters + 0
#define AIS_VENDORID_SZ (AIS_VEDORID_BIT_SZ/6+1)
#define ITU_TO_ORDINAL(C) (C < 78 ? (C-18)*2 : (C-78)*2+1)

typedef struct {
    uint8_t itu;              // The ITU channel #
    uint8_t ordinal;          // A zero-based index as defined by WDS in radio_config.h
    char designation;         // 'A', 'B' or '?'
    double frequency;         // Frequency in MHz, mostly for reference
} ais_channel;

typedef enum {
	CH_87 = 0,
	CH_28,
	CH_88
} VHFChannel;

static const ais_channel AIS_CHANNELS[] = {
        {87, 0, 'A', 161.975},
        {28, 1, '?', 162.000},
        {88, 2, 'B', 162.025}
};



/*
 * -----------------------
 * Metrics
 * -----------------------
 */
static const double EARTH_RADIUS_IN_METERS        = 6378137.0;
static const double EARTH_CIRCUMFERENCE_IN_METERS = (double)EARTH_RADIUS_IN_METERS * 2.0 * (double)M_PI;
static const double MILES_PER_METER               = 0.000621371192;
static const double METERS_PER_FOOT               = 0.30408;
static const double FEET_PER_METER                = 1.0/(double)METERS_PER_FOOT;
#ifndef DEG_TO_RAD
static const double DEG_TO_RAD                    = (double)M_PI / 180.0;
#endif
#ifndef RAD_TO_DEG
static const double RAD_TO_DEG                    = 180.0/(double)M_PI;
#endif
static const double METER_PER_MILE=1852.0;
static const double KM_PER_MILE=1.852;
static const double MILES_PER_DEGREE_EQUATOR=60;
static const double ERR_ZERO=0.00001;
static const double MINUTES_PER_DEGREE=60;
static const double MINUTES_PER_HOUR=60;



/*
 * -----------------------
 * AIS messages
 * -----------------------
 */
typedef enum {
	HEADING_NOT_AVAILABLE=511
}heading_e;

typedef enum {
	COG_NOT_AVAILABLE=0xE10
}cog_e;


/*
 * For message 18 -- all class B "CS" stations send this
 */
typedef enum {
	CS_DEFAULT_COMM_STATE=0b1100000000000000110
}cs_e;


typedef enum {
	PARTNO_24A=0,
	PARTNO_24B=1
}partno_e;

/*
 * message_type_e
 */
typedef enum {
	MSG_NONE=0,
	MSG_1=1,
	MSG_2=2,
	MSG_3=3,
	MSG_4=4,
	MSG_12=12,
	MSG_18=18,
	MSG_19=19,
	MSG_24=24
}message_type_e;

#define AIS_MSG_TYPE_NONE 0
#define AIS_MSG_TYPE_1 1
#define AIS_MSG_TYPE_2 2
#define AIS_MSG_TYPE_3 3
#define AIS_MSG_TYPE_4 4
#define AIS_MSG_TYPE_12 12
#define AIS_MSG_TYPE_18 18
#define AIS_MSG_TYPE_19 10
#define AIS_MSG_TYPE_24A 240
#define AIS_MSG_TYPE_24B 241

/*
 * navigation_status_e
 */
typedef enum {
	UNDER_WAY_ENGINE=0,
	AT_ANCHOR=1,
	NOT_UNDER_COMMAND=2,
	RESTRICTED_MANUOEUVRE=3,
	CONSTRAINED_DRAUGHT=4,
	MOORED=5,
	AGROUND=6,
	FISHING=7,
	UNDER_WAY_SAILING=8,
	NAVIGATION_STATUS_RESERVED1=9,
	NAVIGATION_STATUS_RESERVED2=10,
	NAVIGATION_STATUS_RESERVED3=11,
	NAVIGATION_STATUS_RESERVED4=12,
	NAVIGATION_STATUS_RESERVED5=13,
	AIS_SART=14,
	NAVIGATION_STATUS_NONE=15
}navigation_status_e;


typedef enum {
	MAN_ID_NONE=0,
	NO_SPECIAL_MAN=1,
	SPECIAL_MAN=2,
}manoeuvre_indicator_e;

/*
 * shiptype_e
 */
typedef enum {
SHIPTYPE_NONE=0,
WING=20,
FISH=30,
TOWING=31,
TOWING200=32,
DREDGING=33,
DIVING=34,
MILITARY=35,
SAILING=36,
PLEASSURE_CRAFT=37,
HIGH_SPEED_CRAFT=40,
PILOT=50,
RESCUE=51,
TUG=52,
PORT_TENDER=53,
ANTI_POLLUTION=54,
LAW=55,
MERDICAL=58,
NONECOMBRANT=59,
PASSENGER=60,
CARGO=70,
TANKER=80,
OTHER=90
} shiptype_e;

/*
 * epdf_e
 */
typedef enum {
EPDF_NONE=0,
GPS=1,
GLONASS=2,
GPS_GLONASS=3,
LORAN_C=4,
CHAYKA=5,
INTEGRATED=6,
SURVEYED=7,
GALILEO=8
} epdf_e;



/*
 * -----------------------
 * AIS station
 * -----------------------
 */
class StationData
{
	public:
    char shipname[STATION_SHIP_NAME_SZ];       // Vessel name (all caps)
    char callsign[STATION_CALLSIGN_SZ];    // Radio station call sign assigned with MMSI (if applicable)
    char vendorid[AIS_VENDORID_SZ];
    uint32_t mmsi;           // Vessel MMSI (should be 30 bit)
    uint32_t mothership_mmsi;           // Vessel MMSI (should be 30 bit)
    uint8_t to_bow; // Proue
    uint8_t to_stern; // Poupe
    uint8_t to_starboard; //Tribord
    uint8_t to_port; // Babord
    uint8_t beam; // to_starboard+to_port
    uint8_t len; // to_starboard+to_port
    uint8_t flags;// ?
    uint8_t shiptype;

    inline StationData() {
    	reset();
    }

    inline void reset() {
    	set_shipname((char*)" @@@@@@@@@@@@@@@@@@@@@@@@@@");
    	set_callsign((char*)" @@@@@@");
    	set_vendorid((char*)"DONOVAE");
    	mmsi=0;
    	mothership_mmsi=0;
    	to_bow=0;
    	to_stern=0;
    	to_starboard=0;
    	to_port=0;
    	beam=0;
    	len=5;
    	flags=0;
        shiptype=SHIPTYPE_NONE;
    }

    inline StationData(const StationData & station){
    	set_shipname(station.shipname);
    	set_callsign(station.callsign);
    	set_vendorid(station.vendorid);
    	mmsi=station.mmsi;
    	mothership_mmsi=station.mothership_mmsi;
    	to_bow=station.to_bow;
    	to_stern=station.to_stern;
    	to_starboard=station.to_starboard;
    	to_port=station.to_port;
    	beam=station.beam;
    	len=station.len;
    	flags=station.flags;
    	shiptype=station.shiptype;
    };

    inline void set_shipname(const char* str_pa){
    	uint8_t len_u8=strlen(str_pa);
    	strncpy(shipname,str_pa,STATION_SHIP_NAME_SZ-1);
    	for (uint8_t i=len_u8;i<STATION_SHIP_NAME_SZ;i++){
    		shipname[i]='@';
    	}
    	shipname[STATION_SHIP_NAME_SZ-1]=0;
    	LOG_V("shipname: %s",shipname);
    };
    inline void set_callsign(const char* str_pa){
    	uint8_t len_u8=strlen(str_pa);
    	strncpy(callsign,str_pa,STATION_CALLSIGN_SZ-1);
    	for (uint8_t i=len_u8;i<STATION_CALLSIGN_SZ;i++){
    		callsign[i]='@';
    	}
    	callsign[STATION_CALLSIGN_SZ-1]=0;
    	LOG_V("callsign: %s",callsign);
    };

    inline void set_vendorid(const char* str_pa){
    	uint8_t len_u8=strlen(str_pa);
    	strncpy(vendorid,str_pa,AIS_VENDORID_SZ-1);
    	for (uint8_t i=len_u8;i<AIS_VENDORID_SZ;i++){
    		vendorid[i]='@';
    	}
    	vendorid[AIS_VENDORID_SZ-1]=0;
    	LOG_V("vendorid: %s",vendorid);
    };
};



#endif /* AISCHANNELS_H_ */
