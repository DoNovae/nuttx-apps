/**
 * =====================================
 * utilities.cpp
 * -------------------------------------
 *  AIS DoNovae
 *  www.DoNovae.com
 *  Herve Bailly
 * =====================================
 */

#include "utilities.h"
#include "ais.h"
#include "ais_utils.h"
#include "Arduino.h"
#include "_assert.h"


char conv[UTILITY_CONV_SIZE];

#define DIGIT(n) ('0' + (n))
#define DIGIMOD(n, f) DIGIT(((n)/(f)) % 10)
#define RJDIGIT(n, f) ((n) >= (f) ? DIGIMOD(n, f) : ' ')
#define MINUSOR(n, alt) (n >= 0 ? (alt) : (n = -n, '-'))






/*
 * ========================
 *  Packet
 * ========================
 *
 *
 * ========================
 */

static const uint16_t CRC16_XMODEM_TABLE[] =
{ 0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7, 0x8108,
		0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef, 0x1231,
		0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6, 0x9339,
		0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de, 0x2462,
		0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485, 0xa56a,
		0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d, 0x3653,
		0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4, 0xb75b,
		0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc, 0x48c4,
		0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823, 0xc9cc,
		0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b, 0x5af5,
		0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12, 0xdbfd,
		0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a, 0x6ca6,
		0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41, 0xedae,
		0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49, 0x7e97,
		0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70, 0xff9f,
		0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78, 0x9188,
		0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f, 0x1080,
		0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067, 0x83b9,
		0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e, 0x02b1,
		0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256, 0xb5ea,
		0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d, 0x34e2,
		0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 0xa7db,
		0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c, 0x26d3,
		0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634, 0xd94c,
		0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab, 0x5844,
		0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3, 0xcb7d,
		0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a, 0x4a75,
		0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92, 0xfd2e,
		0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9, 0x7c26,
		0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1, 0xef1f,
		0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8, 0x6e17,
		0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0 };


Packet::~Packet()
{
	//if (mPacket) free((void*)mPacket);
	//if (bit_payload_pu8) free((void*)bit_payload_pu8);
	if (bit_payload_pu8) delete bit_payload_pu8;
	if (mPacket) delete mPacket;
	bit_size_i16=0;
}


Packet::Packet(int16_t nb_bits_i16)
{
	bit_size_i16=nb_bits_i16;
	//bit_payload_pu8=(uint8_t*)malloc(bit_size_i16);
	bit_payload_pu8=new uint8_t[bit_size_i16];
	//mPacket=(uint8_t *)malloc((bit_size_i16+1)>>3);
	mPacket=new uint8_t[(bit_size_i16+1)>>3];
	pk_reset();
}

Packet::Packet(const Packet &copy)
{
	bit_size_i16 = copy.bit_size_i16;
	index_i16=0;
	//if (bit_payload_pu8) free(bit_payload_pu8);
	//if (mPacket) free(mPacket);
	if (bit_payload_pu8) delete bit_payload_pu8;
	if (mPacket) delete mPacket;

	//bit_payload_pu8=(uint8_t*)malloc(bit_size_i16);
	bit_payload_pu8=new uint8_t[bit_size_i16];
	//mPacket=(uint8_t *)malloc((bit_size_i16+1)>>3);
	mPacket=new uint8_t[(bit_size_i16+1)>>3];
	for (int16_t i=0;i<(bit_size_i16+1)>>3;i++) mPacket[i]=copy.mPacket[i];
	for (int16_t i=0;i<bit_size_i16;i++) bit_payload_pu8[i]=copy.bit_payload_pu8[i];
}

Packet::Packet()
{
	mPacket=(uint8_t *)0;
	bit_size_i16=0;
	index_i16=0;
	bit_payload_pu8=(uint8_t*)0;
}



/*
 * -------------------------
 * convert2bytes
 * -------------------------
 * LSBIT_FISRT
 * --------------------------
 */
