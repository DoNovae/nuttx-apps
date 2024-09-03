/**
 * =====================================
 * tx_packet.cpp
 * -------------------------------------
 *  AIS DoNovae
 *  www.DoNovae.com
 *  Herve Bailly
 * =====================================
 */

#include <cstring>
#include <cassert>
#include "tx_packet.h"

#include "utilities.h"
#include "ais_utils.h"


TXPacket::TXPacket(const TXPacket &copy) : Packet{copy}
{
	mChannel = copy.mChannel;
	mTXTime = copy.mTXTime;
}

TXPacket::TXPacket(const Packet &packet) : Packet{packet}
{
	reset();
}

TXPacket::TXPacket (int16_t nb_bits_i16 ) :Packet{nb_bits_i16}
{
#if DEV_MODE
	ASSERT((int16_t)nb_bits_i16<=(int16_t)MAX_AIS_RX_PACKET_SIZE);
#endif
	reset();
}

TXPacket::TXPacket ():Packet{}
{
	reset();
	mChannel  = CH_28;
}

void TXPacket::configure(VHFChannel channel, time_t txTime)
{
	mChannel=channel;
	mTXTime=txTime;
}

TXPacket::~TXPacket ()
{
}

/*
 * Reset buffer and index
 */
void TXPacket::reset()
{
	mTXTime=0;
	pk_reset();
}



time_t TXPacket::txTime()
{
	return mTXTime;
}

VHFChannel TXPacket::channel()
{
	return mChannel;
}

void TXPacket::setChannel(VHFChannel channel)
{
	mChannel=channel;
}

/*
 * ===============================
 * ais_finalize
 * ===============================
 *
 *
 * ===============================
 */

#if defined(__PREAMBLE_FLAGCC) || defined(__NOPREAMBLE_FLAGCC)
#define CRC_START 4
#else
#define CRC_START 1
#endif

void TXPacket::ais_finalize()
{
	/*
	 * Flip bytes
	 */
	//print_bits();
	flip_bits();
	//print_bits();
	/*
	 * Byte packet
	 * CRC-CCITT calculation
	 */
	LOG_D("payloadToBytes-crc(%d):",index());
	//print_bytes();
	/*
	 * Cr16 in MSBit first
	 */
	uint16_t crc=crc16(CRC_START);
	/*
	 * Reverse to LSB first + MSByte
	 */
	uint8_t crcL=Utils::reverseBits_u8(crc&0x00ff);
	uint8_t crcH=Utils::reverseBits_u8((crc&0xff00)>>8);
	add_bytes_lsbfirst((uint32_t)(crcH),8);
	add_bytes_lsbfirst((uint32_t)(crcL),8);
	LOG_D("crc(0x%02x%02x) - index %d",crcH,crcL,index());
	//print_bytes();
	//print_bits();

	/*
	 * Now in bits
	 */
	convert2bits();
	ais_bitStuff();
	LOG_D("ais_bitStuff(%d):",index());
	//print_bits2();

	/*
	 * Bits packet
	 * Now append the end marker and ramp-down bits.
	 */
	add_bits((uint32_t)AIS_HDLC_FLAG,8); // HDLC stop flag
	add_bits((uint32_t)AIS_RAMP_DOWN,3); // Ramp down
	LOG_D("flag(%d):",index());
	//print_bits2();

	ais_nrziEncode();
	LOG_D("ais_nrziEncode(%d):",index());
	//print_bits();

	pad();
	LOG_D("pad(%d):",index());
	//print_bits();
	//print_bytes();
}


/*
 * ============================
 * ais_nrziEncode
 * ============================
 *   Last bit = last bit of flag 7Eh = 0
 *   Coding:
 *   	transition <-> 0
 *   	no transition <-> 1
 *   Convert to bytes
 * ============================
 */



void TXPacket::ais_nrziEncode()
{
	uint8_t prevBit;
#if DEV_MODE
	ASSERT(index_i16<=(int16_t)bit_size_i16);
#endif

	int16_t bit_nb_i16=index_i16;
	reset_mPacket();
	prevBit=AIS_NRZI_INIT;
	for (int16_t i=0,index_i16=0;i<bit_nb_i16;i++,index_i16++) {
		if (bit_payload_pu8[i]){
			mPacket[index_i16>>3]|=prevBit<<(index_i16%8);
		} else
		{
			mPacket[index_i16>>3]|=(!prevBit)<<(index_i16%8);
			prevBit=!prevBit;
		}
	}
}


#if defined(__PREAMBLE_FLAGCC) || defined(__NOPREAMBLE_FLAGCC)
#define BITSTUF_START 32
#else
#define BITSTUF_START 8
#endif

void TXPacket::ais_bitStuff()
{
	int16_t numOnes=0;
	/*
	 * Start after preamble 24 bits + sync word 8 bits
	 */
	const int16_t start_i16=BITSTUF_START;
	for (int16_t i=start_i16;i<index_i16;i++) {
		switch(bit_payload_pu8[i]) {
		case 0:
			numOnes = 0;
			break;
		case 1:
			++numOnes;
			if (numOnes==(int16_t)5) {
#if DEV_MODE
				ASSERT(index_i16+1<=(int16_t)bit_size_i16);
#endif
				// Insert a 0 right after this one
				memmove(bit_payload_pu8+i+2,bit_payload_pu8+i+1,index_i16-i-1);
				bit_payload_pu8[i+1]=0;
				LOG_V("ais_bitStuff(%d):",i);
				index_i16++;
			}
			break;
		default:
			ASSERT(false);
		}
	}
}




