/**
 * =====================================
 *  configuration_store.h
 * -------------------------------------
 *  AIS DoNovae
 *  www.DoNovae.com
 *  Herve Bailly
 * =====================================
 */


#ifndef CONFIGURATION_STORE_H
#define CONFIGURATION_STORE_H

#include <inttypes.h>
#include <stdio.h>

class Ais_settings {
public:
	Ais_settings();
	~Ais_settings();
	static void begin();
	static void reset();
	static bool save();
	static bool load();
	static void report(bool onwifi=false);

private:
	static void postprocess();
	static bool eeprom_read_error, eeprom_write_error;
	static void write_data(const uint8_t* val_pu8, uint16_t size_u16);
	static void read_data(uint8_t* val_pu8, uint16_t size_u16);
	static uint16_t eeprom_checksum;
	static FILE * File_p;
};

#endif // CONFIGURATION_STORE_H
