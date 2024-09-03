/**
 * =====================================
 *  ais_main.cpp
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

#include "version.h"
#include "configuration_store.h"


/*
 * --------------------------
 * Globals data
 * --------------------------
 */


/*
 * --------------------------
 * Circular buffers
 * --------------------------
 */
typedef uint8_t ais_buf_t[MAX_AIS_RX_PACKET_SIZE>>3];
Circular_queue<ais_buf_t> ais_rx_buf(AIS_RX_BUF_SZ);
Circular_queue_simple<uint8_t> ais_channel_buf(AIS_RX_BUF_SZ,0);
Ais_monitoring Monitoring(AIS_CHAINED_LIST_MAX_SZ,AIS_CHAINED_LABEL_MAX_SZ);


/*
 * --------------------------
 * Prototypes
 * --------------------------
 */


/*
 * ------------------
 * Command Queue
 * ------------------
 */
bool Running = true;
//static long gcode_N;
static long gcode_LastN, Stopped_gcode_LastN = 0;
uint8_t commands_in_queue = 0; // Count of commands in the queue
static uint8_t cmd_queue_index_r = 0; // Ring buffer read position
static uint8_t cmd_queue_index_w = 0; // Ring buffer write position
static char command_queue[LOGGER_BUFSIZE][MAX_CMD_SIZE];
static char *current_command,*current_command_args,*seen_pointer;
static const char *injected_commands_P = NULL;
// Number of characters read in the current line of serial input
static int serial_count = 0;
static bool send_ok[LOGGER_BUFSIZE];

void get_available_commands();
void process_next_command();

static bool drain_injected_commands_P() {
	if (injected_commands_P != NULL) {
		uint8_t i = 0;
		char c, cmd[30];
		strncpy_P(cmd, injected_commands_P, sizeof(cmd) - 1);
		cmd[sizeof(cmd) - 1]='\0';
		while ((c = cmd[i]) && c != '\n') i++; // find the end of this gcode command
		cmd[i]='\0';
		if (enqueue_and_echo_command(cmd))     // success?
			injected_commands_P = c ? injected_commands_P + i + 1 : NULL; // next command or done
	}
	return (injected_commands_P != NULL);    // return whether any more remain
}

void enqueue_and_echo_commands_P(const char* pgcode) {
	injected_commands_P = pgcode;
	drain_injected_commands_P(); // first command executed asap (when possible)
}

void clear_command_queue() {
	cmd_queue_index_r = cmd_queue_index_w;
	commands_in_queue = 0;
}

inline void _commit_command(bool say_ok) {
	send_ok[cmd_queue_index_w] = say_ok;
	if (++cmd_queue_index_w >= LOGGER_BUFSIZE) cmd_queue_index_w = 0;
	commands_in_queue++;
}

inline bool _enqueuecommand(const char* cmd, bool say_ok=false) {
	if (*cmd == ';' || commands_in_queue >= LOGGER_BUFSIZE) return false;
	strcpy(command_queue[cmd_queue_index_w], cmd);
	_commit_command(say_ok);
	return true;
}

