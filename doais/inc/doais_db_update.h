/**
 * =====================================
 *  doais_db_update.h
 * -------------------------------------
 *  AIS DoNovae
 *  www.DoNovae.com
 *  Herve Bailly
 * =====================================
 *
 * =====================================
 */

#include <errno.h>
#include <math.h>
#include <poll.h>
#include <string.h>
#include <stdarg.h>
#include <sys/param.h>
#include "doais_mng.h"
#include "ais_channels.h"
#include "ais_monitoring.h"


#ifndef DOAIS_DB_UPDATE_H
#define DOAIS_DB_UPDATE_H

/*
 * Defines
 */

/*
 * Typedef
 */


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
