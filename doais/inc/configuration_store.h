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

#define EEPROM_VERSION "V01"


class Ais_settings {
public:
	Ais_settings();
	~Ais_settings();
	static void begin();
	static void reset();
	static void save();
	static int load();
	static void report(bool onwifi=false);

private:
	static const char Version[sizeof(EEPROM_VERSION)];
	static void postprocess();
	static void write_data(const uint8_t* val_pu8, uint16_t size_u16);
	static void read_data(uint8_t* val_pu8, uint16_t size_u16);
	/*
	 * Checksum of all the data except the version and Eeprom_checksum_u16
	 */
	static uint16_t Eeprom_checksum_u16;
	static FILE * File_p;
};

#endif // CONFIGURATION_STORE_H