bool enqueue_and_echo_command(const char* cmd, bool say_ok/*=false*/) {
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

inline void get_serial_commands() {
	static char serial_line_buffer[MAX_CMD_SIZE];
	static bool serial_comment_mode = false;

	/**
	 * Loop while serial characters are incoming and the queue is not full
	 */
	while (commands_in_queue < LOGGER_BUFSIZE && SERIAL_LOGGER.available() > 0) {

		char serial_char = SERIAL_LOGGER.read();

		/**
		 * If the character ends the line
		 */
		if (serial_char == '\n' || serial_char == '\r') {

			serial_comment_mode = false; // end of line == end of comment

			if (!serial_count) continue; // skip empty lines

			serial_line_buffer[serial_count] = 0; // terminate string
			serial_count = 0; //reset buffer

			char* command = serial_line_buffer;

			while (*command == ' ') command++; // skip any leading spaces

			// Movement commands alert when stopped
			if (IsStopped()) {
				char* gpos = strchr(command, 'G');
				if (gpos) {
					const int codenum = strtol(gpos + 1, NULL, 10);
					switch (codenum) {
					case 0:
					case 1:
					case 2:
					case 3:
						printf("Error: Device stopped due to errors. Fix the error and use M999 to restart.\n");
						break;
					}
				}
			}

#if defined(NO_TIMEOUTS) && NO_TIMEOUTS > 0
			last_command_time = ms;
#endif

			// Add the command to the queue
			_enqueuecommand(serial_line_buffer, true);
		}
		else if (serial_count >= MAX_CMD_SIZE - 1) {
			// Keep fetching, but ignore normal characters beyond the max length
			// The command will be injected when EOL is reached
		}
		else if (serial_char == '\\') {  // Handle escapes
			if (SERIAL_LOGGER.available() > 0) {
				// if we have one more character, copy it over
				serial_char = SERIAL_LOGGER.read();
				if (!serial_comment_mode) serial_line_buffer[serial_count++] = serial_char;
			}
			// otherwise do nothing
		}
		else { // it's not a newline, carriage return or escape char
			if (serial_char == ';') serial_comment_mode = true;
			if (!serial_comment_mode) serial_line_buffer[serial_count++] = serial_char;
		}

	} // queue has space, serial has data
}


void get_available_commands() {

	// if any immediate commands remain, don't get other commands yet
	if (drain_injected_commands_P()) return;

	get_serial_commands();
}

inline bool code_has_value() {
	int i = 1;
	char c = seen_pointer[i];
	while (c == ' ') c = seen_pointer[++i];
	if (c == '-' || c == '+') c = seen_pointer[++i];
	if (c == '.') c = seen_pointer[++i];
	return NUMERIC(c);
}

inline float code_value_float() {
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
	if (ais_wifi::state==AIS_WIFI_ON)
	{
		ais_wifi::wifi_udp_write(help_str,strlen(help_str));
#if DEV_MODE
		ais_wifi::wifi_udp_write(debug_str,strlen(debug_str));
#endif
	}
	printf("%s",help_str);
#if DEV_MODE
	printf("%s",debug_str);
#endif
}

/*
 * ------------------------
 * print_doais
 * ------------------------
 * https://onlineasciitools.com/convert-text-to-ascii-art
 * Doh
 * ------------------------
 */
inline void print_doais(){
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


/*
 * ------------------------
 * M001
 * ------------------------
 * About
 * ------------------------
 */
inline void gcode_M001(){
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
		printf("M102 error: %s - AIS incompatible characters\n",current_command_args);
		return;
	}
	uint8_t len_u8=min(strlen(current_command_args),sizeof(Station_data_s.shipname));
	LOG_I("args: %s - len: %d",args,len_u8);
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
		printf("M103 error: %s - AIS incompatible characters\n",current_command_args);
		return;
	}
	uint8_t len_u8=min(strlen(current_command_args),sizeof(Station_data_s.callsign));
	LOG_I("args: %s - len: %d",args,len_u8);
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
 * M110
 * ------------------------
 * Wifi
 * M110 <on_off>
 * ------------------------
 */
inline void gcode_M110()
{
	ais_wifi::on_off();
}

/*
 * ------------------------
 * M111
 * ------------------------
 * Wifi
 * M111 <mode_ap_sta>
 * ------------------------
 */
inline void gcode_M111()
{
	ais_wifi::on_ap_sta();
}


/*
 * ------------------------
 * M112
 * ------------------------
 * Wifi
 * M112 <ap_ssid>
 * ------------------------
 */
inline void gcode_M112()
{
	const char * const args = current_command_args;
	uint8_t len_u8=min(strlen(current_command_args),sizeof(ais_wifi::ap_ssid));
	LOG_I("args: %s - len: %d",args,len_u8);
	if (len_u8) strncpy(ais_wifi::ap_ssid,args,len_u8);
}

/*
 * ------------------------
 * M113
 * ------------------------
 * Wifi
 * M113 <ap_pwd>
 * ------------------------
 */
inline void gcode_M113()
{
	const char * const args = current_command_args;
	uint8_t len_u8=min(strlen(current_command_args),sizeof(ais_wifi::ap_pwd));
	LOG_I("args: %s - len: %d",args,len_u8);
	if (len_u8) strncpy(ais_wifi::ap_pwd,args,len_u8);
}


/*
 * ------------------------
 * M114
 * ------------------------
 * Wifi
 * M114 <sta_ssid>
 * ------------------------
 */
inline void gcode_M114()
{
	const char * const args = current_command_args;
	uint8_t len_u8=min(strlen(current_command_args),sizeof(ais_wifi::sta_ssid));
	LOG_I("args: %s - len: %d",args,len_u8);
	if (len_u8) strncpy(ais_wifi::sta_ssid,args,len_u8);
}

/*
 * ------------------------
 * M115
 * ------------------------
 * Wifi
 * M115 <sta_pwd>
 * ------------------------
 */
inline void gcode_M115()
{
	const char * const args = current_command_args;
	uint8_t len_u8=min(strlen(current_command_args),sizeof(ais_wifi::sta_pwd));
	LOG_I("args: %s - len: %d",args,len_u8);
	if (len_u8) strncpy(ais_wifi::sta_pwd,args,len_u8);
}



/*
 * ------------------------
 * M117
 * ------------------------
 * NMEA
 * M115 <rs422_on_u8>
 * ------------------------
 */
inline void gcode_M117()
{
	Ais_nmea_gps_s.gps_nmea_on_off();
}


/*
 * ------------------------
 *   M300 U<val_u32> n<nb_bits>
 * ------------------------
 *  Debug
 * ------------------------
 */

/*
 * Test endianess
 */
void gcode_M300()
{
	uint32_t val_u32;
	uint8_t nb_u8;
	val_u32=(uint32_t)code_seen('U')?code_value_ushort():0;
	nb_u8=(uint8_t)code_seen('N')?code_value_ushort():0;
	LOG_D("val_u32(%d) - nb_u8(%d)",val_u32,nb_u8);
	endianess_check __checkendianess;
	__checkendianess.print();
	if ((val_u32>0)&&(nb_u8<32)) __checkendianess.bytes2bits(val_u32,nb_u8);
	Monitoring.beep();
	delay(1000);
	Monitoring.beep();
}





/*
 * -------------------------
 * gcode_M311
 * -------------------------
 * M311 N4
 * Send N AIS messages
 * Set channel
 * -------------------------
 */
void gcode_M311()
{
	uint16_t msg_nb_u16,chan_u16;
	msg_nb_u16=code_seen('N')?code_value_ushort():0;
	chan_u16=code_seen('C')?code_value_ushort():0;

	switch(chan_u16){
	case CH_87:
	case CH_28:
	case CH_88:
		si446x_1.tx_packet_s.setChannel((VHFChannel)chan_u16);
		si446x_1.rx_packet_s.setChannel((VHFChannel)chan_u16);
		si446x_1.rx_start();
		si446x_2.tx_packet_s.setChannel((VHFChannel)chan_u16);
		si446x_2.rx_packet_s.setChannel((VHFChannel)chan_u16);
		si446x_2.rx_start();
		printf("Set channel(%d)",chan_u16);
		break;
	default:
		printf("Channel(%d) error",chan_u16);
		break;
	}

	if (!msg_nb_u16) return;

	AISMessage18 msg;
	/*
	 * Nantes
	 */
	msg.lat_d=msg.double2lat_d(47.22143353);
	msg.lon_d=msg.double2lon_d(-1.58430576);
	/*
	 * Saint Malo
	 */
	msg.lat_d=msg.double2lat_d(48.6472222);
	msg.lon_d=msg.double2lon_d(-2.0088889);
	msg.repeat=1;
	msg.speed_kt=50;
	msg.cog_d=250;
	msg.second=0;

	si446x_2.tx_build_ais_packet(&msg);

	for (int16_t i=0; i<msg_nb_u16;i++){
		si446x_2.tx_send_ais_packet();
		delay(200);
	}

	LOG_I("TX - si446x_2");
	si446x_2.si446x_get_info(1);
	si446x_2.version();
	si446x_2.get_status();
	si446x_2.get_state();
	delay(100);
	si446x_1.rx_get_ais_decode();
	LOG_I("RX - si446x_1");
	si446x_1.si446x_get_info(1);
	si446x_1.version();
	si446x_1.get_status();
	si446x_1.get_state();
}


/*
 * --------------------------
 * M312 T<type 1 or 18 or 240 or 241>
 * Test chain transmitter /receiver
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

void gcode_M312(){
	uint8_t msg_type_u8=code_seen('T')?code_value_ushort():0;
	printf("gcode_M312: msg_type_u8(%d) in {1,4,18,12,240,241,100,101,102,104,112,138}\n",msg_type_u8);

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
		msg->lat_d=msg->double2lat_d(47.21350504);
		msg->lon_d=msg->double2lon_d(-1.56928539);
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
		msg->lat_d=msg->double2lat_d(47.21350504);
		msg->lon_d=msg->double2lon_d(-1.56928539);
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
		msg->lat_d=msg->double2lat_d(47.21350504);
		msg->lon_d=msg->double2lon_d(-1.56928539);
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
		si446x_2.tx_packet_s.reset();
		si446x_2.tx_packet_s.copy_in(data,28);
		si446x_2.tx_packet_s.index_i16=28*8;
		break;
	}
	/*
	 * --------------------------------
	 * Msg 18
	 * --------------------------------
	 */
	case M312_MSG181:
	{
		LOG_I("M312_MSG181");
		/*
		 * RX CRC OK
		 */
		uint8_t data_recv[32]={0xcc,0x5f,0xdb,0x11,0x27,0x4a,0x47,0x55,0x15,0xf8,0x8d,0xe1,0x85,0xc1,0x8b,0x08,0x0a,0x5c,0xfd,0xa8,0x77,0x4c,0xf2,0x12,0x18,0x07,0xfc,0x42,0x00,0x06,0x92,0xff};
		si446x_2.rx_get_ais_test(data_recv,32<<3);
		break;
	}
	/*
	 * --------------------------------
	 * Msg 18
	 * --------------------------------
	 */
	case M312_MSG182:
	{
		LOG_I("M312_MSG182");
		/*
		 * RX CRC OK
		 */
		uint8_t data_recv[32]={0xfc,0x45,0xb5,0x19,0xcb,0x56,0x54,0xaa,0xbe,0xe2,0x57,0x4f,0x1d,0xcd,0x40,0xe1,0xae,0x5e,0xb6,0xad,0xaa,0xaa,0x3a,0x55,0xf1,0xf7,0x88,0x02,0xa0,0x00,0x02,0xff};
		si446x_2.rx_get_ais_test(data_recv,32<<3);
		break;
	}
	default:
		LOG_W("No message id");
		return;
	}

	switch (msg_type_u8)
	{
	case MSG_1:
	case MSG_2:
	case MSG_3:
	case MSG_18:
	case MSG_4:
	case MSG_12:
	case 240:
	case 241:
		si446x_2.tx_build_ais_packet(msg);
		si446x_1.rx_get_ais_test(si446x_2.tx_packet_s.get_pt(),si446x_2.tx_packet_s.bit_size());
		break;
	case M312_REFERENCE_MSG:
		si446x_1.rx_get_ais_test(si446x_2.tx_packet_s.get_pt(),si446x_2.tx_packet_s.bit_size());
		break;
	default:
		break;
	}
}


