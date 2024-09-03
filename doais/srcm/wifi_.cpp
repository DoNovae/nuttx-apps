/**
 * =====================================
 * wifi_cpp.h
 * -------------------------------------
 *  AIS DoNovae
 *  www.DoNovae.com
 *  Herve Bailly
 * =====================================
 */
#include <WiFiUDP.h>
#include <stdarg.h>
#include "ais.h"
#include "wifi_.h"
#include "ais_monitoring.h"
#include "gps.h"


void print_ipaddr(char const *cmt_pc,IPAddress *ipaddr_pa){
	LOG_I("%s %d.%d.%d.%d",cmt_pc, (*ipaddr_pa)[0],(*ipaddr_pa)[1],(*ipaddr_pa)[2],(*ipaddr_pa)[3]);
}

void ipaddr2str(IPAddress *ipaddr_pa,char *ipaddr_pc, uint8_t len_u8)
{
	memset((void*)ipaddr_pc,0,len_u8);
	snprintf(ipaddr_pc,len_u8,"%d.%d.%d.%d",(*ipaddr_pa)[0],(*ipaddr_pa)[1],(*ipaddr_pa)[2],(*ipaddr_pa)[3]);
}


void str2ipaddr(char *ipaddr_pc, uint8_t len_u8,IPAddress *ipaddr_pa)
{
   unsigned int byte3=0;
   unsigned int byte2=0;
   unsigned int byte1=0;
   unsigned int byte0=0;
   char dummyString[2];

   /* The dummy string with specifier %1s searches for a non-whitespace char
    * after the last number. If it is found, the result of sscanf will be 5
    * instead of 4, indicating an erroneous format of the ip-address.
    */
   if (sscanf(ipaddr_pc, "%u.%u.%u.%u%1s",&byte3,&byte2,&byte1,&byte0,dummyString)== 4)
   {
      if ((byte3<256)&&(byte2<256)&&(byte1<256)&&(byte0<256))
      {
    	  (*ipaddr_pa)[0]=byte0;
    	  (*ipaddr_pa)[1]=byte1;
    	  (*ipaddr_pa)[2]=byte2;
    	  (*ipaddr_pa)[3]=byte3;
      }
   }
}

/*
 * -------------------
 * Wifi
 * -------------------
 *   +Set web server port number to 80
 * -------------------
 */

uint8_t ais_wifi::on_u8;
uint8_t ais_wifi::mode_ap_u8;
char ais_wifi::ap_ssid[21];
char ais_wifi::ap_pwd[21];
char ais_wifi::sta_ssid[21];
char ais_wifi::sta_pwd[21];
WiFiUDP ais_wifi::Udp;


/*
 * IP address to send UDP data to:
 * either use the ip address of the server or
 * a network broadcast address 192.168.0.255
 */
IPAddress ais_wifi::local_ip(0,0,0,0);
IPAddress ais_wifi::client_ip(192,168,4,255);
int ais_wifi::udp_client_port=WIFI_UDP_CLIENT_PORT;
int ais_wifi::udp_local_port=WIFI_UDP_LOCAL_PORT;

//IPAddress ais_wifi::sta_local_ip(10,1,1,1);
//IPAddress ais_wifi::sta_gateway_ip(10,1,1,1);
//IPAddress ais_wifi::subnet_ip(255,255,255,0);



/*
 * -------------------
 * start
 * -------------------
 *
 */
/*
Serial.print("IP address: ");
Serial.println(WiFi.localIP());
Serial.print("ESP Mac Address: ");
Serial.println(WiFi.macAddress());
Serial.print("Subnet Mask: ");
Serial.println(WiFi.subnetMask());
Serial.print("Gateway IP: ");
Serial.println(WiFi.gatewayIP());
Serial.print("DNS: ");
Serial.println(WiFi.dnsIP());
*/
ais_wifi_state_machine_e ais_wifi::state=AIS_WIFI_OFF;

