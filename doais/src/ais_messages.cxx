/**
 * =====================================
 * AISMessages.cpp
 * -------------------------------------
 *  AIS DoNovae
 *  www.DoNovae.com
 *  Herve Bailly
 * =====================================
 *     R-REC-M.1371-5-201402-I!!PDF-E.pdf
 *     https://gpsd.gitlab.io/gpsd/AIVDM.html
 *     https://www.navcen.uscg.gov/?pageName=AISMessages
 *     Online decoder: https://www.aggsoft.com/ais-decoder.htm
 * =====================================
 */

#include <math.h>
#include <string.h>
#include "utilities.h"
#include "ais_utils.h"
#include "wifi_.h"
#include "gmath.h"
#include "ais_messages.h"




/*
 * --------------------------
 * AISMessage
 * --------------------------
 *  Ref:
 *     https://gpsd.gitlab.io/gpsd/AIVDM.html
 *     https://www.navcen.uscg.gov/?pageName=AISMessages
 *     Online decoder: https://www.aggsoft.com/ais-decoder.htm
 *  Build payload.
 *  Numerical parameters:
 *     Each parameter field is defined with the most significant bit first.
 *  Text:
 *     Character strings are presented left to right most significant bit first.
 *     All unused characters should be represented by the @ symbol,
 *     and they should be placed at the end of the string.
 * --------------------------
 */
AISMessage::AISMessage()
{
	/*
	 * NMEA
	 */
	nmea_sentence_n=0;
	/*
	 * AIS message
	 */
	mmsi=0;
	repeat=0;
	type=MSG_NONE;
	channel_u8=0;
	cog_d=COG_NOT_AVAILABLE;
	radio=CS_DEFAULT_COMM_STATE;
	speed_kt=0;
	lat_d=0;
	lon_d=0;
	heading_d=HEADING_NOT_AVAILABLE;
	status=NAVIGATION_STATUS_NONE;
	turn=0;
	second=0;
	man=MAN_ID_NONE;
	accuracy=0;
	strncpy(shipname,"@@@@@@@@@@@@@@@@@@@@@@@@@@@",STATION_SHIP_NAME_SZ-1);
	shipname[STATION_SHIP_NAME_SZ-1]=0;
	strncpy(callsign,"@@@@@@@",STATION_CALLSIGN_SZ-1);
	callsign[STATION_CALLSIGN_SZ-1]=0;
	strncpy(vendorid,"@@@@",AIS_VENDORID_SZ-1);
	vendorid[AIS_VENDORID_SZ-1]=0;
	to_bow=0;
	to_stern=0;
	raim=0;
	shiptype=0;
	to_port=0;
	to_start_board=0;
	epfd=0;
	dte=0;
	assigned=0;
	partno=0;
	cs=0;
	display=0;
	dsc=0;
	band=0;
	msg22=0;
}

AISMessage::AISMessage(const AISMessage& msg) {
	/*
	 * NMEA
	 */
	nmea_sentence_n=msg.nmea_sentence_n;
	/*
	 * AIS message
	 */
	mmsi=msg.mmsi;
	repeat=msg.repeat;
	type=msg.type;
	channel_u8=msg.channel_u8;
	cog_d=msg.cog_d;
	radio=msg.radio;
	speed_kt=msg.speed_kt;
	lat_d=msg.lat_d;
	lon_d=msg.lon_d;
	heading_d=msg.heading_d;
	status=msg.status;
	turn=msg.turn;
	second=msg.second;
	man=msg.man;
	accuracy=msg.accuracy;
	strncpy(shipname,msg.shipname,STATION_SHIP_NAME_SZ-1);
	shipname[STATION_SHIP_NAME_SZ-1]=0;
	strncpy(callsign,msg.callsign,STATION_CALLSIGN_SZ-1);
	callsign[STATION_CALLSIGN_SZ-1]=0;
	strncpy(vendorid,msg.vendorid,AIS_VENDORID_SZ-1);
	vendorid[AIS_VENDORID_SZ-1]=0;
	to_bow=msg.to_bow;
	to_stern=msg.to_stern;
	raim=msg.raim;
	shiptype=msg.shiptype;
	to_port=msg.to_port;
	to_start_board=msg.to_starboard;
	epfd=msg.epfd;
	dte=msg.dte;
	assigned=msg.assigned;
	partno=msg.partno;
	cs=msg.cs;
	display=msg.display;
	dsc=msg.dsc;
	band=msg.band;
	msg22=msg.msg22;
}

/*
 * -------------------------
 * encode
 * -------------------------
 * Encoding into char string
 * !AIVDM,1,1,,A,133m@ogP00PD;88MD5MTDww@2D7k,0*46
 *   type=1
 *   mmsi=205344990
 *   repeat=0
 *   status=15
 *   lon=4.4070466
 *   lat=51.229636
 *   accuracy=1
 *   heading=511
 * -------------------------
 */