void Packet::convert2bytes()
{
	LOG_D("mPacket=%0#4x",(uint32_t)mPacket);
	LOG_D("bit_size_i16=%d - index_i16=%d - nb_bytes=%d",bit_size_i16,index_i16,nb_bytes());
	//memset((void*)mPacket,(int)0,nb_bytes());
	reset_mPacket();
	for (int16_t bit_id=0,byte_id=0,off_id=0;bit_id<index_i16;bit_id++,off_id=bit_id%8,byte_id=bit_id>>3)
	{
		LOG_D("bit_id=%d - byte_id=%d - off_id=%d",bit_id,byte_id,off_id);
		if (bit_payload_pu8[bit_id]) {
			mPacket[byte_id]|=(1<<off_id);
		}
	}
	LOG_D("bit_size_i16=%d",bit_size_i16);
}


void Packet::convert2bits()
{
	for (int16_t bit_id=0,byte_id=0,off_id=0;bit_id<bit_size_i16;bit_id++,off_id=bit_id%8,byte_id=bit_id>>3)
	{
		bit_payload_pu8[bit_id]=(mPacket[byte_id]>>off_id)&0x1;
	}
}

void Packet::print_bits(){
	LOG_V("bit_size_i16(%d)",bit_size_i16);
	for (int16_t bit_id=0,byte_id=0,off_id=0;bit_id<bit_size_i16;bit_id++,off_id=bit_id%8,byte_id=bit_id>>3)
	{
		if ((bit_id%32)==0) {
			printf("\n");
			for (int16_t i=0;i<4;i++)
				printf("%d       ",i);
			printf("\n");
		}
		printf("%d",(mPacket[byte_id]>>off_id)&0x1);
	}
	printf("\n");
}

void Packet::print_bits2(){
	LOG_V("index(%d)",index_i16);
	for (int16_t bit_id=0; bit_id<index_i16;bit_id++)
	{
		if ((bit_id%32)==0) {
			printf("\n");
			for (int16_t i=0;i<4;i++)
			{
				printf("%d       ",i);
			}
			printf("\n");
		}
		printf("%d",bit_payload_pu8[bit_id]);
	}
	printf("\n");
}


void Packet::print_bytes(){
	LOG_V("nb_bytes(%d)",nb_bytes());
	for (int16_t i=0; i<16;i++)
	{
		printf("%d  ",i%4);
	}
	printf("\n");
	for (int16_t byte_id=0;byte_id<nb_bytes();byte_id++)
	{
		if ((byte_id%16)==0)
		{
			printf("\n");
		}
		printf("%02x ",mPacket[byte_id]);
	}
	printf("\n");
}



uint8_t Packet::nextBit()
{
#if DEV_MODE
	ASSERT((int16_t)index_i16 < (int16_t)bit_size_i16);
#endif

	uint16_t index = index_i16 / 8;
	uint8_t offset = index_i16 % 8;

	index_i16++;
	return (mPacket[index] & ( 1 << offset )) != 0;
}


void Packet::pad()
{
	int16_t rem=8-(index_i16%8);
#if DEV_MODE
	ASSERT((int16_t)index_i16+rem < (int16_t)bit_size_i16);
#endif
	for ( int16_t i=0;i<rem;i++)
		add_bit(0);
}

void Packet::pad2()
{
	int16_t rem=8-(index_i16%8);
#if DEV_MODE
	ASSERT((int16_t)index_i16+rem < (int16_t)bit_size_i16);
#endif
	for ( int16_t i=0;i<rem;i++)
		add_bit2(0);
}

bool Packet::eof()
{
	if ( bit_size_i16 == 0 )
		return true;

	return index_i16 > bit_size_i16 - 1;
}



void Packet::pk_reset()
{
	index_i16 = 0;
	if (mPacket) reset_mPacket();
	if (bit_payload_pu8) reset_bit_payload();
}




/*
 * ===============================
 * add_byte
 * ===============================
 *	-Input:
 *		byte_u8
 *	-Output:
 *		mPacket: bytes flipped
 * ===============================
 */