#define WIFI_STA_START 2
void ais_wifi::start(){
	if (mode_ap_u8)
	{
		WiFi.softAP(ap_ssid,ap_pwd);
		local_ip=WiFi.softAPIP();
		print_ipaddr("AP local IP: ",&local_ip);
		//HBL120622 client_ip=IPAddress(192,168,4,2);
		state=AIS_WIFI_ON;
	}
	else {
		uint8_t cpt_u8=0;
		WiFi.begin(sta_ssid,sta_pwd);
		while ((cpt_u8<WIFI_STA_START)&&(WiFi.status()!=WL_CONNECTED)){
		        delay(1000);
		        printf(".");
		        cpt_u8++;
		 }
		if (cpt_u8==WIFI_STA_START)
		{
			state=AIS_WIFI_HS;
			LOG_W("Wifi not connected");
		} else
		{
			state=AIS_WIFI_ON;
			local_ip=WiFi.localIP();
			print_ipaddr("STA local IP: ",&local_ip);
			/*
			 * Get gateway IP
			 */
			client_ip=WiFi.gatewayIP();
		}
	}
	//WiFi.setTxPower(WIFI_POWER_19_5dBm);
	LOG_W("getTxPower(%d)",WiFi.getTxPower());
	/*
	 * Broadcast IP
	 * Works Android
	 * 	STA+hotspot
	 * 	AP+hotspot
	 */
	client_ip[3]=255;
	print_ipaddr("AP client_ip: ",&client_ip);

	/*
	 * Start Udp
	 */
	Udp.begin(local_ip,udp_local_port);
}




/*
void ais_wifi::setup_ap(char* ssid_pc,char * pwd_pc) {
  LOG_D("");
  WiFi.softAP(ssid_pc,pwd_pc);
  WiFi.softAPConfig(sta_local_ip,sta_gateway_ip,subnet_ip);
}
*/


void ais_wifi::stop()
{
	Udp.stop();
	if (mode_ap_u8)
	{
		WiFi.softAPdisconnect(true);
	} else
	{
		WiFi.disconnect(true);
	}
	state=AIS_WIFI_OFF;
}

void ais_wifi::on_off()
{
	stop();
	on_u8=(on_u8)?0:1;
	if (on_u8) {
		start();
	}
}

void ais_wifi::on_ap_sta()
{
	stop();
	mode_ap_u8=(mode_ap_u8)?0:1;
	if (on_u8) start();
}

/*
 * -------------------
 * wifi_udp_write
 * -------------------
 */
void ais_wifi::wifi_udp_write(char const * buffer, uint8_t len_u8) {
	/*
	 * client_ip TODO
	 *   mode sta:  client_ip = ip gateway
	 *   mode AP:
	 *     ip broadcast 255 ?
	 *     get all client ip
	 */

	Udp.beginPacket(client_ip,udp_client_port);
	Udp.write((uint8_t*)buffer,len_u8);
	Udp.endPacket();
}

/*
 * -------------------
 * wifi_udp_read
 * -------------------
 */
bool ais_wifi::wifi_udp_read(char *buffer, uint8_t len_u8) {
	bool data_ok=false;
	int16_t packetSize_i16=Udp.parsePacket();
	if (packetSize_i16){
		LOG_V("packetSize_i16(%d)",packetSize_i16);
		memset((void*)buffer,0,len_u8);
		int16_t l_i16=Udp.read((uint8_t*)buffer,len_u8);
		if (l_i16>0){
			buffer[l_i16]=0;
			data_ok=true;
		}
	}
	return data_ok;
}

/*
 * -------------------
 * wifi_printf
 * -------------------
 */
char Wifi_udp_write_char_a[WIFI_UDP_STR_SZ];
char Wifi_udp_read_char_a[WIFI_UDP_STR_SZ];
void ais_wifi::wifi_printf(const char *fmt, ...) {
	va_list ap;
	va_start(ap,fmt);
	memset((void*)Wifi_udp_write_char_a,0,WIFI_UDP_STR_SZ);
	vsnprintf(Wifi_udp_write_char_a,WIFI_UDP_STR_SZ,fmt,ap);
	if (state==AIS_WIFI_ON){
		wifi_udp_write(Wifi_udp_write_char_a,WIFI_UDP_STR_SZ);
	}
	printf("%s",Wifi_udp_write_char_a);
}



