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
