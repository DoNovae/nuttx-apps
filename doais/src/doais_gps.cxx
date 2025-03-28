/**
 * =====================================
 *  doais_gps.cxx
 * -------------------------------------
 *  AIS DoNovae
 *  www.DoNovae.com
 *  Herve Bailly
 * =====================================
 *
 * =====================================
 */

#include <sys/ioctl.h>
#include <stdio.h>
#include <nuttx/config.h>
#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include <nuttx/config.h>
#include <sys/stat.h>
#include <nuttx/arch.h>
#include <pthread.h>
#include <semaphore.h>
#include <nuttx/timers/timer.h>

#include "doais_gps.h"
#include "ais_monitoring.h"
#include "ais_utils.h"
#include "types.h"

#include "types.h"
#include "gps.h"
#include "gmath.h"
#include "sensor_gps.h"
#include "nmea.h"
#include "ubx.h"

/*
 * --------------------------
 * Defines
 * --------------------------
 */
#define GPS_INTERVAL_US (1000*10)
#define GPS_DEFAULT_BAUDRATE 9600
#define GPS_NEO_MODEL 6


/*
 * --------------------------
 * Globals
 * --------------------------
 */
int GPS_TTYS_FD=0; // TTYS device ID
sensor_gps_s Sensor_gps; // GPS infos
satellite_info_s Satellite_info; // Satellite info

int callback(GPSCallbackType type, void *buf, int buf_length, void *user);
GPSDriverNMEA parser(GPSDriverNMEA(callback,(void *)0,&Sensor_gps,&Satellite_info,(float)0));

/*
 * --------------------------
 * Prototypes
 * --------------------------
 */
void update_gps_info(gps_data_t * gps_info_ps);


/*
 * --------------------------
 * Externs
 * --------------------------
 */
extern gps_data_t Gps_info_s; // Cf doais_db_update.c
extern FAR mutex_t Gps_data_mutex_s;// Cf ais_main.c


/*
 * ==========================
 * Functions
 * --------------------------
 */

/*
 * ==========================
 * GPS NMEA
 * --------------------------
 */

/*
 * --------------------------
 * callback
 * --------------------------
 *   type: readDeviceData
 *   data1: pointer to dest buf
 *   data2: size of data1 buf
 * --------------------------
 */