void Packet::add_byte(uint8_t byte_u8)
{
	/*
	 * Fill mPacket with addbit
	 * The payload is LSB_FIRST bit order.
	 */
	add_bit((byte_u8&0x80)>>7); // Bit7
	add_bit((byte_u8&0x40)>>6);
	add_bit((byte_u8&0x20)>>5);
	add_bit((byte_u8&0x10)>>4);
	add_bit((byte_u8&0x08)>>3);
	add_bit((byte_u8&0x04)>>2);
	add_bit((byte_u8&0x02)>>1);
	add_bit(byte_u8&0x01); 		// Bit0
}


/*
 * ===========================
 * add_bit
 * ===========================
 *  - Byte order unchanged
 *  	* Byte mPacket : 0 ...> n
 *  	* bit LSB_FIRST : 0 ...> 7
 * ===========================
 */

void Packet::add_bit(uint8_t bit_u8)
{
	int16_t idx_i16 = index_i16>>3;
	int16_t off_i16 = index_i16%8;
#if DEV_MODE
	ASSERT(index_i16+1<(int16_t)bit_size_i16);
#endif

	if ( bit_u8 & 0x1 )
		mPacket[idx_i16] |= (1<<off_i16);
	else
		mPacket[idx_i16] &= ~(1<<off_i16);

	index_i16++;
}


void Packet::add_bit2(uint8_t bit_u8)
{
#if DEV_MODE
	ASSERT(index_i16+1<(int16_t)bit_size_i16);
#endif
	bit_payload_pu8[index_i16]= bit_u8;
	index_i16++;
}


/*
 * ==================================
 * bytes2bits
 * ==================================
 * -Add bits from bytes LSBIT_FIRST
 * -Byte endian
 * 	      MSB        LSB
 * 	 MSB  1 2 3 ..7  8 9 ...15
 * 	      byte_id=bit_id>>3
 *   LSB  8 9 ...15  1 2 3 ..7
 *        byte_id=(num_bits-bit_id-1)>>3
 *   Conversions:
 *      LSB->LSB:
 *      LSB->MSB: swap endian byte index from input to output
 *      MSB->MSB: swap endian byte index from input to output
 *      MSB->LSB:
 * ==================================
 */

/*
 * ------------------------
 * add_bytes
 * ------------------------
 * Place bit in order
 * 	LSBitfirst
 * 	MSByte end
 * ESP32
 * 	MSByte end
 * 	Take byte in increasing order
 * 	in value_u32.
 * ------------------------
 */
void Packet::add_bytes_lsbfirst(uint32_t value_u32,int16_t numBits_i16)
{
	uint8_t * value_pu8=(uint8_t *)&value_u32;
#if DEV_MODE
	ASSERT((numBits_i16>(int16_t)0)&&(numBits_i16<=(int16_t)32));
	ASSERT(numBits_i16<(int16_t)bit_size_i16);
#endif
	LOG_V("value_u32(%d) - numBits_i16(%d)",value_u32,numBits_i16);
	LOG_V("index_i16(%d)",index_i16);
	for (int16_t bit_id=0,off_id=(index_i16%8),byte_id=(index_i16>>3);bit_id<numBits_i16;bit_id++,index_i16++,off_id=(index_i16%8),byte_id=(index_i16>>3))
	{
		uint8_t bit_u8, byte_u8;
		byte_u8=value_pu8[bit_id>>3];
		bit_u8=(byte_u8>>(bit_id%8))&0x1;
		if (bit_u8){
			mPacket[byte_id]|=(uint8_t)(1<<off_id);
		} else{
			mPacket[byte_id]&= ~((uint8_t)(1<<off_id));
		}
	}
}


