/**
 * =====================================
 *  doais_serial.cxx
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

#include <sys/ioctl.h>
#include <stdio.h>
#include <nuttx/config.h>
#include <stdio.h>
#include <fcntl.h>
#include <nuttx/mqueue.h>


#include "doais_serial.h"
#include "version.h"
#include "configuration_store.h"
#include "ais_channels.h"
#include "ais_monitoring.h"
#include "types.h"
#include "doais_mng.h"
#include "doais_db_update.h"
#include "doais_gps.h"

#include "ais_test.h"

/*
 * --------------------------
 * Defines
 * --------------------------
 */
#define USLEEP_50MS (50*1000)
#define TIMEOUT_UORB_US (1000*1000)

/*
 * --------------------------
 * Globals
 * --------------------------
 */
int Serial_fd;
bool Running = true;
uint8_t commands_in_queue = 0; // Count of commands in the queue
static uint8_t cmd_queue_index_r = 0; // Ring buffer read position
static uint8_t cmd_queue_index_w = 0; // Ring buffer write position
static char command_queue[LOGGER_BUFSIZE][MAX_CMD_SIZE];
static char *current_command,*current_command_args,*seen_pointer;
// Number of characters read in the current line of serial input
static int serial_count = 0;
static bool send_ok[LOGGER_BUFSIZE];
static const char *injected_commands_P = NULL;


ORB_DEFINE(orb_ais_db_update,struct orb_ais_db_update_s,0);

/*
 * --------------------------
 * Prototypes
 * --------------------------
 */
static void get_available_commands();
static void process_next_command();
static bool drain_injected_commands_P();
static void enqueue_and_echo_commands_P(const char* pgcode);
static bool enqueue_and_echo_command(const char* cmd, bool say_ok/*=false*/);
static void ok_to_send();
static void FlushSerialRequestResend();
void print_doais();

/*
 * --------------------------
 * Externs
 * --------------------------
 */
extern StationData Station_data_s;
extern Ais_monitoring Monitoring;
extern gps_data_t Gps_info_s;


/*
 * --------------------------
 * Functions
 * --------------------------
 */

bool drain_injected_commands_P()
{
	if (injected_commands_P != NULL) {
		uint8_t i = 0;
		char c, cmd[MAX_CMD_SIZE];
		memcpy(cmd, injected_commands_P, sizeof(cmd) - 1);
		cmd[sizeof(cmd) - 1]='\0';
		while ((c = cmd[i]) && c != '\n') i++; // find the end of this gcode command
		cmd[i]='\0';
		if (enqueue_and_echo_command(cmd,false))     // success?
			injected_commands_P = c ? injected_commands_P + i + 1 : NULL; // next command or done
	}
	return (injected_commands_P != NULL);    // return whether any more remain
}

void enqueue_and_echo_commands_P(const char* pgcode)
{
	injected_commands_P = pgcode;
	drain_injected_commands_P(); // first command executed asap (when possible)
}

void clear_command_queue()
{
	cmd_queue_index_r = cmd_queue_index_w;
	commands_in_queue = 0;
}

void get_available_commands()
{
	// if any immediate commands remain, don't get other commands yet
	if (drain_injected_commands_P()) return;
}

inline void _commit_command(bool say_ok)
{
	send_ok[cmd_queue_index_w] = say_ok;
	if (++cmd_queue_index_w >= LOGGER_BUFSIZE) cmd_queue_index_w = 0;
	commands_in_queue++;
}

inline bool _enqueuecommand(const char* cmd, bool say_ok=false)
{
	if (*cmd == ';' || commands_in_queue >= LOGGER_BUFSIZE) return false;
	strcpy(command_queue[cmd_queue_index_w], cmd);
	_commit_command(say_ok);
	return true;
}

bool enqueue_and_echo_command(const char* cmd, bool say_ok/*=false*/)
{
	if (_enqueuecommand(cmd, say_ok)) {
		LOG_I("->Enqueueing %s\n",cmd);
		return true;
	}
	return false;
}

void gcode_line_error(const char* err, bool doFlush = true) {
	printf("-> %s\n",err);
	if (doFlush) FlushSerialRequestResend();
	serial_count = 0;
}

void FlushSerialRequestResend()
{
	printf("Resend");
	ok_to_send();
}


