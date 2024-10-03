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
#include "circular_queue.h"
#include "ais_monitoring.h"
#include "types.h"
#include "doais_mng.h"

/*
 * --------------------------
 * Defines
 * --------------------------
 */
#define USLEPP_50MS (50*1000)
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
 * M999: Restart after being stopped
 * ------------------------
 */
void gcode_M999() {
	LOG_I("reboot");
	//esp_restart();
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

	/* Get all published messages,
	 * ensure that publish and subscribe message match
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