void Packet::add_bytes_msbfirst(uint32_t value_u32,int16_t numBits_i16)
{
	uint8_t * value_pu8=(uint8_t *)&value_u32;
#if DEV_MODE
	ASSERT((numBits_i16>(int16_t)0)&&(numBits_i16<=(int16_t)32));
	ASSERT(numBits_i16<(int16_t)bit_size_i16);
#endif
	LOG_V("value_u32(%d) - numBits_i16(%d)",value_u32,numBits_i16);
	LOG_V("index_i16(%d)",index_i16);
	for (int16_t bit_id=numBits_i16-1,off_id=(index_i16%8),byte_id=(index_i16>>3);0<=bit_id;bit_id--,index_i16++,off_id=(index_i16%8),byte_id=(index_i16>>3))
	{
		uint8_t bit_u8, byte_u8;
		byte_u8=value_pu8[bit_id>>3];
		bit_u8=(byte_u8>>(bit_id%8))&0x1;
		if (bit_u8){
			mPacket[byte_id]|=(uint8_t)(1<<off_id);
		} else{
			mPacket[byte_id]&= ~((uint8_t)(1<<off_id));
		}
	}
}

/*
 * --------------------------------
 *
 * --------------------------------
 *
 * --------------------------------
 */
void Packet::flip_bits()
{
	uint8_t byte_u8;
	for (uint16_t idx_i16=0;idx_i16<nb_bytes();idx_i16++){
		byte_u8=mPacket[idx_i16];
		mPacket[idx_i16]=0;
		mPacket[idx_i16]|=(byte_u8&0x80)>>7;// Bit7
		mPacket[idx_i16]|=(byte_u8&0x40)>>5;// Bit6
		mPacket[idx_i16]|=(byte_u8&0x20)>>3;// Bit5
		mPacket[idx_i16]|=(byte_u8&0x10)>>1;// Bit4
		mPacket[idx_i16]|=(byte_u8&0x08)<<1;// Bit3
		mPacket[idx_i16]|=(byte_u8&0x04)<<3;// Bit2
		mPacket[idx_i16]|=(byte_u8&0x02)<<5;// Bit1
		mPacket[idx_i16]|=(byte_u8&0x01)<<7; 	// Bit0
	}
}


/*
 * --------------------------------
 * add_bits
 * --------------------------------
 * !!! In bit_payload_pu8 !!!
 * LSBIT_FIRST
 * --------------------------------
 */
void Packet::add_bits(uint32_t value_u32,int16_t numBits_i16)
{
	uint8_t * value_pu8=(uint8_t *)&value_u32;
#if DEV_MODE
	ASSERT((numBits_i16>(int16_t)0)  && (numBits_i16<=(int16_t)32));
	ASSERT(numBits_i16<(int16_t)bit_size_i16);
#endif
	for (int16_t bit_id=0;bit_id<numBits_i16;bit_id++,index_i16++)
	{
		uint8_t bit_u8, byte_u8;
		byte_u8=value_pu8[bit_id>>3];
		bit_u8=(byte_u8>>(bit_id%8))&0x1;
		bit_payload_pu8[index_i16]=bit_u8;
	}
}

/*
 * ---------------------------
 * get_bit
 * ---------------------------
 * !!! In mPacket !!!
 * ---------------------------
 */
uint8_t Packet::get_bit(int16_t bit_pos_u16) const
{
#if DEV_MODE
	ASSERT(bit_pos_u16<(int16_t)bit_size_i16);
#endif
	int16_t idx_i16=bit_pos_u16>>3;
	int16_t off_i16=bit_pos_u16%8;

	if (mPacket[idx_i16]&((uint8_t)1<<off_i16))
	{
		return (uint8_t)1;
	} else
	{
		return (uint8_t)0;
	}
}

/*
 * ---------------------------
 * get_bit2
 * ---------------------------
 * !!! In bit_payload_pu8 !!!
 * ---------------------------
 */
uint8_t Packet::get_bit2(int16_t bit_pos_u16) const
{
#if DEV_MODE
	ASSERT(bit_pos_u16<(int16_t)bit_size_i16);
#endif

	return bit_payload_pu8[bit_pos_u16];
}