inline bool code_has_value()
{
	int i = 1;
	char c = seen_pointer[i];
	while (c == ' ') c = seen_pointer[++i];
	if (c == '-' || c == '+') c = seen_pointer[++i];
	if (c == '.') c = seen_pointer[++i];
	return NUMERIC(c);
}

inline float code_value_float()
{
	char* e = strchr(seen_pointer, 'E');
	if (!e) return strtod(seen_pointer + 1, NULL);
	*e = 0;
	float ret = strtod(seen_pointer + 1, NULL);
	*e = 'E';
	return ret;
}

inline unsigned long code_value_ulong() {return strtoul(seen_pointer + 1, NULL, 10);}
inline long code_value_long() {return strtol(seen_pointer + 1, NULL, 10);}
inline int code_value_int() {return (int)strtol(seen_pointer + 1, NULL, 10);}
inline uint16_t code_value_ushort() {return (uint16_t)strtoul(seen_pointer + 1, NULL, 10);}
inline uint8_t code_value_byte() {return (uint8_t)(constrain(strtol(seen_pointer + 1, NULL, 10), 0, 255));}
inline bool code_value_bool() {return !code_has_value() || code_value_byte() > 0;}




bool code_seen(char code) {
	seen_pointer = strchr(current_command_args, code);
	return (seen_pointer != NULL); // Return TRUE if the code-letter was found
}


void unknown_command_error()
{
	printf("->Unknown command error \"%s\"/n",current_command);
}

inline void gcode_M000()
{
	char help_str[]="\
DoAis Help\n\
  M0 - Print help\n\
  M1 - About\n\
\n\
SETTINGS\n\
AIS\n\
  M101 M<mmsi> B<beam> L<length> F<flag> T<shiptype>\n\
  M102 <shipname>\n\
  M103 <callsign>\n\
  M104 C<cpa_warn_10thnm> L<lost_target_mn> T<tcpa_max_mn> S<display_target_step_nm> V<display_speed_min_kt>\n\
\n\
WIFI\n\
  M110 <Toggle wifi on_off>\n\
  M111 <wifi on_apsta>\n\
  M112 <wifi ap_ssid>\n\
  M113 <wifi ap_pwd>\n\
  M114 <wifi sta_ssid>\n\
  M115 <wifi sta_pwd>\n\
  M117 <Toggle NMEA gps_nmea_on_u8>\n\
\n\
FLASH\n\
  M600      - Store parameters in EEPROM\n\
  M601      - Restore parameters from EEPROM\n\
  M602      - Revert to the default factory settings\n\
  M603 W<2> - Print the current settings (in memory) - W for on wifi\n\
  M700 B<baud> - Serial GPS : set baudrate 4800(VHF), 9600 (def), \n\
  M999      - Restart\n\
\n\
";
#if DEV_MODE
	char debug_str[]="\
  DEBUG\n\
  M300 - Test endianess\n\
  M311 N<NB MSG> C<channel> - Send N AIS messages and set channel\n\
  M312 T<type 1 or 18 or 240 or 241> Test chain\n\
  M313 N<NB MSG> - Stats over N AIS messages, without or with IT\n\
  M315 D<direction> - Test Bench\n\
  M316 T<type 1 or 18 > M<mmsi> A<azimut_deg> R<range_nm> H<heading_deg> S<speed_kt10> C<myheading_deg> V<myspeed_kt10>\n\
  M320 Test GPS Nmea library\n\
\n";
#endif
	/*
	if (ais_wifi::state==AIS_WIFI_ON)
	{
		ais_wifi::wifi_udp_write(help_str,strlen(help_str));
#if DEV_MODE
		ais_wifi::wifi_udp_write(debug_str,strlen(debug_str));
#endif
	}
	 */
	printf("%s",help_str);
#if DEV_MODE
	printf("%s",debug_str);
#endif
}



/*
 * ------------------------
 * M001
 * ------------------------
 * About
 * ------------------------
 */
void gcode_M001()
{
	print_doais();
	printf("Last Updated: %s-%s\n",STRING_DISTRIBUTION_DATE,STRING_CONFIG_H_AUTHOR);
	printf("Compiled: %s\n",__DATE__);
	printf("%s %s\n",MACHINE_NAME,SHORT_BUILD_VERSION);
}