/*
 * ------------------------
 * M313 N20
 * Make stats on N AIS messages
 * ------------------------
 */
void gcode_M313()
{
	AISMessage18 msg;
	uint16_t msg_nb_u16;
	msg_nb_u16=code_seen('N')?code_value_ushort():0;
	printf("gcode_M313: N=%d\n",msg_nb_u16);

	/*
	 * Path length 177.35m
	 * Bearing 50.18deg
	 */
	msg.mmsi=Station_data_s.mmsi;
	msg.repeat=1;
	msg.speed_kt=50;
	msg.lat_d=msg.double2lat_d(47.21350504);
	msg.lon_d=msg.double2lon_d(-1.56928539);
	msg.heading_d=75;
	msg.cog_d=250;
	msg.second=23;

	si446x_2.tx_build_ais_packet(&msg);
	si446x_1.stats_init();
	for (int16_t i=0;i<msg_nb_u16;i++)
	{
		si446x_2.tx_send_ais_packet();
#if ENABLED(USE_WATCHDOG)
		watchdog_reset();
#endif
		delay(100);// Mini 100ms between 2 tx_send_ais_packet
		if (false)
		{
			si446x_1.stats_do();
			if (!ais_rx_buf.is_full())
			{
				uint8_t * data_pu8=ais_rx_buf.__new(si446x_1.rx_packet_s.nb_bytes());
				isr_LOG_I("new");
				si446x_1.rx_fastget_packet(data_pu8);
			}
			si446x_1.rx_faststart();
		}
	}
	if (false){
		LOG_D("ais_rx_buf=%d",ais_rx_buf.count());
		while (ais_rx_buf.count()>0){
			si446x_1.rx_packet_s.reset();
			/*
			 * Rx packet
			 */
			ais_rx_buf.dequeue(si446x_1.rx_packet_s.mPacket,si446x_1.rx_packet_s.nb_bytes());
			/*
			 * Channel
			 */
			uint8_t ch_u8;
			ch_u8=ais_channel_buf.dequeue();
			/*
			 * Decode
			 */
			si446x_1.rx_ais_decode(ch_u8);
		}
		si446x_1.stats_printf();

		LOG_I("RX - si446x_1");
		si446x_1.si446x_get_info(1);
		si446x_1.get_status();
		si446x_1.get_state();
		LOG_I("TX - si446x_2");
		si446x_2.si446x_get_info(1);
		si446x_2.get_status();
		si446x_2.get_state();
	}
}