/*
 * -------------------
 * Serial
 * -------------------
 *
 * -------------------
 */
ais_serial::ais_serial(){
	serial_count=0;
	commands_in_queue=0;
	cmd_queue_index_r=0;
	cmd_queue_index_w=0;
	clear_command_queue();
}


/*
 * -------------------
 * start
 * -------------------
 * GPS
 *  +3V3
 *  +9600 bds
 *  +Periode 1s
 * -------------------
 */
void ais_serial::start(){
	//Start by default at 9600bds
	SERIAL_GPS.begin(Monitoring.settings_s.gps_bauds_u32,SERIAL_8N1,GPIO_NUM_13,GPIO_NUM_14);
	//SERIAL_GPS.begin(GPS_DEFAULT_BAUDRATE,SERIAL_8N1,GPIO_NUM_13,GPIO_NUM_14);
	delay(100);
	serial_count=0;
	commands_in_queue=0; // Count of commands in the queue
	cmd_queue_index_r=0; // Ring buffer read position
	cmd_queue_index_w=0; // Ring buffer write position
}



/*
 * -------------------
 * get_serial_commands
 * -------------------
 */
void ais_serial::get_available_commands()
{
	/**
	 * Loop while serial characters are incoming and the queue is not full
	 */
	while ((commands_in_queue<GPS_LOGGER_BUFSIZE)&&(SERIAL_GPS.available()>0))
	{
		char serial_char=SERIAL_GPS.read();
		/**
		 * If the character ends the line \r\n or \0
		 */
		if ((serial_char=='\0')||(serial_char=='\r')||(serial_char=='\n')||(serial_count==(GPS_MAX_CMD_SIZE-1)))
		{
			if (serial_count==0) continue;
			serial_line_buffer[serial_count]='\0';
			serial_count=0; //reset buffer
			//
			/*
			 * Add the command to the queue.
			 * Skip messages
			 * 	  - GSV,GSA,GST,MSS,PTNL*,ROT
			 * Keep messages
			 * 	  - GGA,GNS,HDT,GLL,GRS,LLQ,RMC,VTG,ZDA
			 */
			//if (commands_in_queue<GPS_LOGGER_BUFSIZE)
			if ((commands_in_queue<GPS_LOGGER_BUFSIZE)
					&&(memcmp(serial_line_buffer+3, "GSV,", 4)!=0)
					&&(memcmp(serial_line_buffer+3, "GSA,", 4)!=0)
					&&(memcmp(serial_line_buffer+3, "GST,", 4)!=0)
					&&(memcmp(serial_line_buffer+3, "MSS,", 4)!=0)
					&&(memcmp(serial_line_buffer+3, "ROT,", 4)!=0)
					)
			{
				memset((void*)command_queue[cmd_queue_index_w],0,GPS_MAX_CMD_SIZE);
				memcpy((void*)command_queue[cmd_queue_index_w],(void*)serial_line_buffer,GPS_MAX_CMD_SIZE);
				memset((void*)serial_line_buffer,0,GPS_MAX_CMD_SIZE);
				_commit_command();
			}
		} else
		{ // it's not a newline, carriage return or escape char
			serial_line_buffer[serial_count++]=serial_char;
		}
	} // queue has space, serial has data
}


void ais_serial::clear_command_queue()
{
	cmd_queue_index_r=0;
	cmd_queue_index_w=0;
	commands_in_queue=0;
	memset((void*)serial_line_buffer,0,GPS_MAX_CMD_SIZE);
	// HBL 080822
	memset((void*)command_queue,0,GPS_LOGGER_BUFSIZE*GPS_MAX_CMD_SIZE);
}

/*
 * _commit_command
 */
void ais_serial::_commit_command() {
	if (++cmd_queue_index_w >= GPS_LOGGER_BUFSIZE) cmd_queue_index_w=0;
	commands_in_queue++;
}

/*
 * process_next_command
 */
void ais_serial::process_next_command(){}

/*
 * loop
 */