/*
 * ------------------------
 * M100
 * ------------------------
 * Settings
 * ------------------------
 */

/*
 * ------------------------
 * M101
 * ------------------------
 * AIS
 * M101 M<mmsi> B<beam> L<length> F<flag>
 * ------------------------
 */
inline void gcode_M101()
{
	Station_data_s.mmsi=code_seen('M')?code_value_int():Station_data_s.mmsi;
	Station_data_s.beam=code_seen('B')?code_value_ushort():Station_data_s.beam;
	Station_data_s.len=code_seen('L')?code_value_ushort():Station_data_s.len;
	Station_data_s.flags=code_seen('F')?code_value_ushort():Station_data_s.flags;
	uint8_t shiptype_u8=code_seen('T')?code_value_ushort():Station_data_s.shiptype;
	Station_data_s.to_bow=(Station_data_s.len+1)>>1;
	Station_data_s.to_stern=(Station_data_s.len+1)>>1;
	Station_data_s.to_port=(Station_data_s.beam+1)>>1;
	Station_data_s.to_starboard=(Station_data_s.beam+1)>>1;
	switch(shiptype_u8){
	case SAILING:
	case PLEASSURE_CRAFT:
	case HIGH_SPEED_CRAFT:
		Station_data_s.shiptype=shiptype_u8;
		break;
	case SHIPTYPE_NONE:
		break;
	default:
		printf("M101 Error shiptype must be SAILING, PLEASSURE_CRAFT\n");
		break;
	}
}

/*
 * ------------------------
 * M102
 * ------------------------
 * AIS
 * M102 <shipname>
 * ------------------------
 */
inline void gcode_M102()
{
	const char * const args = current_command_args;
	if (!Utils::test_ascii(current_command_args)){
		printf("M102 error: %.*s - AIS incompatible characters\n",MAX_CMD_SIZE,current_command_args);
		return;
	}
	uint8_t len_u8=_min(strlen(current_command_args),sizeof(Station_data_s.shipname));
	LOG_I("args: %.*s - len: %d",MAX_CMD_SIZE,args,len_u8);
	if (len_u8 && (len_u8<STATION_SHIP_NAME_SZ)) strncpy(Station_data_s.shipname,args,len_u8);
	Station_data_s.shipname[len_u8]=0;
}

/*
 * ------------------------
 * M103
 * ------------------------
 * AIS
 * M103 <callsign>
 * ------------------------
 */
inline void gcode_M103()
{
	const char * const args = current_command_args;
	if (!Utils::test_ascii(current_command_args)){
		printf("M103 error: %.*s - AIS incompatible characters\n",MAX_CMD_SIZE,current_command_args);
		return;
	}
	uint8_t len_u8=_min(strlen(current_command_args),sizeof(Station_data_s.callsign));
	LOG_I("args: %.*s - len: %d",MAX_CMD_SIZE,args,len_u8);
	if (len_u8 && (len_u8<STATION_CALLSIGN_SZ)) strncpy(Station_data_s.callsign,args,len_u8);
	Station_data_s.callsign[len_u8]=0;
}

/*
 * ------------------------
 * M104
 * ------------------------
 * AIS
 * M104 C<cpa_warn_10thnm> L<lost_target_nm> T<tcpa_max_mn> S<display_target_step_nm> V<display_speed_min_kt>
 * ------------------------
 */
inline void gcode_M104()
{
	Monitoring.settings_s.cpa_warn_10thnm_u32=code_seen('C')?code_value_int():Monitoring.settings_s.cpa_warn_10thnm_u32;
	Monitoring.settings_s.lost_target_mn_u32=code_seen('L')?code_value_int():Monitoring.settings_s.lost_target_mn_u32;
	Monitoring.settings_s.tcpa_max_mn_u32=code_seen('T')?code_value_int():Monitoring.settings_s.tcpa_max_mn_u32;
	Monitoring.settings_s.display_target_step_nm_u32=code_seen('S')?code_value_int():Monitoring.settings_s.display_target_step_nm_u32;
	Monitoring.settings_s.speed_min_kt_u32=code_seen('V')?code_value_int():Monitoring.settings_s.speed_min_kt_u32;
	if (!Monitoring.settings_s.display_target_step_nm_u32){
		printf("M104 error: display_target_step_nm_u32(%d) must be positive.",Monitoring.settings_s.display_target_step_nm_u32);
		Monitoring.settings_s.display_target_step_nm_u32=MONOTORING_DISPLAY_TARGET_STEP_NM;
	}
	Monitoring.settings_s.lost_target_ticks_u32=Monitoring.settings_s.lost_target_mn_u32*MONOTORING_TICKS_1MN;
	LOG_D("lost_target_mn_u32 : %d",Monitoring.settings_s.lost_target_mn_u32);
}