/*
 * ------------------------
 * M315 D<direction> T<Tx or simu>
 * Test bench
 * ------------------------
 */
void gcode_M315()
{
	int16_t direction_i16, tx_i16;
	direction_i16=code_seen('D')?code_value_int():0;
	tx_i16=code_seen('T')?code_value_int():0;
	/*
	 * Nantes
	 */
	/*
	Gps_info_s.lat_d=(int32_t)((float)47.22143353*LAT_LONG_SCALE);
	Gps_info_s.lon_d=(int32_t)((float)-1.58430576*LAT_LONG_SCALE);
	Gps_info_s.fix=2;
	 */
	/*
	 * Saint Malo
	 */
	//Gps_info_s.lat_d=nmea_degree2ndeg((const double)48.6472222);
	//Gps_info_s.lon_d=nmea_degree2ndeg((const double)-2.0088889);
	Gps_info_s.lat_d=(int32_t)((float)48.6472222*LAT_LONG_SCALE);
	Gps_info_s.lon_d=(int32_t)((float)-2.0088889*LAT_LONG_SCALE);
	Gps_info_s.fix=2;

	/*
	 * Test bench
	 */
	LOG_D("Ais_test_bench");
	Ais_test_bench::init();

	/*
	 * Test bench
	 */
	if (tx_i16)
	{
		si446x_1.stats_init();
		Ais_test_bench::send_nmea_ais();
		Ais_test_bench::set_direction(direction_i16%360);
	}else
	{
		Ais_test_bench::new_mmsi();
		Ais_monitoring::refresh_b=MONOTORING_REFRESH_ASAP;
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
#define LOOP_SYNCHRO_WAIT_MAX_MS 300
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
	//if (xSemaphoreTake(ais_process_semaphore,LOOP_SYNCHRO_WAIT_MAX_MS/portTICK_PERIOD_MS)==pdTRUE)
	if (true)
	{
		//portENTER_CRITICAL_ISR(&Ais_process_mux);
		Display_on=false;
		Si446x_get_on=false;
		//portEXIT_CRITICAL_ISR(&Ais_process_mux);
		//if ((range_nm_d64<(double)Monitoring.settings_s.display_target_step_nm_u32*(double)MONITORING_DISPLAY_STEPS_NB+0.1))
		{
			vessel.new_mmsi(msg_type_u8,mmsi_u32,range_nm_d64);
			vessel.new_mmsi(AIS_MSG_TYPE_24A,mmsi_u32,range_nm_d64);
			vessel.new_mmsi(AIS_MSG_TYPE_24B,mmsi_u32,range_nm_d64);
			Ais_monitoring::refresh_b=MONOTORING_REFRESH_ASAP;
		}
		Si446x_get_on=true;
		Display_on=true;
	} else
	{
		//ais_wifi::wifi_printf("%s\n","Data process loop busy");
		LOG_D("%s\n","Data process loop busy");
	}
}


/*
 * --------------------------
 * M320
 * --------------------------
 * Test GPS Nmea library
 * M320 $GNRMC,201501.00,A,4838.34171,N,00201.35353,W,0.032,,021022,,,D*7B
 *   => Lat: 48.64 deg, Lon: -2.02 deg, Sig: 2, Fix: 0
 * M320 $GPRMC,210230,A,3855.4487,N,09446.0071,W,0.0,076.2,130495,003.8,E*69
 * 	 => Lat: 38.92 deg, Lon: -94.77 deg, Sig: 2, Fix: 0
 * M320 $GPGGA,210230,3855.4487,N,09446.0071,W,1,07,1.1,370.5,M,-29.5,M,,*7A
 * 	=> Lat: 38.92 deg, Lon: -94.77 deg, Sig: 1, Fix: 0
 * M320 $GPGGA,191723.023,5232.978,N,01326.836,E,1,12,1.0,0.0,M,0.0,M,,*6B
 *  => Lat: 52.55 deg, Lon: 13.45 deg, Sig: 1, Fix: 3
 * --------------------------
 */
void gcode_M320()
{
	gps_data_t gps_info_s;
	uint8_t len_u8;

	memset((void*)&gps_info_s,0,sizeof(gps_info_s));

	len_u8=strlen(current_command_args);
	len_u8=(len_u8>(MAX_CMD_SIZE-1)) ? MAX_CMD_SIZE-1:len_u8;
	LOG_D("%s - len: %d",current_command_args,len_u8);

	if (len_u8)
	{
		current_command_args[len_u8]='\0';
		Ais_nmea_gps_s.reset();
		Ais_nmea_gps_s.copy2buf(current_command_args,len_u8);
		Ais_nmea_gps_s.parser.receive(0);
		Ais_nmea_gps_s.update_gps_info(&gps_info_s);
		LOG_I("Lat: %.2f deg, Lon: %.2f deg, Fix: %d",(float)gps_info_s.lat_d/LAT_LONG_SCALE,(float)gps_info_s.lon_d/LAT_LONG_SCALE,gps_info_s.fix);
		LOG_I("speed_kt(%.1f) - heading_d(%.0f)",Gps_info_s.speed_kt,Gps_info_s.heading_d);
		LOG_I("Year(%d) - Day(%d) - Hour(%d:%d:%d)",Gps_info_s.utc_s.tm_year,Gps_info_s.utc_s.tm_mday,Gps_info_s.utc_s.tm_hour,Gps_info_s.utc_s.tm_min,Gps_info_s.utc_s.tm_sec);

	}
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
	bool onwifi=(code_seen('W')and (ais_wifi::state==AIS_WIFI_ON))?true:false;
	Ais_settings::report(onwifi);
}

/*
 * ------------------------
 * M700 f<1> b<1> - Serial GPS : flush or begin\n\
 * ------------------------
 */
inline void gcode_M700()
{
	uint32_t bauds_u32=code_seen('B')?code_value_ulong():9600;
	printf("GPS serial bauds(%d)\n",bauds_u32);
	Monitoring.settings_s.gps_bauds_u32=bauds_u32;
	//Ais_nmea_gps_s.set_baudrate();
}



/*
 * ------------------------
 * M999: Restart after being stopped
 * ------------------------
 */
inline void gcode_M999() {
	LOG_I("reboot");
	esp_restart();
}


/*
 * --------------------------
 * process_next_command
 * --------------------------
 */
void process_next_command() {
	current_command = command_queue[cmd_queue_index_r];
	// Sanitize the current command:
	//  - Skip leading spaces
	//  - Bypass N[-0-9][0-9]*[ ]*
	//  - Overwrite * with null to mark the end
	while (*current_command == ' ') ++current_command;
	/*HBL021022
	if (*current_command == 'N' && NUMERIC_SIGNED(current_command[1])) {
		current_command += 2; // skip N[-0-9]
		while (NUMERIC(*current_command)) ++current_command; // skip [0-9]*
		while (*current_command == ' ') ++current_command; // skip [ ]*
	}
	 */
	/* HBL021022
	char* starpos = strchr(current_command, '*');  // * should always be the last parameter
	if (starpos) while (*starpos == ' ' || *starpos == '*') *starpos-- = '\0'; // nullify '*' and ' '
	 */
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

	case 110: // Wifi
		gcode_M110();
		break;

	case 111: // Wifi
		gcode_M111();
		break;

	case 112: // Wifi
		gcode_M112();
		break;

	case 113: // Wifi
		gcode_M111();
		break;

	case 114: // Wifi
		gcode_M112();
		break;

	case 115: // Wifi
		gcode_M115();
		break;

	case 117: // MNEA
		gcode_M117();
		break;

	case 300: // Test endianess
		gcode_M300();
		break;

	case 311: // M311 N4 - Send N AIS messages
		gcode_M311();
		break;

	case 312: // Test encode - decode
		gcode_M312();
		break;

	case 313: // Make stats on N AIS messages
		gcode_M313();
		break;


	case 315: // Test
		gcode_M315();
		break;

	case 316: // Test
		gcode_M316();
		break;

	case 320: // Test
		gcode_M320();
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
	case 700: // M700: flush or begin GPS serial
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



/*
 * ====================
 * Main loop
 * --------------------
 */

void loop()
{
	uint32_t local_beep_u32;
	LOG_W("Starting loop core %d",xPortGetCoreID());

	local_beep_u32=2;
	while(true){
		if (ais_wifi::state==AIS_WIFI_ON)
		{
			if (ais_wifi::wifi_udp_read(Wifi_udp_read_char_a,WIFI_UDP_STR_SZ)){
				injected_commands_P=Wifi_udp_read_char_a;
			}
		}
		if (commands_in_queue < LOGGER_BUFSIZE) get_available_commands();

		if (commands_in_queue) {
			process_next_command();
			// The queue may be reset by a command handler or by code invoked by idle() within a handler
			if (commands_in_queue) {
				--commands_in_queue;
				if (++cmd_queue_index_r >= LOGGER_BUFSIZE) cmd_queue_index_r = 0;
			}
		}

		//if (Monitoring.display_status==MONITORING_DISPLAY_STATUS_ALERT)
		if ((Monitoring.display_status==MONITORING_DISPLAY_STATUS_ALERT)||(Monitoring.sys_status_u8))
		{
			if (Timer_ticks_beep_u32)
			{
				portENTER_CRITICAL_ISR(&Timer_mux);
				Timer_ticks_beep_u32=0;
				portEXIT_CRITICAL_ISR(&Timer_mux);
				local_beep_u32++;
				LOG_V("local_beep_u32 %d",local_beep_u32);
				if ((local_beep_u32%4)==1)
				{
					Monitoring.voice();
					local_beep_u32=2;

				} else
				{
					Monitoring.beep();
				}
			}
		}
		delay(50);
	}
}



