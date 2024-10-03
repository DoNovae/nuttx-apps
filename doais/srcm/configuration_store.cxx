/**
 * =====================================
 * configuration_store.cpp
 * -------------------------------------
 *  AIS DoNovae
 *  www.DoNovae.com
 *  Herve Bailly
 * =====================================
 */
#include <stdio.h>
#include "configuration_store.h"
#include "ais.h"
#include "ais_monitoring.h"
#include "ais_channels.h"

/*
 * --------------------------
 * Defines
 * --------------------------
 */
#define EEPROM_VERSION "V01"
#define SETTINGS_FILE_NAME "Doais_settings.dat"
#define WIFI_CLIENT_IP_SZ 12
#define GPS_DEFAULT_BAUDRATE 9600


#define EEPROM_START() rewind(File_p)
#define EEPROM_SKIP(VAR) fseek(File_p,sizeof(VAR),SEEK_CUR)
#define EEPROM_WRITE(VAR) write_data((uint8_t*)&(VAR), sizeof(VAR))
#define EEPROM_READ(VAR) read_data((uint8_t*)&(VAR), sizeof(VAR))

/*
 * --------------------------
 * Static
 * --------------------------
 */


/*
 * --------------------------
 * Externs
 * --------------------------
 */
extern StationData Station_data_s;
uint16_t Length_u16=2;
//extern Ais_monitoring Monitoring(AIS_CHAINED_LIST_MAX_SZ,AIS_CHAINED_LABEL_MAX_SZ);

/*
void eeprom_read_block(uint8_t * data, uint32_t address, size_t len)
{
	size_t i;
	for(i=0; i<len; i++){
		data[i] = EEPROM.read(address+i);
	}
}

void eeprom_write_block(uint8_t * data, uint32_t address, size_t len)
{
	size_t i;
	for(i=0; i<len; i++){
		EEPROM.write(address+i, data[i]);
	}
	EEPROM.commit();
}
 */


/*
 * --------------------------
 * Ais_settings
 * --------------------------
 */
Ais_settings::Ais_settings()
{
	/*
	File_p = fopen(SETTINGS_FILE_NAME,"w+b");
	if (!File_p) {
		LOG_E("Ais_settings: error file %s fopen\n",SETTINGS_FILE_NAME);
	} else {
		LOG_D("Ais_settings: file %s opened\n",SETTINGS_FILE_NAME);
	}
	 */
}

Ais_settings::~Ais_settings()
{
	/*
	if (File_p) {
		fclose(File_p);
	}
	 */
}




/**
 * Post-process after Retrieve or Reset
 */
void Ais_settings::postprocess()
{
	/*
	 * Station_data_s
	 */
	Station_data_s.to_bow=(Station_data_s.len+1)>>1;
	Station_data_s.to_stern=(Station_data_s.len+1)>>1;
	Station_data_s.to_port=(Station_data_s.beam+1)>>1;
	Station_data_s.to_starboard=(Station_data_s.beam+1)>>1;

	/*
	 * Monitoring
	 */
	//Monitoring.settings_s.lost_target_ticks_u32=Monitoring.settings_s.lost_target_mn_u32*MONOTORING_TICKS_1MN;
}

const char version[4]=EEPROM_VERSION;

uint16_t Ais_settings::eeprom_checksum;

bool Ais_settings::eeprom_write_error, Ais_settings::eeprom_read_error;

void Ais_settings::write_data(const uint8_t* val_pu8, uint16_t size_u16)
{
	if (eeprom_write_error) return;
	while (size_u16--) {
		uint8_t v_u8;
		uint8_t tmp_u8;

		v_u8 = *val_pu8;
		// EEPROM has only ~100,000 write cycles,
		// so only write bytes that have changed!
		//fread((void*)&tmp_u8,1,1,File_p);
		LOG_D("Ais_settings::write_data: v_u8(%c)/tmp_u8(%c)",v_u8,tmp_u8);
		//if (v_u8 != EEPROM.read(p)) {
		if (v_u8 != tmp_u8) {
			//EEPROM.write(p, v);
			//fseek(File_p,-1,SEEK_CUR);
			//fwrite((void*)&v_u8,1,1,File_p);
			//if (EEPROM.read(p) != v) {
			//fseek(File_p,-1,SEEK_CUR);
			//fread((void*)&tmp_u8,1,1,File_p);
			LOG_D("Ais_settings::write_data: v_u8(%c)/tmp_u8(%c)",v_u8,tmp_u8);
			if (tmp_u8 != v_u8) {
				LOG_E("Ais_settings: error writing to EEPROM!");
				eeprom_write_error = true;
				return;
			}
		}
		eeprom_checksum += v_u8;
		val_pu8++;
	};
}


