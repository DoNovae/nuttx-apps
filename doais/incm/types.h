/**
 * =====================================
 * types.h
 * -------------------------------------
 *  AIS DoNovae
 *  www.DoNovae.com
 *  Herve Bailly
 * =====================================
 */

#include <stdint.h>
#include <stdio.h>
#include <debug.h>

#ifndef __TYPES1_H__
#define __TYPES1_H__

/*
 * =============
 * MACROS
 * =============
 */

/*
 * Debug by levels
 */
// LOG_ALERT - Indicates a condition that should be corrected immediately; for example, a corrupted database.
// LOG_CRIT - Indicates critical conditions; for example, hard device errors.
// LOG_ERR - Indicated error conditions.
// LOG_DEBUG - Displays messages containing information useful to debug a program.
// LOG_EMERG - Indicates a panic condition reported to all users; system is unusable.
// LOG_ERR - Indicated error conditions.
// LOG_INFO - Indicates general information messages.
// LOG_NOTICE - Indicates a condition requiring special handling, but not an error condition.
// LOG_WARNING - Logs warning messages.

#define LOG_E(format, ...) syslog(LOG_ERR,    format, ##__VA_ARGS__)
#define LOG_D(format, ...) syslog(LOG_DEBUG,  format, ##__VA_ARGS__)
#define LOG_I(format, ...) syslog(LOG_INFO,   format, ##__VA_ARGS__)
#define LOG_W(format, ...) syslog(LOG_WARNING,format, ##__VA_ARGS__)
#define LOG_V(format, ...) syslog(LOG_WARNING,format, ##__VA_ARGS__)

#define _min(a,b) ((a)<(b)?(a):(b))
#define _max(a,b) ((a)>(b)?(a):(b))
#define _abs(x) ((x)>0?(x):-(x))  // abs() comes from STL
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#define _round(x)     ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))  // round() comes from STL
#define radians(deg) ((deg)*DEG_TO_RAD)
#define degrees(rad) ((rad)*RAD_TO_DEG)
#define sq(x) ((x)*(x))
#define WITHIN(V,L,H) ((V) >= (L) && (V) <= (H))
#define NUMERIC(a) WITHIN(a, '0', '9')
#define NUMERIC_SIGNED(a) (NUMERIC(a) || (a) == '-')
#define COUNT(a) (sizeof(a)/sizeof(*a))
//#define ZERO(a) memset(a,0,sizeof(a))
#define COPY(a,b) memcpy(a,b,min(sizeof(a),sizeof(b)))

//#define USE_WATCHDOG
#define EEPROM_SIZE 4096
#define LOGGER_BAUDRATE 115200
#define EEPROM_SETTINGS

#define SERIAL_LOGGER Serial

#define AIS_RX_BUF_SZ 16

#define BLOCK_BUFFER_SIZE 16 // maximize block buffer

// The ASCII buffer for serial input
#define MAX_CMD_SIZE 96
#define LOGGER_BUFSIZE 4

#define DEV_MODE 1

/*
 * Globals
 */

#endif //__TYPES1_H__
