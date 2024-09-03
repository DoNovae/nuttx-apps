/*
 * AISPacket.hpp
 *
 *  Created on: Dec 6, 2015
 *      Author: peter
 */

#ifndef RXPACKET_H
#define RXPACKET_H

#include <inttypes.h>
#include "utilities.h"
#include "ais_channels.h"


#define CRC_LSBIT_FISRT 0xf0b8


/*
 * ========================
 * RXPacket
 * ========================
 *  - Bytes packet
 * ========================
 */

class RXPacket : public Packet {
public:
    //friend class Si446x;
    RXPacket (int16_t nb_bits_i16);
    RXPacket ();
    ~RXPacket ();
    RXPacket(const RXPacket &copy);
    //RXPacket &operator=(const RXPacket &copy);
public:
    inline void setChannel(VHFChannel channel){mChannel=channel;}
    inline VHFChannel channel() const {return mChannel;}
    void discardCRC();
    void decodeCRC(uint8_t byte_u8);
    inline uint16_t crc() const {return mCRC;}
    bool checkCRC() const;
    bool isBad() const;
    void reset();

    // These are link-level attributes
    inline uint8_t ais_rssi() const{return mRSSI;}
    inline void ais_setRSSI(uint8_t rssi){mRSSI=rssi;}
    void ais_finalize();
    void ais_fillpk_bitUnStuff();
    bool ais_get_nrzi_bits();
    void addBitCRC(uint8_t bit);

    inline uint8_t ais_type()const {return field2u8_msbfirst(0,6);}
    inline uint32_t ais_mmsi() const {return field2u32_msbfirst(8,30);}
    inline uint8_t get_partno(const RXPacket &packet) const {return packet.field2u32_msbfirst(38,2);};


    uint16_t mCRC;
    VHFChannel mChannel;
    uint8_t mRSSI;
    uint32_t mmsi;
};

#endif