/*
 * ------------------------
 * M600
 * ------------------------
 * Flash
 * ------------------------
 */

/*
 * ------------------------
 * M600: Store settings in EEPROM
 * ------------------------
 */
inline void gcode_M600() {
	printf("Store settings in EEPROM\n");
	Ais_settings::save();
}

/*
 * ------------------------
 * M601: Read settings from EEPROM
 * ------------------------
 */
inline void gcode_M601() {
	printf("Read settings from EEPROM\n");
	Ais_settings::load();
}

/*
 * ------------------------
 * M602: Revert to default settings
 * ------------------------
 */
inline void gcode_M602() {
	printf("Revert to default settings\n");
	Ais_settings::reset();
}

/*
 * ------------------------
 * M603: print settings currently in memory
 * ------------------------
 */
inline void gcode_M603() {
	//bool onwifi=(code_seen('W')and (ais_wifi::state==AIS_WIFI_ON))?true:false;
	bool onwifi=true;
	Ais_settings::report(onwifi);
}

/*
 * ------------------------
 * M700 f<1> b<1> - Serial GPS : flush or begin\n\
 * ------------------------
 */
inline void gcode_M700()
{
	int fd;
	uint32_t bauds_u32=code_seen('B')?code_value_ulong():9600;
	printf("GPS serial bauds(%d)\n",bauds_u32);
	Monitoring.settings_s.gps_bauds_u32=bauds_u32;
}


/*
 * --------------------------
 * M312 T<type 1 or 18 or 240 or 241>
 * Test chain transmitter /receiver
 * M312 T18
 * --------------------------
 * Test
 *   Source: http://www.it-digin.com/blog/?p=20
 *   !AIVDM,1,1,,A,133m@ogP00PD;88MD5MTDww@2D7k,0*46
 *   	+type=1
 *   	+mmsi=205344990
 *   	+repeat=0
 *   	+status=15
 *   	+lon=4.4070466
 *   	+lat=51.229636
 *   	+accuracy=1
 *   	+heading=511
 *   Online decoder: https://www.aggsoft.com/ais-decoder.htm
 *  --------------------------
 */
#define M312_REFERENCE_MSG 100
#define M312_MSG181 118
#define M312_MSG182 119
#define LAT_D (47.21350504)
#define LON_D (-1.56928539)

