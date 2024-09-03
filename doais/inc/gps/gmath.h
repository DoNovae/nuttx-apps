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

#include "gps.h"


#ifndef __NMEA_GMATH_H__
#define __NMEA_GMATH_H__


#define NMEA_PI                     (3.141592653589793)             /**< PI value */
#define NMEA_PI180                  (NMEA_PI / 180)                 /**< PI division by 180 */
#define NMEA_EARTHRADIUS_KM         (6378)                          /**< Earth's mean radius in km */
#define NMEA_EARTHRADIUS_M          (NMEA_EARTHRADIUS_KM * 1000)    /**< Earth's mean radius in m */
#define NMEA_EARTH_SEMIMAJORAXIS_M  (6378137.0)                     /**< Earth's semi-major axis in m according WGS84 */
#define NMEA_EARTH_SEMIMAJORAXIS_KM (NMEA_EARTHMAJORAXIS_KM / 1000) /**< Earth's semi-major axis in km according WGS 84 */
#define NMEA_EARTH_FLATTENING       (1 / 298.257223563)             /**< Earth's flattening according WGS 84 */
#define NMEA_DOP_FACTOR             (5)                             /**< Factor for translating DOP to meters */


/**
 * Position data in fractional degrees or radians
 */
typedef struct _nmeaPOS {
	float lat_d; /**< Latitude degree*/
	float lon_d; /**< Longitude degree*/
	float lat_r; /**< Latitude radian*/
	float lon_r; /**< Longitude radian*/
	float lon_nm;
	float lat_nm;
} nmeaPOS;

/*
 * degree VS radian
 */

inline double nmea_degree2radian(const double val){ return (val * NMEA_PI180);};
inline double nmea_radian2degree(const double val){ return (val / NMEA_PI180);};

/*
 * DOP
 */

double nmea_calc_pdop(double hdop, double vdop);
double nmea_dop2meters(double dop);
double nmea_meters2dop(double meters);

/*
 * positions work
 */

double  nmea_distance(
        const nmeaPOS *from_pos,
        const nmeaPOS *to_pos
        );

double  nmea_distance_ellipsoid(
        const nmeaPOS *from_pos,
        const nmeaPOS *to_pos,
        double *from_azimuth,
        double *to_azimuth
        );

int     nmea_move_horz(
        const nmeaPOS *start_pos,
        nmeaPOS *end_pos,
        double azimuth,
        double distance
        );

int     nmea_move_horz_ellipsoid(
        const nmeaPOS *start_pos,
        nmeaPOS *end_pos,
        double azimuth,
        double distance,
        double *end_azimuth
        );


#endif /* __NMEA_GMATH_H__ */