void AISMessage::nmea_encode(RXPacket &packet)
{
	static uint16_t MAX_SENTENCE_BYTES = 56;
	static uint16_t MAX_SENTENCE_BITS = MAX_SENTENCE_BYTES * 6;
	uint16_t num_seq;
	/*
	 * ASCII 6 bits
	 */

	uint16_t numBits=packet.index();
	uint16_t fillBits=0;

	if (numBits%6) {
		fillBits=6-(numBits%6);
		numBits=packet.index();
	}
	num_seq=numBits/MAX_SENTENCE_BITS+1;
	LOG_V("num_seq(%d)",num_seq);

	if (num_seq>1) {
		++nmea_sentence_n;
		if (nmea_sentence_n>9) nmea_sentence_n=0;
	}

	uint16_t pos=0;
	for (uint16_t snt=1;snt<=num_seq;++snt){
		uint8_t k=0;
		/*
		 * Header in ASCII8
		 */
		if (num_seq>1){
			sprintf(nmea_sentence6_a,"!AIVDM,%d,%d,%d,%c,",num_seq,snt,nmea_sentence_n,AIS_CHANNELS[packet.channel()].designation);
		}else{
			sprintf(nmea_sentence6_a,"!AIVDM,%d,%d,,%c,",num_seq,snt,AIS_CHANNELS[packet.channel()].designation);
		}

		k=strlen(nmea_sentence6_a);
		uint16_t sentenceBits=0;

		/*
		 * Encoding AIS payload
		 * into 6 bits char string
		 */
		uint8_t nmeaByte=0;
		LOG_V("numBits(%d)",numBits);
		for (;(pos<numBits-6)&&(sentenceBits<MAX_SENTENCE_BITS);pos+=6,sentenceBits+=6){
			/*
			 * Get 6 first bits
			 */
			nmeaByte=packet.field2u8_msbfirst(pos,6);
			LOG_V("pos(%d) - nmeaByte=0x%02x - %d",pos,nmeaByte,nmeaByte);
			/*
			 * Encoding in char 6 bits
			 */
			nmeaByte+=(nmeaByte<40)?48:56;
			LOG_V("nmeaByte=0x%02x - '%c'",nmeaByte,nmeaByte);
			nmea_sentence6_a[k++]=nmeaByte;
		}
		LOG_V("pos(%d)",pos);
		uint8_t left_u8=numBits-pos;
		LOG_V("left_u8(%d)",left_u8);
		if (left_u8>0) {
			nmeaByte=packet.field2u8_msbfirst(pos,left_u8);
			nmeaByte+=(nmeaByte<40)?48:56;
			nmea_sentence6_a[k++]=nmeaByte;
		}
		nmea_sentence6_a[k++]=',';
		if (num_seq>1) {
			if (snt==num_seq){
				/*
				 * '0'+2=>'2'
				 */
				nmea_sentence6_a[k++]='0'+fillBits;
			} else {
				nmea_sentence6_a[k++]='0';
			}
		} else {
			nmea_sentence6_a[k++]='0';
		}

		nmea_sentence6_a[k++]='*';
		sprintf(nmea_sentence6_a+k,"%.2X",nmea_crc(nmea_sentence6_a));
		nmea_send(nmea_sentence6_a);
	}
}
/*
 * ------------------
 * nmeaCRC
 * ------------------
 */
uint8_t AISMessage::nmea_crc(const char* buff)
{
	uint8_t p = 1;
	uint8_t crc = buff[p++];
	while (buff[p] != '*' )
		crc ^= buff[p++];
	return crc;
}

/*
 * -------------------
 * send
 * -------------------
 */
void AISMessage::nmea_send(char* nmea_sentence6_a){
	LOG_D("%s\n",nmea_sentence6_a);
	if (ais_wifi::state==AIS_WIFI_ON)
	{
		ais_wifi::wifi_udp_write(nmea_sentence6_a,(int)strlen(nmea_sentence6_a));
	}
}


/*
 * --------------------------------
 * AISMessage123
 * --------------------------------
 *   Position report
 *   type=1,2 or 3
 *     1 - Scheduled position report.
 *   Class A shipborne mobile equipment
 *     2 - Assigned scheduled position report.
 *      Class A shipborne mobile equipment
 *     3 - Special position report, response to interrogation
 *      Class A shipborne mobile equipment
 * --------------------------------
 *         Len  Pos
 * type     6    0
 * repeat   2    6
 * mmsi    30    8
 * status   4   38
 * turn     8   42
 * speed   10   50
 * accuracy 1   60
 * lon     28   61
 * lat     27   89
 * Cog     12  116
 * head     9  128
 * second   6  137
 * man      2  143
 * spare    3  145
 * spare    1  148
 * radio   19  149
 * Total: 168 bits
 * --------------------------------
 */