void gcode_M312(){
	uint8_t msg_type_u8=code_seen('T')?code_value_ushort():0;
	TXPacket tx_packet_s(MAX_AIS_RX_PACKET_SIZE);
	printf("gcode_M312: msg_type_u8(%d) in {1,4,18,12,240,241,100,101,102,104,112,138}\n",msg_type_u8);

	/*
	 * Gps_info_s
	 */
	Gps_info_s.lat_d=(int32_t)((float)LAT_D*LAT_LONG_SCALE);
	Gps_info_s.lon_d=(int32_t)((float)LON_D*LAT_LONG_SCALE);
	Gps_info_s.speed_kt=((float)65/(float)10.0);
	Gps_info_s.heading_d=0;
	Gps_info_s.fix=2;

	/*
	 * Test complete
	 */
	AISMessage *msg;
	AISMessage123 msg123;
	AISMessage18 msg18;
	AISMessage24A msg24A;
	AISMessage24B msg24B;
	AISMessage4 msg4;
	AISMessage12 msg12;
	switch (msg_type_u8)
	{
	case MSG_1:
	case MSG_2:
	case MSG_3:
	{
		LOG_I("AISMessage123");
		msg=&msg123;
		msg->repeat=3;
		msg->accuracy=1;
		msg->status=5;
		msg->turn=124;
		msg->speed_kt=50;
		msg->lat_d=msg->double2lat_d(LAT_D);
		msg->lon_d=msg->double2lon_d(LON_D);
		msg->heading_d=HEADING_NOT_AVAILABLE;
		msg->cog_d=250;
		msg->second=40;
		msg->raim=1;
		break;
	}
	case MSG_18:
	{
		LOG_I("AISMessage18");
		msg=&msg18;
		msg->repeat=3;
		msg->accuracy=1;
		msg->status=5;
		msg->cs=1;
		msg->display=1;
		msg->dsc=1;
		msg->band=1;
		msg->msg22=1;
		msg->assigned=1;
		msg->speed_kt=50;
		msg->lat_d=msg->double2lat_d(LAT_D);
		msg->lon_d=msg->double2lon_d(LON_D);
		msg->heading_d=HEADING_NOT_AVAILABLE;
		msg->cog_d=250;
		msg->second=40;
		msg->raim=1;
		break;
	}
	case 240:
	{
		LOG_I("AISMessage24A");
		msg=&msg24A;

		msg->repeat=3;
		break;
	}
	case 241:
	{
		LOG_I("AISMessage24B");
		msg=&msg24B;

		msg->repeat=3;
		msg->shiptype=SAILING;
		break;
	}
	case MSG_4:
	{
		LOG_I("AISMessage4");
		msg=&msg4;
		msg->lat_d=msg->double2lat_d(LAT_D);
		msg->lon_d=msg->double2lon_d(LON_D);
		msg->repeat=3;
		break;
	}
	case MSG_12:
	{
		LOG_I("AISMessage12");
		msg=&msg12;
		msg->d_mmsi=6789;
		msg->repeat=3;
		/*
		 * MSG12_TXT_SZ chars
		 */
		strncpy(((AISMessage12*)msg)->txt_ac,"HELLO WORLD 1234",sizeof(((AISMessage12*)msg)->txt_ac));
		break;
	}
	case M312_REFERENCE_MSG:
	{
		LOG_I("M312_BINARY_MSG");
		uint8_t data[28]={0x33,0x33,0x33,0x7f,0xa9,0x94,0xaa,0x91,0x4a,0x55,0xae,0x62,0x7a,0x89,0x88,0xb4,0x57,0xec,0x71,0x34,0xcf,0xbc,0xb5,0x6a,0x8c,0xd1,0x0e,0x7f};
		break;
	}
	/*
	 * --------------------------------
	 * Msg 118
	 * --------------------------------
	 */
	case M312_MSG181:
	{
		LOG_I("M312_MSG181");
		/*
		 * RX CRC OK
		 */
		uint8_t data_recv[32]={0xcc,0x5f,0xdb,0x11,0x27,0x4a,0x47,0x55,0x15,0xf8,0x8d,0xe1,0x85,0xc1,0x8b,0x08,0x0a,0x5c,0xfd,0xa8,0x77,0x4c,0xf2,0x12,0x18,0x07,0xfc,0x42,0x00,0x06,0x92,0xff};
		//si446x_2.rx_get_ais_test(data_recv,32<<3);
		break;
	}
	/*
	 * --------------------------------
	 * Msg 119
	 * --------------------------------
	 */
	case M312_MSG182:
	{
		LOG_I("M312_MSG182");
		/*
		 * RX CRC OK
		 */
		uint8_t data_recv[32]={0xfc,0x45,0xb5,0x19,0xcb,0x56,0x54,0xaa,0xbe,0xe2,0x57,0x4f,0x1d,0xcd,0x40,0xe1,0xae,0x5e,0xb6,0xad,0xaa,0xaa,0x3a,0x55,0xf1,0xf7,0x88,0x02,0xa0,0x00,0x02,0xff};
		//si446x_2.rx_get_ais_test(data_recv,32<<3);
		break;
	}
	default:
		LOG_W("No message id");
		return;
	}

	/*
	 * Encode IAS message
	 */
	tx_packet_s.reset();
	msg->encode(Station_data_s,tx_packet_s);
	tx_packet_s.ais_finalize();
	LOG_D("gcode_M312 : tx_packet_s");
	tx_packet_s.print_bytes();
	usleep(1000*1000);

	/*
	 * Send msg to db_update thread.
	 */
	if(0){
		RXPacket rx_packet_s(MAX_AIS_RX_PACKET_SIZE);
		memcpy(rx_packet_s.mPacket,tx_packet_s.mPacket,ORB_AIS_PACKET);
		rx_packet_s.print_bytes();
		rx_ais_decode(rx_packet_s,0xFF);
	}

	if(1){
		struct orb_ais_db_update_s ais_s;
		int ptopic_ais;
		memcpy(ais_s.packet_au8,tx_packet_s.mPacket,ORB_AIS_PACKET);
		ais_s.id_u8=3;

		ptopic_ais=orb_advertise_queue(ORB_ID(orb_ais_db_update),&ais_s,ORB_AIS_DB_UPDATE_QUEUE_SIZE);
		if (ptopic_ais<0)
		{
			LOG_E("timer_thread: orb_ais_db_update advertise failed: %d",errno);
		}
		orb_publish(ORB_ID(orb_ais_db_update),ptopic_ais,&ais_s);
		orb_unadvertise(ptopic_ais);
	}


}