void Ais_settings::read_data(uint8_t* val_pu8, uint16_t size_u16)
{
	uint8_t c_u8;
	do {
		//uint8_t c = EEPROM.read((int const)pos);
		//fread((void*)&c_u8,1,1,File_p);
		if (!eeprom_read_error) *val_pu8 = c_u8;
		eeprom_checksum += c_u8;
		val_pu8++;
	} while (--size_u16);
	LOG_W("read_data: %s",(char*)val_pu8);
}

/*
 * M600 - Store Configuration
 */
bool Ais_settings::save()
{
	//char wifi_ip_ac[WIFI_CLIENT_IP_SZ]="0.0.0.0";
	char ver[4] = EEPROM_VERSION;
	//EEPROM_START();
	eeprom_write_error=false;
	EEPROM_WRITE(ver);     // invalidate data first
	//EEPROM_SKIP(eeprom_checksum); // Skip the checksum slot
	eeprom_checksum=0; // clear

	/*
	 * AIS
	 */
	/*
	EEPROM_WRITE(Station_data_s.mmsi);
	EEPROM_WRITE(Station_data_s.beam);
	EEPROM_WRITE(Station_data_s.len);
	EEPROM_WRITE(Station_data_s.flags);
	EEPROM_WRITE(Station_data_s.callsign);
	EEPROM_WRITE(Station_data_s.shipname);
	EEPROM_WRITE(Station_data_s.shiptype);
	*/

	/*
	 * Wifi
	 */
	/*
	EEPROM_WRITE(ais_wifi::on_u8);
	EEPROM_WRITE(ais_wifi::mode_ap_u8);
	EEPROM_WRITE(ais_wifi::ap_ssid);
	EEPROM_WRITE(ais_wifi::ap_pwd);
	EEPROM_WRITE(ais_wifi::sta_ssid);
	EEPROM_WRITE(ais_wifi::sta_pwd);
	EEPROM_WRITE(ais_wifi::udp_client_port);
	EEPROM_WRITE(ais_wifi::udp_local_port);
	//ais_wifi::client_ip.toString().toCharArray(wifi_ip_ac,WIFI_CLIENT_IP_SZ,0);
	EEPROM_WRITE(wifi_ip_ac);
	 */

	/*
	 * NMEA
	 */
	//EEPROM_WRITE(Ais_nmea_gps_s.gps_nmea_on_u8);

	/*
	 * Monitoring
	 */
	/*
	EEPROM_WRITE(Monitoring.settings_s.cpa_warn_10thnm_u32);
	EEPROM_WRITE(Monitoring.settings_s.lost_target_mn_u32);
	EEPROM_WRITE(Monitoring.settings_s.tcpa_max_mn_u32);
	EEPROM_WRITE(Monitoring.settings_s.display_target_step_nm_u32);
	EEPROM_WRITE(Monitoring.settings_s.speed_min_kt_u32);
	EEPROM_WRITE(Monitoring.settings_s.gps_bauds_u32);
	 */

	if (!eeprom_write_error) {
		const uint16_t final_checksum=eeprom_checksum;

		// Write the EEPROM header
		EEPROM_WRITE(version);
		EEPROM_WRITE(final_checksum);

		// Report storage size
		printf("Ais_settings: settings stored\n");
	}

	// Commit
	//fflush(File_p);
	return !eeprom_write_error;
}


/*
 * M601 - Retrieve Configuration
 */
