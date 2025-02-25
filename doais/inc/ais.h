/*
 * =====================================
 * ais.h
 * =====================================
 * History:
 *   01/09/24 - Creation
 * =====================================
 */
#ifndef AIS_H
#define AIS_H

#include <nuttx/mutex.h>
#include "types.h"
#include "ais_channels.h"
#include "si4463.h"
#include "si4362.h"
#include "version.h"


#define SI4463_PART_INFO 0x4463
#define SI4362_PART_INFO 0x4362

#define si446x_1 SI4362::si446x1_
#define si446x_2 SI4362::si446x2_





/*
 * =========================
 * Externals
 * -------------------------
 */
extern gps_data_t Gps_info_s;
extern StationData Station_data_s;
extern FAR mutex_t Gps_data_mutex_s;

#endif //AIS_H
