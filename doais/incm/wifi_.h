/**
 * =====================================
 * wifi_h.h
 * -------------------------------------
 *  AIS DoNovae
 *  www.DoNovae.com
 *  Herve Bailly
 * =====================================
 */

#include "types.h"
#include "gps.h"
#include "gmath.h"
#include "sensor_gps.h"
#include "nmea.h"
#include "ubx.h"

/*
 * BUG WITH ONE OF THE FOLLOWING .h
#include "ashtech.h"
#include "emlid_reach.h"
#include "mtk.h"
#include "ubx.h"
*/


#ifndef __WIFI_H__
#define __WIFI_H__

/*
 * ===================
 * Defines
 * ===================
 */
#define WIFI_AP_SSID "DoAIS_V01"
#define WIFI_AP_PWD "doais1234"
#define WIFI_STA_SSID "HB44100"
#define WIFI_STA_PWD "b74c219cf856"
#define WIFI_UDP_CLIENT_PORT 10110
#define WIFI_UDP_LOCAL_PORT 10009
#define WIFI_UDP_STR_SZ 128
#define WIFI_CLIENT_IP "192.168.4.2"
#define WIFI_CLIENT_IP_SZ 12


/*
 * Serial2
 *   +RX:IO 16
 *   +TX: IO 17
 *
 */

#define SERIAL_GPS Serial2

// GPS DEFAULT
#define GPS_DEFAULT_BAUDRATE 9600

// VHF
//#define GPS_BAUDRATE 4800

// Neo GPS
#define GPS_BAUDRATE 115200

#define GPS_NEO_MODEL 6
//#define GPS_NEO_MODEL 7
#define GPS_SERIAL_TIMEOUT_MS 50

/*
 * ===================
 * Prototypes
 * ===================
 */
void ipaddr2str(IPAddress *ipaddr_pa,char *ipaddr_pc, uint8_t len_u8);
void str2ipaddr(char *ipaddr_pc, uint8_t len_u8,IPAddress *ipaddr_pa);
void print_ipaddr(char const *cmt_pc,IPAddress *ipaddr);

typedef enum {
	AIS_WIFI_START=0,
	AIS_WIFI_ON,
	AIS_WIFI_HS,
	AIS_WIFI_OFF
}ais_wifi_state_machine_e;

class ais_wifi
{
private:
	static WiFiUDP Udp;

public:
	//static IPAddress client_ip;
	//static IPAddress local_ip;
	//static IPAddress subnet_ip;

	static int udp_client_port;
	static int udp_local_port;
	static char ap_ssid[21];
	static char ap_pwd[21];
	static char sta_ssid[21];
	static char sta_pwd[21];
	static uint8_t on_u8;
	static uint8_t mode_ap_u8;
	static ais_wifi_state_machine_e state;

	static void on_off();
	static void on_ap_sta();
	static void start();
	static void stop();
	static void wifi_udp_write(char const * buffer, uint8_t len_u8);
	static bool wifi_udp_read(char * buffer, uint8_t len_u8);
	static void wifi_printf(const char *fmt, ...);
	static void setup_ap(char* ssid_pc,char * pwd_pc);
};




/*
 * -----------------------
 * Serial GPS
 * -----------------------
 */
//#define GPS_LOGGER_BUFSIZE 10
#define GPS_LOGGER_BUFSIZE 32
#define GPS_MAX_CMD_SIZE GPS_READ_BUFFER_SIZE
#define AIS_NMEA_BUF_SZ GPS_MAX_CMD_SIZE



class ais_serial
{
protected:
	 char serial_line_buffer[GPS_MAX_CMD_SIZE];
	 static char command_queue[GPS_LOGGER_BUFSIZE][GPS_MAX_CMD_SIZE];
	 static uint8_t commands_in_queue; // Count of commands in the queue
	 static uint8_t cmd_queue_index_r; // Ring buffer read position
	 static uint8_t cmd_queue_index_w; // Ring buffer write position
	// Number of characters read in the current line of serial input
	 int serial_count;
public:
	 ais_serial();
	 void loop();
	 void start();
	 void get_available_commands();
	 virtual void process_next_command();
	 void _commit_command();
	 void clear_command_queue();
};


#endif //__WIFI_H__


/*
 * -----------------------
 * NMEA GPS
 * -----------------------
 */

class Ais_nmea_gps : public ais_serial
{
private:
	//nmeaPARSER parser;
	char buff[AIS_NMEA_BUF_SZ];
	static int callback(GPSCallbackType type, void *buf, int data_int, void *user);

public:
	GPSDriverNMEA parser;
	GPSDriverUBX neo;
	sensor_gps_s sensor_gps;
	satellite_info_s satellite_info;
	uint8_t gps_nmea_on_u8;
	Ais_nmea_gps();
	~Ais_nmea_gps();
	void set_baudrate();
	void process_next_command() override;
	void parse();
	inline void gps_nmea_on_off() {gps_nmea_on_u8=(gps_nmea_on_u8)?0:1;};
	void update_gps_info(gps_data_t * gps_info_ps);
	void copy2buf(char *data1, uint16_t len_u16);
	void reset();
};

/*
 * Externs
 */
extern Ais_nmea_gps Ais_nmea_gps_s;
extern char Wifi_udp_write_char_a[WIFI_UDP_STR_SZ];
extern char Wifi_udp_read_char_a[WIFI_UDP_STR_SZ];