bool Ais_settings::load()
{
	//char wifi_ip_ac[WIFI_CLIENT_IP_SZ];
	//EEPROM_START();
	eeprom_read_error=false;

	char stored_ver[4];
	EEPROM_READ(stored_ver);

	uint16_t stored_checksum;
	EEPROM_READ(stored_checksum);

	// Version has to match or defaults are used
	//SERIAL_ECHOPAIR("EEPROM=", stored_ver);
	/*
	if (strncmp(version,stored_ver,3) != 0) {
		if (stored_ver[0] != 'V') {
			stored_ver[0] = '?';
			stored_ver[1] = '\0';
		}
		LOG_W("Ais_settings: version mismatch: %s/%s",stored_ver,EEPROM_VERSION);
		reset();
	}
	else {
	*/
		eeprom_checksum = 0; // clear before reading first "real data"

		/*
		 * AIS
		 */
		/*
		EEPROM_READ(Station_data_s.mmsi);
		EEPROM_READ(Station_data_s.beam);
		EEPROM_READ(Station_data_s.len);
		EEPROM_READ(Station_data_s.flags);
		EEPROM_READ(Station_data_s.callsign);
		EEPROM_READ(Station_data_s.shipname);
		EEPROM_READ(Station_data_s.shiptype);
		*/
		Station_data_s.len=Length_u16;

		/*
		 * Wifi
		 */
		/*
		EEPROM_READ(ais_wifi::on_u8);
		EEPROM_READ(ais_wifi::mode_ap_u8);
		EEPROM_READ(ais_wifi::ap_ssid);
		EEPROM_READ(ais_wifi::ap_pwd);
		EEPROM_READ(ais_wifi::sta_ssid);
		EEPROM_READ(ais_wifi::sta_pwd);
		EEPROM_READ(ais_wifi::udp_client_port);
		EEPROM_READ(ais_wifi::udp_local_port);
		EEPROM_READ(wifi_ip_ac);
		//ais_wifi::client_ip.fromString(wifi_ip_ac);
		 */

		/*
		 * NMEA
		 */
		//EEPROM_READ(Ais_nmea_gps_s.gps_nmea_on_u8);

		/*
		 * Monitoring
		 */
		/*
		EEPROM_READ(Monitoring.settings_s.cpa_warn_10thnm_u32);
		EEPROM_READ(Monitoring.settings_s.lost_target_mn_u32);
		EEPROM_READ(Monitoring.settings_s.tcpa_max_mn_u32);
		EEPROM_READ(Monitoring.settings_s.display_target_step_nm_u32);
		EEPROM_READ(Monitoring.settings_s.speed_min_kt_u32);
		EEPROM_READ(Monitoring.settings_s.gps_bauds_u32);
		 */
/*
		if (eeprom_checksum == stored_checksum) {
			if (eeprom_read_error)
				reset();
			else {
				postprocess();
				LOG_I("Ais_settings: %s stored settings retrieved",version);
			}
		}
		else {
			LOG_W("Ais_settings: checksum mismatch: stored %d/%d",stored_checksum,eeprom_checksum);
			reset();
		}

	}
			*/

	return !eeprom_read_error;
}


/*
 * M602 - Reset Configuration
 */
void Ais_settings::reset()
{
	LOG_I("Ais_settings: default settings loaded");

	/*
	 * AIS
	 */
	Station_data_s.reset();
	Length_u16=7;
	Station_data_s.len=Length_u16;

	/*
	 * Wifi
	 */
	/*
	ais_wifi::on_u8=0;
	ais_wifi::mode_ap_u8=1;
	strncpy(ais_wifi::ap_ssid,WIFI_AP_SSID,sizeof(ais_wifi::ap_ssid));
	strncpy(ais_wifi::ap_pwd,WIFI_AP_PWD,sizeof(ais_wifi::ap_pwd));
	strncpy(ais_wifi::sta_ssid,WIFI_STA_SSID,sizeof(ais_wifi::sta_ssid));
	strncpy(ais_wifi::sta_pwd,WIFI_STA_PWD,sizeof(ais_wifi::sta_pwd));
	ais_wifi::udp_client_port=WIFI_UDP_CLIENT_PORT;
	ais_wifi::udp_local_port=WIFI_UDP_LOCAL_PORT;
	ais_wifi::client_ip.fromString(WIFI_CLIENT_IP);
	 */

	/*
	 * NMEA
	 */
	//Ais_nmea_gps_s.gps_nmea_on_u8=0;


	/*
	 * Monitoring
	 */
	/*
	Monitoring.settings_s.cpa_warn_10thnm_u32=MONOTORING_CPA_WARN_10THNM;
	Monitoring.settings_s.lost_target_mn_u32=MONOTORING_LOST_TARGET_MN;
	Monitoring.settings_s.tcpa_max_mn_u32=MONOTORING_MAX_TCPA_MN;
	Monitoring.settings_s.display_target_step_nm_u32=MONOTORING_DISPLAY_TARGET_STEP_NM;
	Monitoring.settings_s.speed_min_kt_u32=MONITORING_DISPLAY_SPEED_MIN_KT;
	Monitoring.settings_s.gps_bauds_u32=GPS_DEFAULT_BAUDRATE;
	 */

	/*
	 * Post process
	 */
	postprocess();
	report(false);
}

