/**
 * =====================================
 * rx_packet.cpp
 * -------------------------------------
 *  AIS DoNovae
 *  www.DoNovae.com
 *  Herve Bailly
 * =====================================
 */
#include <cassert>
#include <algorithm>// Min max
#include "rx_packet.h"

#include "utilities.h"
#include "ais_utils.h"



RXPacket::RXPacket (int16_t nb_bits_i16 ):Packet{nb_bits_i16}
{
#if DEV_MODE
	ASSERT((int16_t)nb_bits_i16<=(int16_t)MAX_AIS_RX_PACKET_SIZE);
#endif
	reset();
}

RXPacket::RXPacket ():Packet{}
{
	reset();
	mChannel=CH_28;
}

RXPacket::~RXPacket ()
{
}


RXPacket::RXPacket(const RXPacket &copy) : Packet{copy}
{
	mCRC=copy.mCRC;
	mRSSI=copy.mRSSI;
	mChannel=copy.mChannel;
}
/*
 * Reset mCRC, index and buffers
 */
void RXPacket::reset()
{
	mCRC=0xffff;
	mRSSI=0;
	mmsi=0;

	pk_reset();
}



/*
 * ===============================
 * addBitCRC
 * ===============================
 *
 * ===============================
 */
void RXPacket::addBitCRC(uint8_t bit_u8)
{
	if ( (bit_u8 ^ mCRC) & 0x0001 )
		mCRC = (mCRC >> 1) ^ 0x8408;
	else
		mCRC >>= 1;
}



/*
 * ===============================
 * decodeCRC
 * ===============================
 */
void RXPacket::decodeCRC(uint8_t byte_u8)
{
	/*
	* Now we can update our CRC in MSB first bit order
	* which is how it was calculated during encoding by the sender ...
	*/
	addBitCRC(byte_u8&0x01); 		// Bit0
	addBitCRC((byte_u8&0x02)>>1);
	addBitCRC((byte_u8&0x04)>>2);
	addBitCRC((byte_u8&0x08)>>3);
	addBitCRC((byte_u8&0x10)>>4);
	addBitCRC((byte_u8&0x20)>>5);
	addBitCRC((byte_u8&0x40)>>6);
	addBitCRC((byte_u8&0x80)>>7); // Bit7
}



bool RXPacket::isBad() const
{
	/*
	 * We don't anticipate anything less than 168 + 16 = 184 bits
	 */

	//return bit_size_i16 < 184;
	return (int16_t)bit_size_i16<(int16_t)64;
}


void RXPacket::discardCRC()
{
	if ( mCRC == 0xffff )
		return;
	bit_size_i16 -= 16;
	mCRC = 0xffff;
}


bool RXPacket::checkCRC() const
{
	LOG_V("mCRC=0x%04x",mCRC);
	return mCRC == CRC_LSBIT_FISRT;
}





/*
 * ============================
 * ais_get_nrzi_bits
 * ============================
 *  -Last bit = last bit of flag 7Eh = 0
 *  -LSBIT_FIRST
 *  	-FIFO is LSBIT_FIRST cf PKT_CONFIG1 0x02
 *  	-SPI is MSBIT_FIRST for communication
 *  	-Arduino get FIFO LSBIT_FIRST data
 *  -Decode NRZI
 *  -Detect flag
 *  	-NRZI
 *  	-0x7E, bit reversed equal to 0x7E
 *  	-Stuffing prevents from any other 0x7E word in the data
 *  -NRZi: NRZi(bit)=(bit(prev)XOR bit)
 * ============================
 */

/*
 * LAST_NRZI_BIT of word sync 0xCC LSBit first
 *   LAST_NRZI_BIT=1
 * WINDOW_INIT with last NRZI 0xC
 *   -> 0x5 deNRZI
 */
#define LAST_NRZI_BIT 1
#define WINDOW_INIT 0x5
#define SYNC_WORD_PATTERN AIS_HDLC_FLAG