void Packet::copy_in(uint8_t *data_pu8,int16_t nb_bytes_i16)
{
#if DEV_MODE
	ASSERT(nb_bytes_i16 <= (bit_size_i16+1)>>3);
#endif
	pk_reset();
	for (int16_t i=0;i<nb_bytes_i16;i++) {
		mPacket[i]=data_pu8[i];
		uint8_t byte_u8=data_pu8[i];
		for (uint8_t b_u8=0;b_u8<8;b_u8++){
			bit_payload_pu8[index_i16]=byte_u8&0x1;
			byte_u8>>=1;
			index_i16++;
		}
	}
}

/*
 * -------------------------
 * crc16
 * -------------------------
 *  ISO/IEC 3309
 * 		input: MSBIT_FIRST
 * 	=> Must be revert into LSBIT_FIRST before
 * 		output: LSBIT_FIRST
 * -------------------------
 */

uint16_t Packet::crc16(int16_t start_i16)
{
    uint16_t crc=0xffff;
    for ( uint16_t b=start_i16;b<((index_i16+1)>>3);++b) {
        crc=((crc<<8)&0xff00)^CRC16_XMODEM_TABLE[((crc >> 8)&0xff)^Utils::reverseBits_u8(mPacket[b])];
    }
    return (~crc);
}



uint32_t Packet::reverse_bytes(uint32_t word_u32) const
{
	uint32_t res_u32=0;
	for (int16_t i=0;i<(int16_t)sizeof(word_u32);i++){
		((uint8_t*)&res_u32)[i]=((uint8_t*)&word_u32)[sizeof(word_u32)-i-1];
	}
	return res_u32;
}


/*
 * -------------------------
 * add_str
 * -------------------------
 * In mPacket
 * Add bytes MSBit first
 * -------------------------
 */
void Packet::add_str(const char *str_p,uint16_t nb_bit)
{
	uint8_t mum_chars_u8=strlen(str_p);
#if DEV_MODE
	LOG_D("mum_chars_u8(%d) - nb_bit(%d)",mum_chars_u8,nb_bit);
	ASSERT(mum_chars_u8*6<=nb_bit);
	ASSERT(nb_bit%6==0);
#endif
	char s[mum_chars_u8];
	memset(s,0,sizeof(s));
	strncpy(s,str_p,strlen(str_p));

	/*
	 * ASCII8 to ASCII6
	 * 63 chars in [32,95] ASCII
	 */
	uint8_t c=0;
	for (c=0;c<mum_chars_u8;++c) {
		uint8_t byte=(s[c]>64)?s[c]-64:s[c];
		LOG_V("%c - byte(%d)",s[c],byte);
		add_bytes_msbfirst(byte,6);
	}
	for (;c<nb_bit/6;++c) {
		uint8_t byte='@';
		LOG_V("%c - byte(%d)",byte,byte);
		add_bytes_msbfirst(byte,6);
	}
}


/*
 * -----------------------
 * field2u8_lsbirst
 * -----------------------
 * !!! LSBit first !!!
 * In mPacket
 * -----------------------
 */

uint8_t Packet::field2u8_lsbfirst(uint8_t start_bit, uint8_t nb_bit) const
{
	uint8_t rtn_u8=0;

#if DEV_MODE
	ASSERT(nb_bit<9);
	ASSERT((start_bit+nb_bit)<bit_size_i16);
#endif

	for (int idx=start_bit,i=0;idx<(start_bit+nb_bit);idx++,i++) {
		if (get_bit(idx))
		{
			rtn_u8|=(1<<i);
		}
	}
	LOG_V("0x%02x",rtn_u8);
	return rtn_u8;
}