void ais_serial::loop() {
	if (commands_in_queue<GPS_LOGGER_BUFSIZE) get_available_commands();
	while (commands_in_queue>0)
	{
		process_next_command();
		// The queue may be reset by a command handler or by code invoked by idle() within a handler
		if (commands_in_queue)
		{
			--commands_in_queue;
			if ((++cmd_queue_index_r)>=GPS_LOGGER_BUFSIZE) cmd_queue_index_r=0;
		}
	}
}

char ais_serial::command_queue[GPS_LOGGER_BUFSIZE][GPS_MAX_CMD_SIZE];
uint8_t ais_serial::commands_in_queue; // Count of commands in the queue
uint8_t ais_serial::cmd_queue_index_r; // Ring buffer read position
uint8_t ais_serial::cmd_queue_index_w; // Ring buffer write position


/*
 * -----------------------
 * NMEA GPS
 * -----------------------
 */

void Ais_nmea_gps::reset()
{
	memset((void*)&sensor_gps,0,sizeof(sensor_gps));
	memset((void*)&satellite_info,0,sizeof(satellite_info));
}

void Ais_nmea_gps::set_baudrate()
{
	uint32_t gps_bauds_u32=Monitoring.settings_s.gps_bauds_u32;
	//uint32_t gps_bauds_u32=0;
	GPSHelper::GPSConfig config={GPSHelper::OutputMode::GPS,GPSHelper::GNSSSystemsMask::RECEIVER_DEFAULTS};
	neo.configure(gps_bauds_u32,config);
	neo.reset(GPSRestartType::None);
}


//UBXMode mode = UBXMode::Normal;
Ais_nmea_gps::Ais_nmea_gps() :
		parser(GPSDriverNMEA(callback,(void *)0,&sensor_gps,&satellite_info,(float)0))
		,neo(GPSDriverUBX(GPSHelper::Interface::UART,callback,(void *)0,&sensor_gps,&satellite_info,GPS_NEO_MODEL,0.f,GPS_DEFAULT_BAUDRATE,GPSDriverUBX::UBXMode::Normal))
{
	reset();
	memset((void*)&Gps_info_s,0,sizeof(Gps_info_s));
}

Ais_nmea_gps::~Ais_nmea_gps()
{
}



/*
 * --------------------------
 * callback
 * --------------------------
 *   type: readDeviceData
 *   data1: pointer to dest buf
 *   data2: size of data1 buf
 *
 */

int Ais_nmea_gps::callback(GPSCallbackType type, void *buf, int buf_length, void *user)
{
	switch (type) {
	case GPSCallbackType::readDeviceData: {
		//memset((void*)buf,0,GPS_MAX_CMD_SIZE);
		//memcpy(buf,(void*)command_queue[cmd_queue_index_r],buf_length);
		//buf=command_queue[cmd_queue_index_r];
		//return buf_length;
		uint16_t idx_u16=0;
		uint8_t *data_pu8=(uint8_t*)buf;
		memset((void*)buf,0,GPS_MAX_CMD_SIZE);
		while ((idx_u16<buf_length)&&(SERIAL_GPS.available()>0))
		{
			data_pu8[idx_u16]=(uint8_t)SERIAL_GPS.read();
			idx_u16++;
		}
		return strlen((char*)buf);
	}

	case GPSCallbackType::writeDeviceData:
		return SERIAL_GPS.write((uint8_t*)buf,buf_length);
		break;

	case GPSCallbackType::setBaudrate:
		Monitoring.settings_s.gps_bauds_u32=buf_length;
		SERIAL_GPS.updateBaudRate(Monitoring.settings_s.gps_bauds_u32);
		break;

	case GPSCallbackType::gotRTCMMessage:
		break;

	case GPSCallbackType::surveyInStatus:
		break;

	case GPSCallbackType::setClock:
		break;
	default:
		break;
	}

	return 0;
}

