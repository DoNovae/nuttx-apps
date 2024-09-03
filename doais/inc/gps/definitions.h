/****************************************************************************
 *
 *   Copyright (c) 2016 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file definitions.h
 * common platform-specific definitions & abstractions for gps
 * @author Beat Küng <beat-kueng@gmx.net>
 */

#include <unistd.h>
#include <time.h>
#include "satellite_info.h"
#include "sensor_gps.h"
//#include "esp_timer.h"

#ifndef __DEFINITIONS_H__
#define __DEFINITIONS_H__

/*
#define GPS_INFO(...) LOG_I(__VA_ARGS__)
#define GPS_WARN(...) LOG_W(__VA_ARGS__)
#define GPS_DEBUG(...) LOG_D(__VA_ARGS__)
#define GPS_ERR(...) LOG_E(__VA_ARGS__)
*/

#define GPS_INFO(...)
#define GPS_WARN(...)
#define GPS_DEBUG(...)
#define GPS_ERR(...)

#define M_DEG_TO_RAD 		(M_PI / 180.0)
#define M_RAD_TO_DEG 		(180.0 / M_PI)
#define M_DEG_TO_RAD_F 		0.01745329251994f
#define M_RAD_TO_DEG_F 		57.2957795130823f


//HBL #define gps_usleep px4_usleep
#define gps_usleep usleep

/**
 * Get the current time in us. Function signature:
 * uint64_t hrt_absolute_time()
 */
typedef uint64_t gps_abstime;

typedef struct{
	uint64_t timestamp;                  // time since system start (microseconds)
	uint64_t timestamp_sample;            //time since system start (microseconds)

	uint32_t device_id;                  // unique device ID for the sensor that does not change between power cycles

	uint64_t time_utc_usec;              // Timestamp (microseconds, UTC), this is the timestamp which comes from the gps module. It might be unavailable right after cold start, indicated by a value of 0

	uint16_t reference_station_id;       // Reference Station ID

	float position[3];               // GPS NED relative position vector (m)
	float position_accuracy[3]; // Accuracy of relative position (m)

	float heading;                   // Heading of the relative position vector (radians)
	float heading_accuracy;          // Accuracy of heading of the relative position vector (radians)

	float position_length;
	float accuracy_length;

	bool gnss_fix_ok;                  // GNSS valid fix (i.e within DOP & accuracy masks)
	bool differential_solution;        // differential corrections were applied
	bool relative_position_valid;
	bool carrier_solution_floating;    // carrier phase range solution with floating ambiguities
	bool carrier_solution_fixed;       // carrier phase range solution with fixed ambiguities
	bool moving_base_mode;             // if the receiver is operating in moving base mode
	bool reference_position_miss;      // extrapolated reference position was used to compute moving base solution this epoch
	bool reference_observations_miss;  // extrapolated reference observations were used to compute moving base solution this epoch
	bool heading_valid;
	bool relative_position_normalized; // the components of the relative position vector (including the high-precision parts) are normalized
}sensor_gnss_relative_s;

// HBL #define gps_absolute_time hrt_absolute_time
static inline gps_abstime gps_absolute_time() {
	//gps_abstime time_u64=millis()*1000;
	gps_abstime time_u64= esp_timer_get_time();
	//GPS_DEBUG("%d us",time_u64);
	return time_u64;
}


// TODO: this functionality is not available on the Snapdragon yet
// HBL #ifdef __PX4_QURT
//#define NO_MKTIME
// HBL #endif

#define NO_MKTIME

#endif //__DEFINITIONS_H__