uint8_t Packet::field2u8(uint8_t start_bit, uint8_t nb_bit) const
{
	uint8_t rtn_u8=0,idx_u8=0,offset_u8=0;

#if DEV_MODE
	ASSERT(nb_bit<9);
	ASSERT((start_bit+nb_bit)<bit_size_i16);
#endif
	idx_u8=start_bit>>3;
	offset_u8=start_bit%8;
	rtn_u8|=(uint8_t)(mPacket[idx_u8]>>offset_u8);
	rtn_u8|=(uint8_t)(mPacket[idx_u8+1]<<(8-offset_u8));
	rtn_u8&=(uint8_t)(((uint16_t)1<<nb_bit)-1);

	LOG_V("0x%02x",rtn_u8);
	return rtn_u8;
}


/*
 * -----------------------
 * field2u8_msbirst
 * -----------------------
 * !!! MSBit first !!!
 * In mPacket
 * -----------------------
 */
uint8_t Packet::field2u8_msbfirst(uint8_t start_bit, uint8_t nb_bit) const
{
	uint8_t rtn_u8=0;

#if DEV_MODE
	ASSERT(nb_bit<9);
	ASSERT((start_bit+nb_bit)<bit_size_i16);
#endif

	for (int idx=start_bit;idx<(start_bit+nb_bit);idx++) {
		rtn_u8<<=1;
		rtn_u8|=get_bit(idx);
	}
	LOG_V("0x%02x",rtn_u8);
	return rtn_u8;
}


/*
 * -----------------------
 * field2u32_lsbfirst
 * -----------------------
 * !!! LSBit first !!!
 * In mPacket
 * -----------------------
 */


uint32_t Packet::field2u32_lsbfirst(uint16_t start_bit, uint8_t nb_bit) const
{
	uint8_t *msb_pu8;
	uint32_t rtn_u32=0;

#if DEV_MODE
	ASSERT(nb_bit<33);
	ASSERT((start_bit+nb_bit)<bit_size_i16);
#endif

	msb_pu8=(uint8_t*)&rtn_u32;
	for (int idx=start_bit,i=0,byte=0,bit=0;i<nb_bit;idx++,i++,bit=i%8,byte=(i>>3)) {
		if (get_bit(idx))
		{
			msb_pu8[byte]|=(1<<bit);
		}
	}
	LOG_V("0x%08x",rtn_u32,rtn_u32);

	return rtn_u32;
}


uint32_t Packet::field2u32(uint16_t start_bit, uint8_t nb_bit) const
{
	uint8_t *msb_pu8;
	uint32_t rtn_u32=0;
	uint8_t idx_u8=0,offset_u8=0,byte_n_u8=0;

#if DEV_MODE
	ASSERT(nb_bit<33);
	ASSERT((start_bit+nb_bit)<bit_size_i16);
#endif

	msb_pu8=(uint8_t*)&rtn_u32;
	idx_u8=start_bit>>3;
	offset_u8=start_bit%8;
	byte_n_u8=(nb_bit>>3)+1;
	for (uint8_t i=0;i<byte_n_u8;i++,idx_u8++){
		msb_pu8[i]|=(uint8_t)(mPacket[idx_u8]>>offset_u8);
		msb_pu8[i]|=(uint8_t)(mPacket[idx_u8+1]<<(8-offset_u8));
		//LOG_V("mPacket=0x%02x-0x%02x - msb_pu8[i]=0x%02x-0x%02x",mPacket[idx_u8],mPacket[idx_u8+1],(uint8_t)(mPacket[idx_u8]>>offset_u8),(uint8_t)(mPacket[idx_u8+1]<<(8-offset_u8)));
	}
	msb_pu8[byte_n_u8-1]&=(uint8_t)(((uint16_t)1<<(nb_bit%8))-1);

	LOG_V("0x%08x",rtn_u32,rtn_u32);

	return rtn_u32;
}



