#include <errno.h>
#include <math.h>
#include <poll.h>
#include <string.h>
#include <stdarg.h>
#include <sys/param.h>
#include "uORB/uORB.h"
#include "ais_channels.h"
#include "ais_monitoring.h"


#ifndef DOAIS_GPS_H
#define DOAIS_GPS_H

/*
 * Defines
 */
#define GPS_SERIAL_DEVICE "/dev/ttyS1"


/*
 * Typedef
 */


/*
 * Prptotypes
 */
int setBaudrate(int _serial_fd, unsigned baud);

/*
 * Threads
 */
FAR void *gps_thread(pthread_addr_t arg);


/*
 * --------------------------
 * Externs
 * --------------------------
 */
extern gps_data_t Gps_info_s; // Cf doais_db_update


#endif //DOAIS_GPS_H