/*
 * --------------------------
 * M316 T<type 1 or 18 > M<mmsi> A<azimut_deg> R<range_nm> H<heading_deg> S<speed_kt10> C<myheading_deg> V<myspeed_kt10>
 * --------------------------
 * Test monitoring
 * M316 T18 R2 A90 H270 M1111 S60 C250 V45
 * M316 T18 R2 A315 H135 M1111 S60 C250 V45
 *
 * Quadrant 1 : M316 T18 R4 A90 H315 M1111 S60 C45 V60
 *    - relat_speed_kt(8.5) -> sqrt(2)*6
 *    - rel_heading_d(225.0)
 *    - rel_pos(3.993;-0.003NM) - rel_speed_kt(-8.485;0.000Kt)
 *    - cpa_d64(0.0NM) - cross(1)
 *    - time_to_cpa_mn_u32(28)
 * Quadrant 2 : M316 T18 R4 A135 H270 M1111 S60 C180 V60
 * Quadrant 3 : M316 T18 R4 A225 H90 M1111 S60 C180 V60
 * Quadrant 4 : M316 T18 R4 A315 H90 M1111 S60 C0 V60
 *
 * --------------------------
 */
void gcode_M316()
{
	double range_nm_d64;
	uint8_t msg_type_u8=code_seen('T')?code_value_ushort():18;
	uint32_t mmsi_u32=code_seen('M')?code_value_ulong():1234;
	int32_t heading_d_i32=code_seen('H')?code_value_int():0;
	uint32_t range_nm_u32=code_seen('R')?code_value_ulong():2;
	int32_t azimut_d_i32=code_seen('A')?code_value_int():0;
	int32_t myheading_d_i32=code_seen('C')?code_value_int():0;
	int32_t myspeed_kt_u32=code_seen('V')?code_value_int():45;
	uint32_t speed_kt_u32=code_seen('S')?code_value_int():60;

	Ais_test_vessel vessel;
	StationData station_s;

	if (heading_d_i32<0) heading_d_i32+=360;
	if (azimut_d_i32<0) azimut_d_i32+=360;
	LOG_W("M316: M(%d) A(%d) R(%d) H(%d)",mmsi_u32,azimut_d_i32,range_nm_u32,heading_d_i32);

	/*
	 * Saint-Malo
	 */
	Gps_info_s.lat_d=(int32_t)((float)48.6472222*LAT_LONG_SCALE);
	Gps_info_s.lon_d=(int32_t)((float)-2.0088889*LAT_LONG_SCALE);
	Gps_info_s.speed_kt=((float)myspeed_kt_u32/(float)10.0);
	Gps_info_s.heading_d=myheading_d_i32;
	Gps_info_s.fix=2;

	station_s.set_shipname((char*)"TETE D'ARTICHAUT      ");
	station_s.mmsi=mmsi_u32;
	station_s.shiptype=SAILING;
	vessel.init(station_s,Gps_info_s);
	vessel.new_postion((double)azimut_d_i32,(double)range_nm_u32,(float)speed_kt_u32/(float)10.0,(double)heading_d_i32);
	range_nm_d64=Ais_monitoring::range3_nm((float)vessel.gps_i_s.lon_d/LAT_LONG_SCALE,(float)vessel.gps_i_s.lat_d/LAT_LONG_SCALE);
	LOG_W("range_nm_d64(%.1f)",range_nm_d64);

	vessel.new_mmsi(msg_type_u8,mmsi_u32,range_nm_d64);
	vessel.new_mmsi(AIS_MSG_TYPE_24A,mmsi_u32,range_nm_d64);
	vessel.new_mmsi(AIS_MSG_TYPE_24B,mmsi_u32,range_nm_d64);
}




