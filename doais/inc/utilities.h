

#include <stdint.h>
#include "types.h"

#ifndef UTILITY_H
#define UTILITY_H



#define UTILITY_CONV_SIZE 11


extern char conv[UTILITY_CONV_SIZE];
//inline void utility_reset_conv(){memset(conv, 0, UTILITY_CONV_SIZE);};

#define UTIL_MSB_ENDIAN false
#define UTIL_LSBIT_FIRST true

/*
 * ========================
 *  Packet
 * ========================
 */
class Packet
{
public:
	Packet(int16_t nb_bits_i16);
	Packet();
	Packet(const Packet &copy);
	~Packet();
	void print_bytes();
	void print_bits();
	void print_bits2();

	void add_byte(uint8_t byte_lsb_first_u8);
	void add_bit(uint8_t bit);
	void add_bit2(uint8_t bit);

	uint16_t size() const;
	uint8_t get_bit(int16_t bit_pos)const;
	uint8_t get_bit2(int16_t bit_pos)const;
	uint32_t bits(uint16_t pos, uint8_t count)const;
	bool eof();
	void pk_reset();
	uint8_t nextBit();
	inline uint16_t bit_size() {return bit_size_i16;}
	void pad();
	void pad2();
	uint16_t crc16(int16_t start_i16);

	void copy_in(uint8_t *data_pu8,int16_t nb_bytes_i16);
	void add_bytes_lsbfirst(uint32_t value_u32,int16_t numBits_i16);
	void add_bytes_msbfirst(uint32_t value_u32,int16_t numBits_i16);
	void add_bits(uint32_t value_u32,int16_t numBits_i16);
	void flip_bits();
	inline int16_t nb_bytes(){return (bit_size_i16+1)>>3;};
	inline int16_t index(){return index_i16;}
	inline void index(int16_t val_i16 ){index_i16=val_i16;}

	void add_str(const char *str_p,uint16_t nb_bit);

	uint32_t reverse_bytes(uint32_t word_u32) const;
	void convert2bits();
	void convert2bytes();
	inline uint8_t * get_pt() {return mPacket;}
	inline void reset_mPacket() {memset((void*)mPacket,0,nb_bytes());}
	inline void reset_bit_payload() {memset((void*)bit_payload_pu8,0,bit_size_i16);}

	uint8_t field2u8(uint8_t start_bit, uint8_t nb_bit) const;
	uint8_t field2u8_lsbfirst(uint8_t start_bit, uint8_t nb_bit) const;
    uint8_t field2u8_msbfirst(uint8_t start_bit, uint8_t nb_bit) const;
    uint32_t field2u32(uint16_t start_bit,uint8_t nb_bit) const;
    uint32_t field2u32b(uint16_t start_bit,uint8_t nb_bit) const;
    uint32_t field2u32_lsbfirst(uint16_t start_bit,uint8_t nb_bit) const;
    uint32_t field2u32_msbfirst(uint16_t start_bit,uint8_t nb_bit) const;
    void field2str_lsbfirst(char * str_p,uint8_t mum_chars_u8,uint16_t start_bit, uint16_t nb_bit)const;
    void field2str_msbfirst(char * str_p,uint8_t mum_chars_u8,uint16_t start_bit, uint16_t nb_bit)const;

	uint8_t * mPacket;
	uint8_t * bit_payload_pu8;
	int16_t bit_size_i16;
	/*
	 * Writing index in [0,bit_size_i16]
	 */
	int16_t index_i16;
};




#endif // UTILITY_H
