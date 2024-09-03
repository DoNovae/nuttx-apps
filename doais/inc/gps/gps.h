/*
 * This file is part of nmealib.
 *
 * Copyright (c) 2008 Timur Sinitsyn
 * Copyright (c) 2011 Ferry Huberts
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*! \file */

#ifndef __GPS_H__
#define __GPS_H__

#include <stdbool.h>
#include "tok.h"



#define LAT_LONG_SCALE ((float)10000000.0)

typedef tm timeinfo_t;

/**
 * Summary GPS information from all parsed packets,
 * used also for generating NMEA stream
 * @see nmea_parse
 * @see nmea_GPGGA2info,  nmea_...2info
 */

typedef struct gps_data_s
{
	timeinfo_t utc_s;
	float elv;         /* Antenna altitude above/below mean sea level (geoid) in meters in [-100,100] */
	float speed_kt;    /* Speed over the ground in knots */
	float heading_d;   /* Track angle in degrees True in [0,360] */
	int32_t fix;       /* Fix quality 1 means just a normal 3D fix. 3 for auto, 4 for DGPS, 5 for floats, 6 for fixed. cf GGA */
	int32_t lat_d;     /* Latitude in DEG * 10000000.0 */
	int32_t lon_d;     /* Longitude in DEG * 10000000.0 */
} gps_data_t;


void nmea_zero_INFO(gps_data_t *info);

void nmea_INFO_sanitise(gps_data_t *gps_data);

void nmea_INFO_unit_conversion(gps_data_t * gps_data);


#endif /* __GPS_H__ */
