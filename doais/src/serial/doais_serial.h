/**
 * =====================================
 *  doais_serial.h
 * -------------------------------------
 *  AIS DoNovae
 *  www.DoNovae.com
 *  Herve Bailly
 * =====================================
 * "M" Codes
 * M000 - Print help
 * M001 - About
 * M100 - Settings
 * M300 - Debug
 * M600 - Store parameters in EEPROM. (Requires EEPROM_SETTINGS)
 * M601 - Restore parameters from EEPROM. (Requires EEPROM_SETTINGS)
 * M602 - Revert to the default "factory settings". ** Does not write them to EEPROM! **
 * M603 - Print the current settings (in memory): "M503 S<verbose>". S0 specifies compact output.
 * M999 - Restart after being stopped by error
 * =====================================
 */
#include <nuttx/mqueue.h>

int serial_task(int argc, FAR char *argv[]);




