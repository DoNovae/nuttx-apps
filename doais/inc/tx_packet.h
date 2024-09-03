/*
 * TXPacket.hpp
 *
 *  Created on: Dec 12, 2015
 *      Author: peter
 */

#ifndef TXPACKET_H
#define TXPACKET_H

#include <inttypes.h>
#include <time.h>

#include "utilities.h"
#include "ais_channels.h"


class TXPacket : public Packet {
public:
    TXPacket(int16_t nb_bits_i16);
    TXPacket();
    TXPacket(const TXPacket &copy);
    TXPacket(const Packet &copy);
    ~TXPacket();

    VHFChannel channel();
    void setChannel(VHFChannel channel);
    time_t txTime();
    void configure(VHFChannel channel, time_t txTime);
    void reset();

    void ais_bitStuff();
    void ais_nrziEncode();
    void ais_finalize();

    VHFChannel mChannel;
    time_t mTXTime;
};





#endif