uint32_t Packet::field2u32b(uint16_t start_bit, uint8_t nb_bit) const
{
	uint8_t *msb_pu8;
	uint32_t rtn_u32=0;
	uint8_t idx_u8=0,byte_n_u8=0;

#if DEV_MODE
	ASSERT(nb_bit<33);
	ASSERT((start_bit+nb_bit)<bit_size_i16);
#endif

	msb_pu8=(uint8_t*)&rtn_u32;
	idx_u8=(start_bit+nb_bit-8)>>3;
	byte_n_u8=(nb_bit>>3)+1;
	for (uint8_t i=0;i<byte_n_u8;i++,idx_u8--){
		msb_pu8[i]|=mPacket[idx_u8];
	}
	if (nb_bit%8) msb_pu8[byte_n_u8-1]>>=(uint8_t)(8-(nb_bit%8));

	LOG_V("0x%08x",rtn_u32,rtn_u32);

	return rtn_u32;
}

/*
 * -----------------------
 * field2u32_msbfirst
 * -----------------------
 * !!! MSBit first !!!
 * In mPacket
 * -----------------------
 */
uint32_t Packet::field2u32_msbfirst(uint16_t start_bit, uint8_t nb_bit) const
{
	uint8_t *msb_pu8;
	uint32_t rtn_u32=0;

#if DEV_MODE
	ASSERT(nb_bit<33);
	ASSERT((start_bit+nb_bit)<bit_size_i16);
#endif

	msb_pu8=(uint8_t*)&rtn_u32;
	for (int idx=start_bit,i=nb_bit-1,byte=(i>>3),bit=i%8;i>=0;idx++,i--,bit=i%8,byte=(i>>3)) {
		if (get_bit(idx))
		{
			msb_pu8[byte]|=(1<<bit);
		}
	}
	LOG_V("0x%08x",rtn_u32,rtn_u32);

	return rtn_u32;
}


/*
 * -----------------------
 * field2str_msbfirst
 * -----------------------
 * !!! MSBit first !!!
 * In mPacket
 * -----------------------
 */
void Packet::field2str_msbfirst(char * str_p,uint8_t mum_chars_u8,uint16_t start_bit, uint16_t nb_bit) const
{
#if DEV_MODE
	LOG_V("mum_chars_u8(%d) - nb_bit(%d)",mum_chars_u8,nb_bit);
	ASSERT(nb_bit==(mum_chars_u8)*6);
	ASSERT((start_bit+nb_bit)<bit_size_i16);
#endif

	for (uint16_t idx=start_bit,i=0,byte=0,byte_id=0,bit_id=0; i<nb_bit; idx++,i++,bit_id=i%6,byte_id=i/6) {
		if (bit_id==0){
			byte=0;
		}
		/*
		 * MSBit first
		 */
		byte<<=1;
		byte|=get_bit(idx);
		if (bit_id==5){
			LOG_V("0x%02x",byte);
			/*
			 * ASCII6 to ASCII8
			 * Char in ASCII [64,95] and [32,63]
			 */
			byte=(byte<=31)?byte+64:byte;
			str_p[byte_id]=(byte=='@')?0:byte;
			LOG_V("%c - %02x",str_p[byte_id],byte);
		}
	}
}

void Packet::field2str_lsbfirst(char * str_p,uint8_t mum_chars_u8,uint16_t start_bit, uint16_t nb_bit) const
{
#if DEV_MODE
	LOG_V("mum_chars_u8(%d) - nb_bit(%d)",mum_chars_u8,nb_bit);
	ASSERT(nb_bit==(mum_chars_u8)*6);
	ASSERT((start_bit+nb_bit)<bit_size_i16);
#endif

	for (uint16_t idx=start_bit,i=0,byte=0,byte_id=0,bit_id=0; i<nb_bit; idx++,i++,bit_id=i%6,byte_id=i/6) {
		if (bit_id==0){
			byte=0;
		}
		/*
		 * LSBit first
		 */
		if (get_bit(idx))
		{
			byte|=(1<<bit_id);
		}
		if (bit_id==5){
			LOG_V("0x%02x",byte);
			/*
			 * ASCII6 to ASCII8
			 * Char in ASCII [64,95] and [32,63]
			 */
			byte=(byte<=31)?byte+64:byte;
			str_p[byte_id]=(byte=='@')?0:byte;
			LOG_V("%c - %02x",str_p[byte_id],byte);
		}
	}
}