int callback(GPSCallbackType type, void *buf, int buf_length, void *user)
{
	switch (type) {
	case GPSCallbackType::readDeviceData:
	{
		uint16_t chid_u16=0;
		uint8_t byte_u8,*p_u8;
		p_u8=(uint8_t*)buf;

		while ((chid_u16<buf_length)&&(read(GPS_TTYS_FD,&byte_u8,1)>0))
		{
			p_u8[chid_u16]=byte_u8;
			chid_u16++;
		}
		return chid_u16;
	}

	case GPSCallbackType::writeDeviceData:
		return write(GPS_TTYS_FD,(uint8_t*)buf,buf_length);
		break;

	case GPSCallbackType::setBaudrate:
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

/*
 * --------------------------
 * update_gps_info
 * --------------------------
 * Data
 * 	 elv: Antenna altitude above/below mean sea level (geoid) in meters in [-100,100]
 *   speed_kt: Speed over the ground in knots
 *   heading_d: Track angle in degrees True in [0,360]
 *   fix: Fix quality 1 means just a normal 3D fix. 3 for auto, 4 for DGPS, 5 for floats, 6 for fixed. cf GGA
 *   lat_d: Latitude in DEG * 10000000.0
 *   lon_d: Longitude in DEG * 10000000.0
 * Update
 * 	 always gps_info_ps->fix
 *   lat_d, lon_d, speed_kt, heading_d, elv only if fix > 0
 *   Calculate speed between 2 positions only if fix >1 in last and current data, and no speed available.
 *
 */
void update_gps_info()
{
	float heading_f32;

	/*
	 * Update gps_info_ps if fix > 0
	 */
	if (Sensor_gps.fix_type)
	{
		/*
		 * Calculate default speed if both current and last fix suuficient
		 */
		if ((Sensor_gps.vel_m_s==0.0)&&(Gps_info_s.fix>0))
		{
			float dist_nm_f, duration_h_f;

			dist_nm_f=(float)Utils::distance_nm((float)Gps_info_s.lat_d/LAT_LONG_SCALE,(float)Gps_info_s.lon_d/LAT_LONG_SCALE,Sensor_gps.lat/LAT_LONG_SCALE,Sensor_gps.lon/LAT_LONG_SCALE);
			duration_h_f=(float)Gps_info_s.utc_s.tm_hour+(float)Gps_info_s.utc_s.tm_min/(float)60.0+(float)Gps_info_s.utc_s.tm_sec/(float)3600.0;

			LOG_D("update_gps_info: dist_nm_f(%.2f)/duration_h_f(%.2f)",dist_nm_f,duration_h_f);
			Gps_info_s.speed_kt=(duration_h_f>0.01)? dist_nm_f/duration_h_f:0.0;
		}
		memcpy((void*)&(Gps_info_s.utc_s),(void*)&(Sensor_gps.utc_s),sizeof(timeinfo_t));
		LOG_D("update_gps_info: tm_hour(%d) - tm_min(%d) - tm_sec(%d)",Sensor_gps.utc_s.tm_hour,Sensor_gps.utc_s.tm_min,Sensor_gps.utc_s.tm_sec);
		Gps_info_s.lat_d=Sensor_gps.lat;
		Gps_info_s.lon_d=Sensor_gps.lon;
		Gps_info_s.speed_kt=Sensor_gps.vel_m_s*(float)3.6/(float)KM_PER_MILE;
		Gps_info_s.elv=(float)Sensor_gps.alt/(float)1000.0;

		// Calculate heading
		heading_f32=((Sensor_gps.cog_rad<0)?Sensor_gps.cog_rad+2*(float)M_PI_F:Sensor_gps.cog_rad)*(float)180.0/(float)M_PI_F+(float)0.5;
		Gps_info_s.heading_d=heading_f32;
	}
	Gps_info_s.fix=Sensor_gps.fix_type;

	//ais_wifi::wifi_printf("Lat_d(%.2f) - Lon_d(%.2f) - Fix(%d)\n",(float)Gps_info_s.lat_d/LAT_LONG_SCALE,(float)Gps_info_s.lon_d/LAT_LONG_SCALE,Gps_info_s.fix);
	//ais_wifi::wifi_printf("speed_kt(%.1f) - heading_d(%.0f)\n",Gps_info_s.speed_kt,Gps_info_s.heading_d);
	//ais_wifi::wifi_printf("Year(%d) - Day(%d) - Hour(%d:%d:%d)\n",Gps_info_s.utc_s.tm_year,Gps_info_s.utc_s.tm_mday,Gps_info_s.utc_s.tm_hour,Gps_info_s.utc_s.tm_min,Gps_info_s.utc_s.tm_sec);
}





/*
 * ==========================
 * Thread
 * --------------------------
 */

/*
 * --------------------------
 * setBaudrate
 * --------------------------
 */
int setBaudrate(int _serial_fd, unsigned baud)
{
	/* process baud rate */
	int speed;
	FAR struct termios uart_config;
	int termios_state;

	LOG_D("setBaudrate: baud(%d)\n",baud);
	switch (baud) {
	case 9600:   speed = B9600;   break;
	case 19200:  speed = B19200;  break;
	case 38400:  speed = B38400;  break;
	case 57600:  speed = B57600;  break;
	case 115200: speed = B115200; break;
	case 230400: speed = B230400; break;
	default:
		LOG_E("setBaudrate: ERR unknown baudrate: %d\n", baud);
		return -EINVAL;
	}

	/* fill the struct for the new configuration */
	tcgetattr(_serial_fd,&uart_config);


	//	/* properly configure the terminal (see also https://en.wikibooks.org/wiki/Serial_Programming/termios ) */
	//
	//	//
	//	// Input flags - Turn off input processing
	//	//
	//	// convert break to null byte, no CR to NL translation,
	//	// no NL to CR translation, don't mark parity errors or breaks
	//	// no input parity check, don't strip high bit off,
	//	// no XON/XOFF software flow control
	//	//
	//	uart_config.c_iflag &= ~(IGNBRK|BRKINT|ICRNL|INLCR|PARMRK|INPCK|ISTRIP|IXON);
	//
	//	//
	//	// Output flags - Turn off output processing
	//	//
	//	// no CR to NL translation, no NL to CR-NL translation,
	//	// no NL to CR translation, no column 0 CR suppression,
	//	// no Ctrl-D suppression, no fill characters, no case mapping,
	//	// no local output processing
	//	//
	//	// config.c_oflag &= ~(OCRNL | ONLCR | ONLRET |
	//	//                     ONOCR | ONOEOT| OFILL | OLCUC | OPOST);
	//	uart_config.c_oflag=0;
	//
	//	//
	//	// No line processing
	//	//
	//	// echo off, echo newline off, canonical mode off,
	//	// extended input processing off, signal chars off
	//	//
	//	uart_config.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
	//
	//	/* no parity, one stop bit, disable flow control */
	//	uart_config.c_cflag &= ~(CSTOPB | PARENB | CRTSCTS);

	//	/* set baud rate */
	//	LOG_D("setBaudrate: Set %d (cfsetispeed)\n",speed);
	//	if ((termios_state=cfsetispeed(&uart_config,speed))<0) {
	//		LOG_E("setBaudrate: ERR %d (cfsetispeed)\n", termios_state);
	//		return -1;
	//	}
	//
	//	LOG_D("Set: %d (cfsetospeed)\n",speed);
	//	if ((termios_state=cfsetospeed(&uart_config,speed))<0) {
	//		LOG_E("setBaudrate: ERR %d (cfsetospeed)",termios_state);
	//		return -1;
	//	}

	LOG_D("setBaudrate: set baud %d\n",baud);
	if ((termios_state=cfsetspeed(&uart_config,speed))<0) {
		LOG_E("setBaudrate: ERR %d (cfsetspeed)",termios_state);
		return -1;
	}

	// TCSANOW   0         /* Change attributes immediately */
	// TCSADRAIN 1         /* Change attributes when output has drained */
	// TCSAFLUSH 2         /* Change attributes when output has drained;
	//                             * also flush pending input */
	if ((termios_state=tcsetattr(_serial_fd,TCSANOW,&uart_config))<0) {
		LOG_E("setBaudrate: ERR %d (tcsetattr)",termios_state);
		return -1;
	}

	return 0;
}


/*
 * --------------------------
 * gps_thread
 * --------------------------
 */
FAR void *gps_thread(pthread_addr_t arg)
{
	LOG_D("Starting gps_thread\n");
	/*
	 * Open GPS serial
	 */
	GPS_TTYS_FD=open(GPS_SERIAL_DEVICE,O_RDWR);
	if (GPS_TTYS_FD<0) {
		LOG_E("gps_thread: Error UART1\n");
	}

	/*
	 * setBaudrate
	 */
	setBaudrate(GPS_TTYS_FD,Settings_s.gps_bauds_u32);

	/*
	 * Init Gps_info_ps
	 */
	memset((void*)&Gps_info_s,0,sizeof(Gps_info_s));

	while (1)
	{
		if (parser.receive(0)>0) {

			nxmutex_lock(&Gps_data_mutex_s);
			update_gps_info();
			nxmutex_unlock(&Gps_data_mutex_s);

			LOG_D("Lat: %.2f deg - Lon: %.2f deg - Spd: %.1f kt - Fix: %d\n",
					(float)Gps_info_s.lat_d/(float)10000000.0,(float)Gps_info_s.lon_d/(float)10000000.0,Gps_info_s.speed_kt,Gps_info_s.fix);
			//ais_wifi::wifi_printf("Lat: %.2f deg, Lon: %.2f deg, Fix: %d\n",(float)Gps_info_s.lat_d/(float)10000000.0,(float)Gps_info_s.lon_d/(float)10000000.0,Gps_info_s.fix);
		}
	}

	close(GPS_TTYS_FD);
	return 0;
}