/*
 * ------------------------
 * M999: Restart after being stopped
 * ------------------------
 */
void gcode_M999() {
	LOG_I("reboot");
}


/*
 * --------------------------
 * process_next_command
 * --------------------------
 */
void process_next_command() {
	current_command = command_queue[cmd_queue_index_r];
	while (*current_command == ' ') ++current_command;
	char *cmd_ptr = current_command;
	LOG_I("process_next_command: %s",current_command);

	// Get the command code, which must be G, M, or T
	char command_code = *cmd_ptr++;

	// Skip spaces to get the numeric part
	while (*cmd_ptr == ' ') cmd_ptr++;

	uint16_t codenum = 0; // define ahead of goto

	// Bail early if there's no code
	bool code_is_good = NUMERIC(*cmd_ptr);
	if (!code_is_good) goto ExitUnknownCommand;

	// Get and skip the code number
	do {
		codenum = (codenum * 10) + (*cmd_ptr - '0');
		cmd_ptr++;
	} while (NUMERIC(*cmd_ptr));


	// Skip all spaces to get to the first argument, or null
	while (*cmd_ptr == ' ') cmd_ptr++;

	// The command's arguments (if any) start here, for sure!
	current_command_args = cmd_ptr;


	// Handle a known G, M, or T
	switch (command_code) {
	case 'M': switch (codenum) {

	case 000: // Help
		gcode_M000();
		break;

	case 001: // About
		gcode_M001();
		break;

	case 101: // AIS
		gcode_M101();
		break;

	case 102: // AIS
		gcode_M102();
		break;

	case 103: // AIS
		gcode_M103();
		break;

	case 104: // AIS
		gcode_M104();
		break;

	case 312: // Debug
		gcode_M312();
		break;

	case 316: // Debug
		gcode_M316();
		break;


	case 600: // M600: Store settings in EEPROM
		gcode_M600();
		break;
	case 601: // M601: Read settings from EEPROM
		gcode_M601();
		break;
	case 602: // M602: Revert to default settings
		gcode_M602();
		break;
	case 603: // M603: print settings currently in memory
		gcode_M603();
		break;

	case 700: // M700: set GPS serial bauds
		gcode_M700();
		break;

	case 999: // M999: Restart after being Stopped
		gcode_M999();
		break;
	}
	break;

	case 'T': switch (codenum) {
	case 99:

		break;
	}
	break;


	default: code_is_good = false;
	printf("Command error!\n");
	break;
	}

	/*
	 * Anyway executed when function finishes
	 */
	ExitUnknownCommand:
	if (!code_is_good) {
		unknown_command_error();
	}else{
		ok_to_send();
	}
}

void ok_to_send(){
	if (!send_ok[cmd_queue_index_r]) return;
	printf("-> ok\n");
}



/*
 * --------------------
 * serial_thread
 * --------------------
 */