void Ais_nmea_gps::update_gps_info(gps_data_t * gps_info_ps)
{
	float heading_f32;
	gps_info_ps->fix=sensor_gps.fix_type;
	gps_info_ps->lat_d=sensor_gps.lat;
	gps_info_ps->lon_d=sensor_gps.lon;
	gps_info_ps->speed_kt=sensor_gps.vel_m_s*(float)3.6/(float)KM_PER_MILE;
	gps_info_ps->elv=(float)sensor_gps.alt/(float)1000.0;
	//HBL260623 heading_f32=((sensor_gps.cog_rad<0)?sensor_gps.cog_rad+(float)PI:sensor_gps.cog_rad)*(float)180.0/(float)PI+(float)0.5;
	heading_f32=((sensor_gps.cog_rad<0)?sensor_gps.cog_rad+2*(float)PI:sensor_gps.cog_rad)*(float)180.0/(float)PI+(float)0.5;
	gps_info_ps->heading_d=heading_f32;
	memcpy((void*)&(gps_info_ps->utc_s),(void*)&(sensor_gps.utc_s),sizeof(timeinfo_t));
	//ais_wifi::wifi_printf("Lat_d(%.2f) - Lon_d(%.2f) - Fix(%d)\n",(float)Gps_info_s.lat_d/LAT_LONG_SCALE,(float)Gps_info_s.lon_d/LAT_LONG_SCALE,Gps_info_s.fix);
	//ais_wifi::wifi_printf("speed_kt(%.1f) - heading_d(%.0f)\n",Gps_info_s.speed_kt,Gps_info_s.heading_d);
	//ais_wifi::wifi_printf("Year(%d) - Day(%d) - Hour(%d:%d:%d)\n",Gps_info_s.utc_s.tm_year,Gps_info_s.utc_s.tm_mday,Gps_info_s.utc_s.tm_hour,Gps_info_s.utc_s.tm_min,Gps_info_s.utc_s.tm_sec);
}

void Ais_nmea_gps::copy2buf(char *data, uint16_t len_u16)
{
	memset((void*)command_queue[cmd_queue_index_r],0,GPS_MAX_CMD_SIZE);
	memcpy((void*)command_queue[cmd_queue_index_r],data,len_u16);
}

/*
 * process_next_command
 * current_command is a GPS NMEA line
 * terminated with a \0
 */
void Ais_nmea_gps::process_next_command()
{
	char *current_command;

	current_command=command_queue[cmd_queue_index_r];
	//LOG_D("%s",current_command);

	/*
	 * Forward current_command over Wifi
	 */
	if ((ais_wifi::state==AIS_WIFI_ON)&&Ais_nmea_gps_s.gps_nmea_on_u8){
		ais_wifi::wifi_udp_write(current_command,(int)strlen(current_command));
	}

	/*
	 * Parse NMEA current_command
	 * retrieve by read()
	 */
	if (parser.receive(0)>0) {
		update_gps_info(&Gps_info_s);
		//LOG_D("Lat: %.2f deg, Lon: %.2f deg, Fix: %d\n",(float)Gps_info_s.lat_d/(float)10000000.0,(float)Gps_info_s.lon_d/(float)10000000.0,Gps_info_s.fix);
		ais_wifi::wifi_printf("Lat: %.2f deg, Lon: %.2f deg, Fix: %d\n",(float)Gps_info_s.lat_d/(float)10000000.0,(float)Gps_info_s.lon_d/(float)10000000.0,Gps_info_s.fix);
	}

	// HBL 10/08/22
	memset((void*)current_command,0,GPS_MAX_CMD_SIZE);
}

void Ais_nmea_gps::parse()
{
	if (parser.receive(GPS_SERIAL_TIMEOUT_MS)>0) {
		update_gps_info(&Gps_info_s);
		//LOG_D("Lat: %.2f deg, Lon: %.2f deg, Fix: %d\n",(float)Gps_info_s.lat_d/(float)10000000.0,(float)Gps_info_s.lon_d/(float)10000000.0,Gps_info_s.fix);
		ais_wifi::wifi_printf("Lat: %.2f deg, Lon: %.2f deg, Fix: %d\n",(float)Gps_info_s.lat_d/(float)10000000.0,(float)Gps_info_s.lon_d/(float)10000000.0,Gps_info_s.fix);
	}
}


/*
 * External
 */

Ais_nmea_gps Ais_nmea_gps_s;