void Ais_settings::report(bool onwifi)
{
	//char wifi_ip_ac[WIFI_CLIENT_IP_SZ];
	/*
	 * AIS
	 */
	/*
	ais_wifi::wifi_printf("%s\n","AIS");
	ais_wifi::wifi_printf("  mmsi: %d\n",Station_data_s.mmsi);
	ais_wifi::wifi_printf("  beam: %d\n",Station_data_s.beam);
	ais_wifi::wifi_printf("  len: %d\n",Station_data_s.len);
	ais_wifi::wifi_printf("  flags: %d\n",Station_data_s.flags);
	ais_wifi::wifi_printf("  callsign: %s\n",Station_data_s.callsign);
	ais_wifi::wifi_printf("  shipname: %s\n",Station_data_s.shipname);
	ais_wifi::wifi_printf("  vendorid: %s\n",Station_data_s.vendorid);
	ais_wifi::wifi_printf("  shiptype: %d\n",Station_data_s.shiptype);
	 */
	printf("%s\n","AIS");
	printf("  mmsi: %d\n",Station_data_s.mmsi);
	printf("  beam: %d\n",Station_data_s.beam);
	printf("  len: %d\n",Station_data_s.len);
	printf("  flags: %d\n",Station_data_s.flags);
	printf("  callsign: %s\n",Station_data_s.callsign);
	printf("  shipname: %s\n",Station_data_s.shipname);
	printf("  vendorid: %s\n",Station_data_s.vendorid);
	printf("  shiptype: %d\n",Station_data_s.shiptype);


	/*
	 * Wifi
	 */
	/*
	ais_wifi::wifi_printf("%s\n","WIFI");
	ais_wifi::wifi_printf("  on: %d\n",ais_wifi::on_u8);
	ais_wifi::wifi_printf("  mode_ap: %d\n",ais_wifi::mode_ap_u8);
	ais_wifi::wifi_printf("  ap_ssid: %s\n",ais_wifi::ap_ssid);
	ais_wifi::wifi_printf("  ap_pwd: %s\n",ais_wifi::ap_pwd);
	ais_wifi::wifi_printf("  sta_ssid: %s\n",ais_wifi::sta_ssid);
	ais_wifi::wifi_printf("  sta_pwd: %s\n",ais_wifi::sta_pwd);
	ais_wifi::wifi_printf("  udp_client_port: %d\n",ais_wifi::udp_client_port);
	ais_wifi::wifi_printf("  udp_local_port: %d\n",ais_wifi::udp_local_port);
	ais_wifi::client_ip.toString().toCharArray(wifi_ip_ac,WIFI_CLIENT_IP_SZ,0);
	ais_wifi::wifi_printf("  client_ip: %s\n",wifi_ip_ac);
	 */

	/*
	 * NMEA
	 */
	/*
	ais_wifi::wifi_printf("%s\n","NMEA");
	ais_wifi::wifi_printf("  gps_nmea_on_u8: %d\n",Ais_nmea_gps_s.gps_nmea_on_u8);
	 */
	//printf("%s\n","NMEA");
	//printf("  gps_nmea_on_u8: %d\n",Ais_nmea_gps_s.gps_nmea_on_u8);

	/*
	 * Monitoring
	 */
	/*
	ais_wifi::wifi_printf("%s\n","Monitoring");
	ais_wifi::wifi_printf("  cpa_warn_10thnm: %d\n",Monitoring.settings_s.cpa_warn_10thnm_u32);
	ais_wifi::wifi_printf("  lost_target_mn: %d\n",Monitoring.settings_s.lost_target_mn_u32);
	ais_wifi::wifi_printf("  tcpa_max_mn: %d\n",Monitoring.settings_s.tcpa_max_mn_u32);
	ais_wifi::wifi_printf("  display_target_step_nm_u32: %d\n",Monitoring.settings_s.display_target_step_nm_u32);
	ais_wifi::wifi_printf("  speed_min_kt_u32: %d\n",Monitoring.settings_s.speed_min_kt_u32);
	ais_wifi::wifi_printf("  gps_bauds_u32: %d\n",Monitoring.settings_s.gps_bauds_u32);
	 */
	/*
	printf("%s\n","Monitoring");
	printf("  cpa_warn_10thnm: %d\n",Monitoring.settings_s.cpa_warn_10thnm_u32);
	printf("  lost_target_mn: %d\n",Monitoring.settings_s.lost_target_mn_u32);
	printf("  tcpa_max_mn: %d\n",Monitoring.settings_s.tcpa_max_mn_u32);
	printf("  display_target_step_nm_u32: %d\n",Monitoring.settings_s.display_target_step_nm_u32);
	printf("  speed_min_kt_u32: %d\n",Monitoring.settings_s.speed_min_kt_u32);
	printf("  gps_bauds_u32: %d\n",Monitoring.settings_s.gps_bauds_u32);
	 */
}