FAR void *serial_thread(pthread_addr_t arg)
{

	struct pollfd fds[1];
	struct mng_msg_s sample;
	bool updated;
	int sfd;
	int ret;

	// Subscribe
	if ((sfd = orb_subscribe(ORB_ID(mng_msg))) < 0)
	{
		printf("mng_subscriber_task: subscribe failed: %d\n", errno);
		return NULL;
	}

	/*
	 * Get mng_msg published messages
	 */
	do
	{
		// Check and get
		orb_check(sfd, &updated);
		if (updated)
		{
			orb_copy(ORB_ID(mng_msg),sfd,&sample);
		}
	}
	while (updated);

	fds[0].fd     = sfd;
	fds[0].events = POLLIN;

	/*
	 * Load settings
	 */
	Ais_settings::begin();
	Ais_settings::load();


	/*
	 * Loop
	 */
	while(1){
		int poll_ret;

		// Timeout
		poll_ret = poll(fds,1,TIMEOUT_UORB_US);
		if (!poll_ret){
			printf("serial_task: poll timeout\n");
		}

		if (OK!=orb_check(sfd, &updated))
		{
			 printf("serial_task: check failed\n");
			 return NULL;
		}
		else if (poll_ret < 0 && errno != EINTR)
		{
			LOG_E("serial_task: poll error (%d, %d)\n", poll_ret, errno);
		}

		if (fds[0].revents & POLLIN)
		{
			orb_copy(ORB_ID(mng_msg),sfd,&sample);
			printf("serial_task: %s\n",sample.cmd_cha);
			enqueue_and_echo_commands_P(sample.cmd_cha);
		}
		if (commands_in_queue<LOGGER_BUFSIZE) get_available_commands();

		if (commands_in_queue) {
			process_next_command();
			// The queue may be reset by a command handler or by code invoked by idle() within a handler
			if (commands_in_queue) {
				--commands_in_queue;
				if (++cmd_queue_index_r >= LOGGER_BUFSIZE) cmd_queue_index_r = 0;
			}
		}
	}

	// unsubscribe
	ret = orb_unsubscribe(sfd);
	if (ret != OK)
	{
		printf("serial_task: orb_unsubscribe failed: %i", ret);
		return NULL;
	}
	return NULL;
}


/*
 * print_doais
 */
void print_doais()
{
	printf(
			"\n"
			"	 ))))))    ))))))    ))))))     DDDDDDDDDDDDD                                      AAA                 iiii                   \n"
			"	)::::::)) )::::::)) )::::::))   D::::::::::::DDD                                  A:::A               i::::i                  \n"
			"	 ):::::::))):::::::))):::::::)) D:::::::::::::::DD                               A:::::A               iiii                   \n"
			"	  )):::::::))):::::::))):::::::)DDD:::::DDDDD:::::D                             A:::::::A                                     \n"
			"	    )::::::)  )::::::)  )::::::)  D:::::D    D:::::D    ooooooooooo            A:::::::::A           iiiiiii     ssssssssss   \n"
			"	     ):::::)   ):::::)   ):::::)  D:::::D     D:::::D oo:::::::::::oo         A:::::A:::::A          i:::::i   ss::::::::::s  \n"
			"	     ):::::)   ):::::)   ):::::)  D:::::D     D:::::Do:::::::::::::::o       A:::::A A:::::A          i::::i ss:::::::::::::s \n"
			"	     ):::::)   ):::::)   ):::::)  D:::::D     D:::::Do:::::ooooo:::::o      A:::::A   A:::::A         i::::i s::::::ssss:::::s\n"
			"	     ):::::)   ):::::)   ):::::)  D:::::D     D:::::Do::::o     o::::o     A:::::A     A:::::A        i::::i  s:::::s  ssssss \n"
			"	     ):::::)   ):::::)   ):::::)  D:::::D     D:::::Do::::o     o::::o    A:::::AAAAAAAAA:::::A       i::::i    s::::::s      \n"
			"	     ):::::)   ):::::)   ):::::)  D:::::D     D:::::Do::::o     o::::o   A:::::::::::::::::::::A      i::::i       s::::::s   \n"
			"	    )::::::)  )::::::)  )::::::)  D:::::D    D:::::D o::::o     o::::o  A:::::AAAAAAAAAAAAA:::::A     i::::i ssssss   s:::::s \n"
			"	  )):::::::))):::::::))):::::::)DDD:::::DDDDD:::::D  o:::::ooooo:::::o A:::::A             A:::::A   i::::::is:::::ssss::::::s\n"
			"	 ):::::::))):::::::))):::::::)) D:::::::::::::::DD   o:::::::::::::::oA:::::A               A:::::A  i::::::is::::::::::::::s \n"
			"	)::::::)  )::::::)  )::::::)    D::::::::::::DDD      oo:::::::::::ooA:::::A                 A:::::A i::::::i s:::::::::::ss  \n"
			"	 ))))))    ))))))    ))))))     DDDDDDDDDDDDD           ooooooooooo AAAAAAA                   AAAAAAAiiiiiiii  sssssssssss    \n"
			"	                                                                                                                              \n"
			"	                                                                                                                              \n"
			"	                                                                                                                              \n"
			"	                                                                                                                              \n"
			"	                                                                                                                              \n"
			"\n"
	);
}