AISMessage123::AISMessage123()
{
	type=MSG_1;
}

void AISMessage123::encode(const StationData &station,const gps_data_t &gps_i_s,TXPacket &packet)
{
	mmsi=station.mmsi;
	speed_kt=(uint32_t)(gps_i_s.speed_kt*(double)10.0);
	/*
	lon_d=double2lon_d(nmea_ndeg2degree(gps_i_s.lon_d));
	lat_d=double2lat_d(nmea_ndeg2degree(gps_i_s.lat_d));
	*/
	lon_d=double2lon_d((float)gps_i_s.lon_d/LAT_LONG_SCALE);
	lat_d=double2lat_d((float)gps_i_s.lat_d/LAT_LONG_SCALE);

	cog_d=(uint32_t)(gps_i_s.heading_d*(double)10.0);

	encode(packet);
}

void AISMessage123::encode(const StationData &station,TXPacket &packet)
{
	mmsi=station.mmsi;
	encode(packet);
}

void AISMessage123::encode(TXPacket &packet)
{
	uint32_t value_u32;

	packet.index(0);

#if defined(__PREAMBLE_FLAGCC) || defined(__NOPREAMBLE_FLAGCC)
	value_u32=AIS_PREAMBLE;
	packet.add_bytes_msbfirst(value_u32,24);// Preamble 0101
#endif

	value_u32=(uint32_t)AIS_HDLC_FLAG;
	packet.add_bytes_msbfirst(value_u32,8);// Sync word

	value_u32=type;
	LOG_D("type: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,6);

	value_u32=repeat;
	LOG_V("repeat: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,2);

	value_u32=mmsi;
	LOG_V("mmsi: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,30);

	value_u32=status;
	LOG_V("status: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,4);

	value_u32=turn;
	LOG_V("turn: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,8);

	value_u32=speed_kt;
	LOG_V("speed_kt: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,10);

	value_u32=accuracy;
	LOG_V("accuracy: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,1);

	value_u32=lon_d;
	LOG_V("lon_d: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,28);

	value_u32=lat_d;
	LOG_V("lat_d: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,27);

	value_u32=cog_d;
	LOG_V("cog_d: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,12);

	value_u32=heading_d;
	LOG_V("heading_d: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,9);

	value_u32=second;
	LOG_V("second: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,6);

	value_u32=man;
	LOG_V("man: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,2);

	value_u32=0;
	packet.add_bytes_msbfirst(value_u32,3);

	value_u32=raim;
	LOG_V("raim: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,1);

	value_u32=radio;
	LOG_V("radio: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,19);

	LOG_V("packet index(%d)",packet.index());

#if DEV_MODE
#if defined(__PREAMBLE_FLAGCC) || defined(__NOPREAMBLE_FLAGCC)
	LOG_D("index(%d)",packet.index());
	ASSERT(packet.index()==(168+32));
#endif
#if defined(__NSTDPREAMBLE_NOFLAG)
	ASSERT(packet.index()==(168+24));
#endif
#endif
}

/*
 * --------------------------------
 * AISMessage123
 * --------------------------------
 *         Len  Pos
 * type     6    0
 * repeat   2    6
 * mmsi    30    8
 * status   4   38
 * turn     8   42
 * speed   10   50
 * accuracy 1   60
 * lon     28   61
 * lat     27   89
 * Cog     12  116
 * head     9  128
 * second   6  137
 * man      2  143
 * spare    3  145
 * spare    1  148
 * radio   19  149
 * utc      6  135
 * --------------------------------
 */
bool AISMessage123::decode(const RXPacket &packet,uint8_t ch_u8)
{
	type=(uint8_t)packet.field2u8_msbfirst(0,6);
	LOG_V("type: 0x%02x",type);

	repeat=(uint8_t)packet.field2u8_msbfirst(6,2);
	LOG_V("repeat: 0x%02x",repeat);

	mmsi=packet.field2u32_msbfirst(8,30);
	LOG_V("mmsi: 0x%08x",mmsi);

	status=packet.field2u8_msbfirst(38,4);
	LOG_V("status: 0x%08x",status);

	turn=packet.field2u8_msbfirst(42,8);
	LOG_V("turn: 0x%08x",turn);

	speed_kt=packet.field2u32_msbfirst(50,10);
	LOG_V("speed_kt: 0x%08x",speed_kt);

	accuracy=packet.field2u8_msbfirst(60,1);
	LOG_V("accuracy: 0x%08x",accuracy);

	lon_d=packet.field2u32_msbfirst(61,28);
	LOG_V("lon_d: 0x%08x",lon_d);

	lat_d=packet.field2u32_msbfirst(89,27);
	LOG_V("lat_d: 0x%08x",lat_d);

	cog_d=packet.field2u32_msbfirst(116,12);
	LOG_V("cog_d: 0x%08x",cog_d);

	heading_d=packet.field2u32_msbfirst(128,9);
	LOG_V("heading_d: 0x%08x",heading_d);

	second=packet.field2u32_msbfirst(137,6);
	LOG_V("second: 0x%08x",second);

	man=packet.field2u32_msbfirst(143,2);
	LOG_V("man: 0x%08x",man);

	raim=packet.field2u32_msbfirst(148,1);
	LOG_V("raim: 0x%08x",raim);

	radio=packet.field2u32_msbfirst(149,19);
	LOG_V("radio: 0x%08x",radio);

	channel_u8=ch_u8;
	LOG_V("Type(%d) - repeat(%d) - MMSI(%d) - speed_kt(%d) - lon_d(%.2f) - lat_d(%.2f) - cog_d(%d) - heading_d(%d) - Ch(%d)\n",type,repeat,mmsi,speed_kt,lon_d2double(lon_d),lat_d2double(lat_d),cog_d,heading_d,channel_u8);
	return true;
}

/*
 * --------------------------------
 * AISMessage18
 * --------------------------------
 *        Len  Pos
 * type    6    0
 * repeat  2    6
 * mmsi   30    8
 * spare   8   38
 * speed  10   46
 * Accu    1   56
 * lon    28   57
 * lat    27   85
 * Cog    12  112
 * head    9  124
 * utc     6  133
 * --------------------------------
 */

AISMessage18::AISMessage18()
{
	type=MSG_18;
	cs=1;// We are a "CS" unit
	display=1;// We have no display
	dsc=0;// We have no DSC
	band=0;// We don't switch frequencies so this doesn't matter
	msg22=0;// We do not respond to message 22 to switch frequency
	assigned=1;// We operate in autonomous and continuous mode
	raim=0;// No RAIM
	radio=CS_DEFAULT_COMM_STATE;// Standard communication state flag for CS units
	cog_d=COG_NOT_AVAILABLE;
}

void AISMessage18::encode(const StationData &station,const gps_data_t &gps_i_s,TXPacket &packet)
{
	mmsi=station.mmsi;
	speed_kt=(uint32_t)(gps_i_s.speed_kt*(double)10.0);
	/*
	lon_d=double2lon_d(nmea_ndeg2degree(gps_i_s.lon_d));
	lat_d=double2lat_d(nmea_ndeg2degree(gps_i_s.lat_d));
	*/
	lon_d=double2lon_d((float)gps_i_s.lon_d/LAT_LONG_SCALE);
	lat_d=double2lat_d((float)gps_i_s.lat_d/LAT_LONG_SCALE);
	cog_d=(uint32_t)(gps_i_s.heading_d*(double)10.0);

	encode(packet);
}

void AISMessage18::encode(const StationData &station,TXPacket &packet)
{
	mmsi=station.mmsi;
	encode(packet);
}

void AISMessage18::encode(TXPacket &packet)
{
	uint32_t value_u32;

	packet.index(0);

#if defined(__PREAMBLE_FLAGCC) || defined(__NOPREAMBLE_FLAGCC)
	value_u32=AIS_PREAMBLE;
	packet.add_bytes_msbfirst(value_u32,24);// Preamble 0101
#endif

	value_u32=(uint32_t)AIS_HDLC_FLAG;
	packet.add_bytes_msbfirst(value_u32,8);

	value_u32=type;
	LOG_D("type: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,6);

	value_u32=repeat;
	LOG_V("repeat: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,2);

	value_u32=mmsi;
	LOG_V("mmsi: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,30);

	value_u32=0;
	packet.add_bytes_msbfirst(value_u32,8);

	value_u32=speed_kt;
	LOG_V("speed_kt: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,10);

	value_u32=accuracy;
	LOG_V("accuracy: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,1);

	value_u32=lon_d;
	LOG_V("lon_d: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,28);

	value_u32=lat_d;
	LOG_V("lat_d: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,27);

	value_u32=cog_d;
	LOG_V("cog_d: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,12);

	value_u32=heading_d;
	LOG_V("heading_d: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,9);

	value_u32=second;
	LOG_V("second: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,6);

	value_u32=0; // Spare
	packet.add_bytes_msbfirst(value_u32,2);

	value_u32=cs;
	LOG_V("cs",value_u32);
	packet.add_bytes_msbfirst(value_u32,1);

	value_u32=display;
	LOG_V("display: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,1);

	value_u32=dsc;
	LOG_V("dsc: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,1);

	value_u32=band;
	LOG_V("band: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,1);

	value_u32=msg22;
	LOG_V("msg22: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,1);

	value_u32=assigned;
	LOG_V("assigned: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,1);

	value_u32=raim;
	LOG_V("raim: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,1);

	value_u32=radio;
	LOG_V("radio: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,20);

	LOG_V("packet index(%d)",packet.index());

#if DEV_MODE
#if defined(__PREAMBLE_FLAGCC) || defined(__NOPREAMBLE_FLAGCC)
	LOG_D("index(%d)",packet.index());
	ASSERT(packet.index()==(168+32));
#endif
#if defined(__NSTDPREAMBLE_NOFLAG)
	ASSERT(packet.index()==(168+24));
#endif
#endif
}

/*
 * -----------------------------
 * AISMessage18
 * -----------------------------
 *        Len  Pos
 * type    6    0
 * repea   2    6
 * mmsi   30    8
 * spare   8   38
 * speed  10   46
 * accu    1   56
 * lon    28   57
 * lat    27   85
 * Cog    12  112
 * head    9  124
 * utc     6  133
 *
 * -----------------------------
 */

bool AISMessage18::decode(const RXPacket &packet,uint8_t ch_u8)
{
	type=(uint8_t)packet.field2u8_msbfirst(0,6);
	LOG_V("type: 0x%02x",type);

	repeat=(uint8_t)packet.field2u8_msbfirst(6,2);
	LOG_V("repeat: 0x%02x",repeat);

	mmsi=packet.field2u32_msbfirst(8,30);
	LOG_V("mmsi: 0x%08x",mmsi);

	uint8_t spare_u8=packet.field2u8_msbfirst(38,8);
	LOG_V("spare_u8: 0x%02x",spare_u8);

	speed_kt=packet.field2u32_msbfirst(46,10);
	LOG_V("speed_kt: 0x%08x",speed_kt);

	accuracy=(uint8_t)packet.field2u8_msbfirst(56,1);
	LOG_V("accuracy: 0x%02x",accuracy);

	lon_d=packet.field2u32_msbfirst(57,28);
	LOG_V("lon_d: 0x%08x",lon_d);

	lat_d=packet.field2u32_msbfirst(85,27);
	LOG_V("lat_d: 0x%08x",lat_d);

	cog_d=packet.field2u32_msbfirst(112,12);
	LOG_V("cog_d: 0x%08x",cog_d);

	heading_d=packet.field2u32_msbfirst(124,9);
	LOG_V("heading_d: 0x%08x",heading_d);

	second=packet.field2u8_msbfirst(133,6);
	LOG_V("second: 0x%02x",second);

	channel_u8=ch_u8;
	LOG_V("Type(%d) - repeat(%d) - MMSI(%d) - speed_kt(%d) - lon_d(%.2f) - lat_d(%.2f) - cog_d(%d) - heading_d(%d) - second(%d) - Ch(%d)\n",type,repeat,mmsi,speed_kt,lon_d2double(lon_d),lat_d2double(lat_d),cog_d,heading_d,second,channel_u8);
	return true;
}



/*
 * --------------------------------
 * AISMessage24A
 * --------------------------------
 *   Static data report
 *   Additional data assigned to an MMSI
 *   Part A: Name
 *   Part B: Static Data
 *
 * --------------------------------
 * AISMessage24A
 * --------------------------------
 *          Len  Pos
 * type       6    0
 * repeat     2    6
 * mmsi      30    8
 * partno     2    38
 * name     120    40
 * spare      8   160
 * Total: 168
 * --------------------------------
 */
AISMessage24A::AISMessage24A()
{
	type=MSG_24;
	partno=PARTNO_24A;
}

void AISMessage24A::encode(const StationData &station,TXPacket &packet)
{
	mmsi=station.mmsi;
	set_shipname(station.shipname);
	encode(packet);
}

void AISMessage24A::encode(TXPacket &packet)
{
	uint32_t value_u32;
	packet.index(0);

#if defined(__PREAMBLE_FLAGCC) || defined(__NOPREAMBLE_FLAGCC)
	value_u32=AIS_PREAMBLE;
	packet.add_bytes_msbfirst(value_u32,24);// Preamble 0101
#endif

	value_u32=AIS_HDLC_FLAG;
	packet.add_bytes_msbfirst(value_u32,8);

	value_u32=type;
	LOG_D("type: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,6);

	value_u32=repeat;
	LOG_V("repeat: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,2);

	value_u32=mmsi;
	LOG_V("mmsi: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,30);

	value_u32=partno;
	LOG_V("partno: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,2);

	LOG_D("shipname: %s",shipname);
	packet.add_str(shipname,120);

	value_u32=0;
	packet.add_bytes_msbfirst(value_u32,8);

	LOG_V("packet index(%d)",packet.index());
#if DEV_MODE
#if defined(__PREAMBLE_FLAGCC) || defined(__NOPREAMBLE_FLAGCC)
	LOG_D("index(%d)",packet.index());
	ASSERT(packet.index()==(168+32));
#endif
#if defined(__NSTDPREAMBLE_NOFLAG)
	ASSERT(packet.index()==(168+24));
#endif
#endif
}


/*
 * --------------------------------
 *          Len  Pos
 * type       6    0
 * repeat     2    6
 * mmsi      30    8
 * partno     2    38
 * name     120    40
 * spare      8   160
 * Total: 168
 * --------------------------------
 */
bool AISMessage24A::decode(const RXPacket &packet,uint8_t ch_u8)
{
	type=(uint8_t)packet.field2u8_msbfirst(0,6);
	LOG_D("type: 0x%02x",type);

	repeat=(uint8_t)packet.field2u8_msbfirst(6,2);
	LOG_V("repeat: 0x%02x",repeat);

	mmsi=packet.field2u32_msbfirst(8,30);
	LOG_V("mmsi: 0x%08x",mmsi);

	partno=packet.field2u32_msbfirst(38,2);
	LOG_V("partno: 0x%08x",partno);

	packet.field2str_msbfirst(shipname,STATION_SHIP_NAME_SZ-1,40,120);
	LOG_V("shipname: %s",shipname);

	return true;
}


/*
 * --------------------------------
 * AISMessage24B
 * --------------------------------
 *   Additional data assigned to an MMSI
 *   Part A: Name
 *   Part B: Static Data
 *
 * --------------------------------
 *          Len  Pos
 * type       6    0
 * repeat     2    6
 * mmsi      30    8
 * partno     2   38
 * ship       8   40
 * vendor    42   48
 * callsign  42   90
 * bow        9  132
 * stern      9  141
 * port       6  150
 * startbrd   6  156
 * spare      6  162
 * mmsi2     30  132 // if auxiliary craft
 * Total: 168
 * --------------------------------
 */
AISMessage24B::AISMessage24B()
{
	type=MSG_24;
	partno=PARTNO_24B;
	repeat=0;
	shiptype=SHIPTYPE_NONE;
}

void AISMessage24B::encode(const StationData &station, TXPacket &packet)
{
	mmsi=station.mmsi;
	shiptype=station.shiptype;
	to_bow=station.to_bow;
	to_stern=station.to_stern;
	to_port=station.to_port;
	to_starboard=station.to_starboard;
	set_callsign(station.callsign);
	set_vendorid(station.vendorid);

	encode(packet);
}

void AISMessage24B::encode(TXPacket &packet)
{
	uint32_t value_u32;
	packet.index(0);


#if defined(__PREAMBLE_FLAGCC) || defined(__NOPREAMBLE_FLAGCC)
	value_u32=AIS_PREAMBLE;
	packet.add_bytes_msbfirst(value_u32,24); // Preamble 0101
#endif

	value_u32=AIS_HDLC_FLAG;
	packet.add_bytes_msbfirst(value_u32,8);

	value_u32=type;
	LOG_V("type: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,6);

	value_u32=repeat;
	LOG_V("repeat: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,2);

	value_u32=mmsi;
	LOG_V("mmsi: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,30);

	value_u32=partno;
	LOG_V("partno: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,2);

	value_u32=shiptype;
	LOG_D("shiptype: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,8);

	LOG_V("vendorid: %s",vendorid);
	packet.add_str(vendorid,42);

	LOG_V("callsign: %s",callsign);
	packet.add_str(callsign,42);

	value_u32=to_bow;
	LOG_V("to_bow: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,9);

	value_u32=to_stern;
	LOG_V("to_stern: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,9);

	value_u32=to_port;
	LOG_V("to_port: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,6);

	value_u32=to_starboard;
	LOG_V("to_start_board: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,6);

	value_u32 = 0;
	packet.add_bytes_msbfirst(value_u32,6);

	LOG_V("packet index(%d)",packet.index());

#if DEV_MODE
#if defined(__PREAMBLE_FLAGCC) || defined(__NOPREAMBLE_FLAGCC)
	LOG_D("index(%d)",packet.index());
	ASSERT(packet.index()==(168+32));
#endif
#if defined(__NSTDPREAMBLE_NOFLAG)
	ASSERT(packet.index()==(168+24));
#endif
#endif
}

/*
 * --------------------------------
 *          Len  Pos
 * type       6    0
 * repeat     2    6
 * mmsi      30    8
 * partno     2   38
 * ship       8   40
 * vendor    42   48
 * callsign  42   90
 * bow        9  132
 * stern      9  141
 * port       6  150
 * startbrd   6  156
 * spare      6  162
 * mmsi2     30  132 // if auxiliary craft
 * Total: 168
 * --------------------------------
*/
bool AISMessage24B::decode(const RXPacket &packet,uint8_t ch_u8)
{
	type=(uint8_t)packet.field2u8_msbfirst(0,6);
	LOG_D("type: 0x%02x",type);

	repeat=(uint8_t)packet.field2u8_msbfirst(6,2);
	LOG_V("repeat: 0x%02x",repeat);

	mmsi=packet.field2u32_msbfirst(8,30);
	LOG_V("mmsi: 0x%08x",mmsi);

	partno=packet.field2u32_msbfirst(38,2);
	LOG_V("partno: 0x%08x",partno);

	shiptype=packet.field2u32_msbfirst(40,8);
	LOG_V("shiptype: 0x%08x",shiptype);

	packet.field2str_msbfirst(vendorid,AIS_VENDORID_SZ-1,48,42);
	LOG_I("vendorid: %s",vendorid);

	packet.field2str_msbfirst(callsign,STATION_CALLSIGN_SZ-1,90,42);
	LOG_I("callsign: %s",callsign);

	to_bow=packet.field2u32_msbfirst(132,9);
	LOG_V("to_bow: 0x%08x",to_bow);

	to_stern=packet.field2u32_msbfirst(141,9);
	LOG_V("to_stern: 0x%08x",to_stern);

	to_port=packet.field2u32_msbfirst(150,6);
	LOG_V("to_port: 0x%08x",to_port);

	to_starboard=packet.field2u32_msbfirst(156,6);
	LOG_V("to_starboard: 0x%08x",to_port);

	return true;
}




/*
 * --------------------------------
 * AISMessage4
 * --------------------------------
 *         Len  Pos
 * type     6    0
 * repeat   2    6
 * mmsi    30    8
 * year    14   38
 * month    4   52
 * day      5   56
 * hour     5   61
 * minute   6   66
 * second   6   72
 * accuracy 1   78
 * lon     28   79
 * lat     27  107
 * epfd     4  134
 * spare   10  138
 * raim    1   148
 * radio   19  149
 * --------------------------------
 */

AISMessage4::AISMessage4()
{
	type=MSG_4;
	repeat=0;
}

void AISMessage4::encode(const StationData &station,const gps_data_t &gps_i_s,TXPacket &packet)
{
	mmsi=station.mmsi;
	/*
	lon_d=double2lon_d(nmea_ndeg2degree(gps_i_s.lon_d));
	lat_d=double2lat_d(nmea_ndeg2degree(gps_i_s.lat_d));
	*/
	lon_d=double2lon_d((float)gps_i_s.lon_d/LAT_LONG_SCALE);
	lat_d=double2lat_d((float)gps_i_s.lat_d/LAT_LONG_SCALE);


	encode(packet);
}

void AISMessage4::encode(const StationData &station,TXPacket &packet)
{
	mmsi=station.mmsi;
	encode(packet);
}

void AISMessage4::encode(TXPacket &packet)
{
	uint32_t value_u32;

	packet.index(0);

#if defined(__PREAMBLE_FLAGCC) || defined(__NOPREAMBLE_FLAGCC)
	value_u32=AIS_PREAMBLE;
	packet.add_bytes_msbfirst(value_u32,24);// Preamble 0101
#endif

	value_u32=(uint32_t)AIS_HDLC_FLAG;
	packet.add_bytes_msbfirst(value_u32,8);// Sync word

	value_u32=type;
	LOG_D("type: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,6);

	value_u32=repeat;
	LOG_V("repeat: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,2);

	value_u32=mmsi;
	LOG_D("mmsi: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,30);

	value_u32=0;
	LOG_V("year: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,14);

	value_u32=0;
	LOG_V("month: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,4);

	value_u32=0;
	LOG_V("day: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,5);

	value_u32=0;
	LOG_V("hour: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,5);

	value_u32=0;
	LOG_V("minute: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,6);

	value_u32=0;
	LOG_V("second: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,6);

	value_u32=0;
	LOG_V("accuracy: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,1);

	value_u32=lon_d;
	LOG_D("lon_d: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,28);

	value_u32=lat_d;
	LOG_D("lat_d: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,27);

	value_u32=0;
	LOG_V("epfd: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,4);

	value_u32=0;
	LOG_V("spare: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,10);

	value_u32=0;
	LOG_V("raim: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,1);

	value_u32=0;
	LOG_V("radio: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,19);

#if DEV_MODE
#if defined(__PREAMBLE_FLAGCC) || defined(__NOPREAMBLE_FLAGCC)
	LOG_D("index(%d)",packet.index());
	ASSERT(packet.index()==(168+32));
#endif
#if defined(__NSTDPREAMBLE_NOFLAG)
	ASSERT(packet.index()==(134+24));
#endif
#endif
}

/*
 * --------------------------------
 * AISMessage4
 * --------------------------------
 *         Len  Pos
 * type     6    0
 * repeat   2    6
 * mmsi    30    8
 * year    14   38
 * month    4   52
 * day      5   56
 * hour     5   61
 * minute   6   66
 * second   6   72
 * accuracy 1   78
 * lon     28   79
 * lat     27  107
 * epfd     4  134
 * spare   10  138
 * raim    1   148
 * radio   19  149
 * --------------------------------
 */
bool AISMessage4::decode(const RXPacket &packet,uint8_t ch_u8)
{
	channel_u8=ch_u8;

	type=(uint8_t)packet.field2u8_msbfirst(0,6);
	LOG_V("type: 0x%02x",type);

	repeat=(uint8_t)packet.field2u8_msbfirst(6,2);
	LOG_V("repeat: 0x%02x",repeat);

	mmsi=packet.field2u32_msbfirst(8,30);
	LOG_V("mmsi: 0x%08x",mmsi);

	lon_d=packet.field2u32_msbfirst(79,28);
	LOG_V("lon_d: 0x%08x",lon_d);

	lat_d=packet.field2u32_msbfirst(107,27);
	LOG_V("lat_d: 0x%08x",lat_d);

	LOG_V("Type(%d) - repeat(%d) - MMSI(%d) - lon_d(%.2f) - lat_d(%.2f) - Ch(%d)",
			type,repeat,mmsi,lon_d2double(lon_d),lat_d2double(lat_d),channel_u8);
	return true;
}


/*
 * --------------------------------
 * AISMessage12
 * --------------------------------
 *         Len  Pos
 * type     6    0
 * repeat   2    6
 * mmsi    30    8
 * seqno    2   38
 * d_mmsi  30   40
 * retran   1   70
 * spare    1   71
 * text   936   72
 * --------------------------------
 */

AISMessage12::AISMessage12()
{
	type=MSG_12;
	repeat=0;
}


void AISMessage12::encode(const StationData &station,TXPacket &packet)
{
	mmsi=station.mmsi;
	encode(packet);
}



void AISMessage12::encode(TXPacket &packet)
{
	uint32_t value_u32;

	packet.index(0);

#if defined(__PREAMBLE_FLAGCC) || defined(__NOPREAMBLE_FLAGCC)
	value_u32=AIS_PREAMBLE;
	packet.add_bytes_msbfirst(value_u32,24);// Preamble 0101
#endif

	value_u32=(uint32_t)AIS_HDLC_FLAG;
	packet.add_bytes_msbfirst(value_u32,8);// Sync word

	value_u32=type;
	LOG_D("type: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,6);

	value_u32=repeat;
	LOG_D("repeat: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,2);

	value_u32=mmsi;
	LOG_D("mmsi: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,30);

	value_u32=0;
	LOG_D("seqno: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,2);

	value_u32=d_mmsi;
	LOG_D("d_mmsi: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,30);

	value_u32=0;
	LOG_D("retran: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,1);

	value_u32=0;
	LOG_D("spare: 0x%08x",value_u32);
	packet.add_bytes_msbfirst(value_u32,1);

	packet.add_str(txt_ac,MSG12_TXT_SZ*6);


#if DEV_MODE
#if defined(__PREAMBLE_FLAGCC) || defined(__NOPREAMBLE_FLAGCC)
	LOG_D("index(%d)",packet.index());
	ASSERT(packet.index()==(168+32));
#endif
#if defined(__NSTDPREAMBLE_NOFLAG)
	ASSERT(packet.index()==(134+24));
#endif
#endif
}

/*
 * --------------------------------
 * AISMessage12
 * --------------------------------
 *         Len  Pos
 * type     6    0
 * repeat   2    6
 * mmsi    30    8
 * seqno    2   38
 * d_mmsi  30   40
 * retran   1   70
 * spare    1   71
 * text   936   72
 * --------------------------------
 */
bool AISMessage12::decode(const RXPacket &packet,uint8_t ch_u8)
{
	uint32_t d_mmsi=0;
	channel_u8=ch_u8;

	type=(uint8_t)packet.field2u8_msbfirst(0,6);
	LOG_V("type: 0x%02x",type);

	repeat=(uint8_t)packet.field2u8_msbfirst(6,2);
	LOG_V("repeat: 0x%02x",repeat);

	mmsi=packet.field2u32_msbfirst(8,30);
	LOG_V("mmsi: 0x%08x",mmsi);

	d_mmsi=packet.field2u32_msbfirst(40,30);
	LOG_V("d_mmsi: 0x%08x",d_mmsi);

	memset((void*)txt_ac,0,MSG12_TXT_SZ);
	packet.field2str_msbfirst(txt_ac,MSG12_TXT_SZ,72,MSG12_TXT_SZ*6);
	txt_ac[MSG12_TXT_SZ]=0;

	LOG_V("Type(%d) - repeat(%d) - MMSI(%d) - d_mmsi(%d) - Ch(%d) - Text: %s",type,repeat,mmsi,d_mmsi,channel_u8,txt_ac);

	return true;
}