bool RXPacket::ais_get_nrzi_bits()
{
	bool end,start;
	int16_t idx_i16,start_i16;
	uint8_t flag_u8,bit_u8,last_u8,cur_u8;
	uint8_t window_u8;

	idx_i16=0;
	start_i16=0;
	end=false;
	start=false;
	flag_u8=0;
	//window_u8=WINDOW_INIT;
	window_u8=0;

	/*
	 * Sync flag
	 * 	 First 4 bits of 0xCC=0011 (first bit left)
	 * 	 LAST_NRZI_BIT=1
	 */
	last_u8=LAST_NRZI_BIT;

	while ((!end)&&(idx_i16<bit_size_i16)&&(idx_i16<(int16_t)MAX_AIS_RX_PACKET_SIZE)){
		// Decode NRZ
		cur_u8=get_bit(idx_i16);
		bit_u8=!(cur_u8^last_u8);
		LOG_V("last_u8=%d - get_bit(idx_i16)=%d - bit_u8=%d",last_u8,get_bit(idx_i16),bit_u8);
		last_u8=cur_u8;
		if (!start){
			/*
			 * Check for SYNC_WORD_PATTERN
			 */
			window_u8 <<= 1;
			window_u8|=bit_u8;
			LOG_V("%d - window_u16=%#04x",idx_i16,window_u8);
			if (window_u8==SYNC_WORD_PATTERN)
			{
				start=true;
				start_i16=idx_i16+1;
				LOG_V("sync word - start_i16=%d",start_i16);
			}
		} else {
			bit_payload_pu8[idx_i16-start_i16]=bit_u8;

			// 1st shift dummy
			flag_u8<<=1;
			flag_u8|=bit_u8;
			/*
			 * AIS_HDLC_FLAG_REVERSE==AIS_HDLC_FLAG
			 */
			end=(flag_u8==AIS_HDLC_FLAG);
			//LOG_V("flag(%x) - bid(%d) - end(%d)",flag,bid,end);
			//LOG_V("bit_i16=%d - bit_payload_pu8[%d]=%d - last_i16=%d",bit_i16,idx_i16,bit_payload_pu8[idx_i16],last_i16);
		}
		idx_i16++;
	}
	index_i16=(end)?idx_i16-8:idx_i16; // Substract 8 bits for the flag
	index_i16-=start_i16;
	return end;
}



/*
 * ==========================
 * rx_finalize
 * ==========================
 *  -Packet from SI446X
 *  	-Start after flag 0x7F
 *  	-CRC included
 *  	-Up to end flag 0x7F included
 *  	-Data NRZI encoded +bit stuffing
 * 	-Set msize in bytes
 * 	-NRZi decode
 * 	-Unstuff
 * 	-Check CRC
 * 	-Ready for OpenCpn
 * ==========================
 */

void RXPacket::ais_finalize()
{
	bool flag_detected;
	//print_bytes();
	//print_bits();

	/*
	 * Demodulate NZRI
	 * Detection flag
	 * Set index
	 * Convert2bits
	 *
	 */
	flag_detected=ais_get_nrzi_bits();
	LOG_D("ais_get_nrzi_bits(%d)",index());
	//print_bits2();

	if (!flag_detected) return;

	/*
	 * UnStuff
	 * Set index
	 * Convert to bytes
	 * !!! Copy AIS frame into mPacket
	 * at byte 0 !!!
	 */
	ais_fillpk_bitUnStuff();
	LOG_D("ais_fill_bitUnStuff(%d):",index());
	//print_bits();
	/*
	 * Trunc CRC
	 */
	index(index()-16);

	if (checkCRC()){
		//print_bytes();
		//print_bits();
	}
	LOG_I("crc(%x)-%s",crc(),checkCRC()?"OK":"KO");
}


/*
 * ==========================
 * ais_fillpk_bitUnStuff
 * ==========================
 * - Unstuff
 * - Convert to bytes
 * - Reverse order of payload in decodeCRC:
 *  	* Byte received  : 0 ...> n
 *  	  LSBIT_FIRST    : 0 ...> 7
 *  	* Byte after     : 0 ...> n
 *  	  LSBIT_FIRST    : 0 ...> 7
 *
 * ==========================
 */
void RXPacket::ais_fillpk_bitUnStuff()
{
	int16_t mOneBitCount,len_i16,mBitCount_i16;
	uint8_t bit_u8,byte_u8;
	bool unstuff;

	len_i16=index_i16;
	index_i16=0;
	mBitCount_i16=0;
	mOneBitCount=0;
	byte_u8=0;
	unstuff=false;
	reset_mPacket();

	for (int16_t i=0;i<len_i16;i++)
	{
		bit_u8=bit_payload_pu8[i];
		unstuff=false;

		if (bit_u8==1)
		{
			mOneBitCount++;
		} else
		{
			/*
			 * if 0 then mOneBitCount
			 * If 5 ones behind,then unstuff
			 */
			if (mOneBitCount==(int16_t)5){
				// Next bit is a dummy 0 - Unstuff
				unstuff=true;
			}
			mOneBitCount=0;
		}

		if (!unstuff)
		{
			/*
			 * LSBIT_FIRST
			 * The first shift on 0 is dummy
			 */
			if (bit_u8)
			{
				byte_u8|=(1<<mBitCount_i16);
			}

			mBitCount_i16++;
			if (mBitCount_i16==(int16_t)8){
				//LOG_V("mRXByte(%#02x)-mRXByte0(%#02x)",bit_size_i16,mRXByte,mRXByte1);
				/*
				 * Commit to the packet
				 * The payload is LSB (inverted MSB bytes).
				 * This brings it back into MSB format.
				 */
				//LOG_V("byte_lsbfirst_u8(%d) - mCRC(%d)",byte_lsbfirst_u8,mCRC);
				add_byte(byte_u8);
				decodeCRC(byte_u8);
				mBitCount_i16=0;
				byte_u8=0;
			}
		}
	}
}







