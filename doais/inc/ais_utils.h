/**
 * =====================================
 *  ais_utils.h
 * -------------------------------------
 *  AIS DoNovae
 *  www.DoNovae.com
 *  Herve Bailly
 * =====================================
 */

#include <inttypes.h>
#include "ais_channels.h"
#include "ais_messages.h"
#include "circular_queue.h"
#include "gps.h"

#ifndef __AIS_UTILS_H__
#define __AIS_UTILS_H__


/*
 * =========================
 *  Defines
 * =========================
 */
/*
 * --------------------------------
 * TX configuration
 * 	PREAMBLE_FLAGCC
 * 	  AIS TX
 * 	    AIS_PREAMBLE in payload : 3 bytes
 * 	    Flag 0x7E in payload : 1 bytes
 * 	  Config SI TX AIS
 * 	    Flag
 * 	      Not transmitted
 * 	      3 bytes CC
 * 	    Preamble
 * 	      3 bytes
 * 	      std 1010
 * 	      Preamble not tested (th=0)
 * --------------------------------
 * 	NOPREAMBLE_FLAGCC
 * 	  AIS TX
 * 	    AIS_PREAMBLE in payload : 3 bytes
 * 		Flag 0x7E in payload : 1 bytes
 * 	  Config SI std
 * 	    Flag
 * 	      4 bits OxCC
 * 	      Sync word transmitted
 * 	    Preamble
 * 	    	Standard 1010
 * 	    	20 bits
 * --------------------------------
 * 	NSTDPREAMBLE_NOFLAG
 * 	  TX
 * 		Flag 0x7E in payload : 1 bytes
 * 	  Config SI RX AIS
 * 		Preamble
 * 		  6 nibbles / 3 bytes
 * 		  Nstd 0x10=16+1=17 bits => threshold 16 bits
 * 		Flag
 * 		  0XCCCC
 * 		  4 bits / 1 byte
 * 		  Not transmitted
 * --------------------------------
 */
#define __PREAMBLE_FLAGCC
//#define __NSTDPREAMBLE_NOFLAG
//#define __NOPREAMBLE_FLAGCC
//#define AIS_PREAMBLE 0XAAAAAA
#define AIS_PREAMBLE 0X555555



//#define AIS_NRZI_INIT 1
#define AIS_NRZI_INIT 0

/*
 * =========================
 *  Classes
 * =========================
 */
class Utils
{
public:
	static float mnea_parse_lat_d(const char* decimal,const char hemisphere);
	static float mnea_parse_long_d(const char* decimal,const char hemisphere);
	static float arc_rad(const float lat1, const float lon1, const float lat2, const float lon2);
	static float distance_m(const float lat1, const float lon1, const float lat2, const float lon2);
	static float distance_nm(const float lat1_r,const float lon1_r,const float lat2_r,const float lon2_r);

	static uint16_t crc16(uint8_t* data,int16_t len);
	static uint16_t reverseBits(uint16_t data);
	static uint16_t reverseBits(uint8_t data);

	static uint32_t coordinate_uint32(const float value,const uint8_t numBits);
	static float uint32_coordinate(const uint32_t aisCoordinate,const uint8_t numBits);
	static void bytesToBits(uint8_t *byteArray,int16_t num_bytes,uint8_t * payload_pu8,int16_t numBits,bool lsb_first);
	static void putBits(uint8_t *bitVector,uint32_t value,int16_t numBits);
	static void msb2lsb_bit(uint8_t *bitVector,int16_t size);
	static void printhex(uint8_t *byteArray,int16_t size);
	static void printbits(uint8_t *bitVector,int16_t size);
	static void printbits_from_bytes(uint8_t *bitVector,int16_t bitnb_i16);
	static void test_endianess();
	static bool test_ascii(const char *str_p);

	static inline uint8_t reverseBits_u8(uint8_t word_u8)
	{
		uint8_t res_u8 = 0;
		res_u8|=((word_u8&0x80)>>7); // Bit7
		res_u8|=(((word_u8&0x40)>>6)<<1);
		res_u8|=(((word_u8&0x20)>>5)<<2);
		res_u8|=(((word_u8&0x10)>>4)<<3);
		res_u8|=(((word_u8&0x08)>>3)<<4);
		res_u8|=(((word_u8&0x04)>>2)<<5);
		res_u8|=(((word_u8&0x02)>>1)<<6);
		res_u8|=((word_u8&0x01)<<7);// Bit0
		return res_u8;
	}
};

class endianess_check
{
private:
	bool Litle_endian;
public:
	inline endianess_check()
	{
		uint32_t w_u32=1;
		uint8_t *bp=(uint8_t*)&w_u32;
		Litle_endian=((*bp)==0x01);
	}

	inline void print(){
		LOG_I("test_endianess: Litle_endian=%s\n",Litle_endian?"YES":"NO");
	}

	inline void bytes2bits(uint32_t value_u32,int16_t numBits_i16)
	{
		uint8_t * value_pu8=(uint8_t *)&value_u32;
#if DEV_MODE
		ASSERT((numBits_i16>(int16_t)0)&&(numBits_i16<=(int16_t)32));
#endif
		printf("value_u32 %d : ",value_u32);
		for (int16_t bit_id=0;bit_id<numBits_i16;bit_id++)
		{
			uint8_t bit_u8, byte_u8;
			byte_u8=value_pu8[bit_id>>3];
			bit_u8=(byte_u8>>(bit_id%8))&0x1;
			printf("%d",bit_u8);
			if (bit_u8&&(bit_u8%4)==0) printf("%d",bit_u8);
		}
		printf("\n");
	}
};






#endif //__AIS_UTILS_H__
