/*
 * AISMessages.hpp
 *
 *  Created on: Dec 12, 2015
 *      Author: peter
 */

#ifndef AISMESSAGES_HPP_
#define AISMESSAGES_HPP_

#include <time.h>
#include "rx_packet.h"
#include "tx_packet.h"
#include "ais_utils.h"

/*
 * NMEA
 */
#define NMEA_SENTENCE_CHAR_SZ 85

//#define MSG12_TXT_SZ (156)
#define MSG12_TXT_SZ (16)



/*
 * =============================
 * Classes
 * =============================
 */


/*
 * --------------------------
 * AISMessage
 * --------------------------
 *  Ref:
 *     R-REC-M.1371-5-201402-I!!PDF-E.pdf
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
class AISMessage
{
public:
	AISMessage();
	AISMessage(const AISMessage&);

	virtual inline void encode(const StationData &station,const gps_data_t &gps_info_ps,TXPacket &packet){ASSERT(false);};
	virtual inline void encode(const StationData &station,TXPacket &packet){ASSERT(false);};
	virtual inline void encode(TXPacket &packet){ASSERT(false);};
	virtual inline bool decode(const RXPacket &packet,uint8_t ch_u8){ASSERT(false);return false;};

	inline static double lon_d2double(const uint32_t val_u32){return Utils::uint32_coordinate(val_u32,28);}
	inline static double lat_d2double(const uint32_t val_u32){return Utils::uint32_coordinate(val_u32,27);}
	inline static uint32_t double2lon_d(const double lon){return Utils::coordinate_uint32(lon,28);};
	inline static uint32_t double2lat_d(const double lat){return Utils::coordinate_uint32(lat,27);};

    inline void set_shipname(const char* str_pa){
    	uint8_t len_u8=strlen(str_pa);
    	strncpy(shipname,str_pa,STATION_SHIP_NAME_SZ-1);
    	for (uint8_t i=len_u8;i<STATION_SHIP_NAME_SZ-1;i++){
    		shipname[i]='@';
    	}
    	shipname[STATION_SHIP_NAME_SZ-1]=0;
    	LOG_D("shipname: %s",shipname);
    };

    inline void set_callsign(const char* str_pa){
    	uint8_t len_u8=strlen(str_pa);
    	strncpy(callsign,str_pa,STATION_CALLSIGN_SZ-1);
    	for (uint8_t i=len_u8;i<STATION_CALLSIGN_SZ-1;i++){
    		callsign[i]='@';
    	}
    	callsign[STATION_CALLSIGN_SZ-1]=0;
    	LOG_D("callsign: %s",callsign);
    };

    inline void set_vendorid(const char* str_pa){
    	uint8_t len_u8=strlen(str_pa);
    	strncpy(vendorid,str_pa,AIS_VENDORID_SZ-1);
    	for (uint8_t i=len_u8;i<AIS_VENDORID_SZ-1;i++){
    		vendorid[i]='@';
    	}
    	vendorid[AIS_VENDORID_SZ-1]=0;
    	LOG_D("vendorid: %s",vendorid);
    };

	/*
	 * NMEA
	 */
	void nmea_encode(RXPacket &packet);

	/*
	 * Message
	 */
	char shipname[STATION_SHIP_NAME_SZ];
	char callsign[STATION_CALLSIGN_SZ];
	uint32_t mmsi;
	uint32_t d_mmsi;
	uint32_t lat_d; // Minutes/10000
	uint32_t lon_d; // Minutes/10000
	uint32_t speed_kt; // 1/10 knot
	uint32_t cog_d; // 1/10 degree - [0,3599] - E10h=3600 not available
	uint32_t radio;
	uint32_t heading_d; //[0,359] degree - 511 not available
	char vendorid[AIS_VENDORID_SZ];
	uint16_t to_bow;
	uint16_t to_stern;
	uint8_t status;
	uint8_t turn;
	uint8_t second;
	uint8_t man;
	uint8_t accuracy;
	uint8_t raim;
	uint8_t shiptype;
	uint8_t type;
	uint8_t repeat;
	uint8_t channel_u8;
	uint8_t to_port;
	uint8_t to_start_board;
	uint8_t epfd;
	uint8_t dte;
	uint8_t assigned;
	uint8_t partno;
	uint8_t cs;
	uint8_t display;
	uint8_t dsc;
	uint8_t band;
	uint8_t msg22;
	uint8_t to_starboard;

protected:
	/*
	 * NMEA
	 */
	uint8_t nmea_crc(const char* buff);
	void nmea_send(char* nmea_sentence6_a);
	uint8_t nmea_sentence_n;
	char nmea_sentence6_a[NMEA_SENTENCE_CHAR_SZ];
};


/*
 * --------------------------
 * AISMessage123
 * --------------------------
 */
class AISMessage123 : public AISMessage
{
public:
	AISMessage123();

	void encode(const StationData &station,const gps_data_t &gps_info_ps,TXPacket &packet)override;
	void encode(const StationData &station,TXPacket &packet) override;
	void encode(TXPacket &packet) override;
	bool decode(const RXPacket &packet,uint8_t ch_u8)  override;
};


/*
 * --------------------------
 * AISMessage18
 * --------------------------
 */
class AISMessage18 : public AISMessage
{
public:
	AISMessage18();
	AISMessage18(const AISMessage18& msg):AISMessage(msg){};

	void encode(const StationData &station,const gps_data_t &gps_info_ps,TXPacket &packet)override;
	void encode(const StationData &station,TXPacket &packet) override;
	void encode(TXPacket &packet) override;
	bool decode(const RXPacket &packet,uint8_t ch_u8)  override;
};


/*
 * --------------------------
 * AISMessage24A
 * --------------------------
 */
class AISMessage24A : public AISMessage
{
public:
	AISMessage24A();

	void encode(const StationData &station, TXPacket &packet)override;
	void encode(TXPacket &packet) override;
	bool decode(const RXPacket &packet,uint8_t ch_u8) override;
};


/*
 * --------------------------
 * AISMessage24B
 * --------------------------
 */
class AISMessage24B : public AISMessage
{
public:
	AISMessage24B();

	void encode(const StationData &station, TXPacket &packet) override;
	void encode(TXPacket &packet) override;
	bool decode(const RXPacket &packet,uint8_t ch_u8) override;
};



/*
 * --------------------------
 * AISMessage4
 * --------------------------
 */
class AISMessage4 : public AISMessage
{
public:
	AISMessage4();

	void encode(const StationData &station,const gps_data_t &gps_info_ps,TXPacket &packet)override;
	void encode(const StationData &station,TXPacket &packet) override;
	void encode(TXPacket &packet) override;
	bool decode(const RXPacket &packet,uint8_t ch_u8)  override;
};


/*
 * --------------------------
 * AISMessage12
 * --------------------------
 */
class AISMessage12 : public AISMessage
{
public:
	AISMessage12();

	void encode(const StationData &station,TXPacket &packet) override;
	void encode(TXPacket &packet) override;
	bool decode(const RXPacket &packet,uint8_t ch_u8)  override;
	char txt_ac[MSG12_TXT_SZ+1];
};

#endif /* AISMESSAGES_HPP_ */
