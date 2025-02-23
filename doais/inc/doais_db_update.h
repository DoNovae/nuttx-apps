#include <errno.h>
#include <math.h>
#include <poll.h>
#include <string.h>
#include <stdarg.h>
#include <sys/param.h>
#include "uORB/uORB.h"
#include "ais_channels.h"
#include "ais_monitoring.h"


#ifndef DOAIS_DB_UPDATE_H
#define DOAIS_DB_UPDATE_H

/*
 * Defines
 */
#define TIMER_INTERVAL_US 5000000
#define ORB_AIS_PACKET ((MAX_AIS_RX_PACKET_SIZE+1)>>3)
#define ORB_AIS_DB_UPDATE_QUEUE_SIZE 10

/*
 * Typedef
 */
struct orb_ais_db_update_s
{
	uint8_t packet_au8[ORB_AIS_PACKET];
	uint64_t timestamp;
	uint8_t id_u8;
};

//ORB_DECLARE(orb_ais_db_update_s);

/*
 * Prptotypes
 */
/*
 * Threads
 */
FAR void *db_update_thread(pthread_addr_t arg);
FAR void *timer_thread(pthread_addr_t arg);
bool rx_ais_decode(RXPacket &rx_packet_s, uint8_t ch_u8);


#endif //DOAIS_DB_UPDATE_H
