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
#include "types.h"



/*
 * --------------------------
 * Defines
 * --------------------------
 */
#define GPS_INTERVAL_US (1000*10)


/*
 * --------------------------
 * Globals
 * --------------------------
 */





/*
 * --------------------------
 * Prototypes
 * --------------------------
 */



/*
 * --------------------------
 * Externs
 * --------------------------
 */
extern gps_data_t Gps_info_s;


/*
 * --------------------------
 * Functions
 * --------------------------
 */

/*
 * --------------------------
 * setBaudrate
 *   - Not working !?
 *
 */
int setBaudrate(int _serial_fd, unsigned baud)
{
	/* process baud rate */
	int speed, rc;
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
 * gps_thread
 */
FAR void *gps_thread(pthread_addr_t arg)
{
	int fd;
	char buffer;
	char buffer_aux[256]={};
	int ret;
	int i = 0;

	LOG_D("Starting gps_thread\n");
	fd=open(GPS_SERIAL_DEVICE,O_RDWR);
	if (fd<0) {
		LOG_E("gps_thread: Error UART1\n");
	}

	/*
	 * setBaudrate
	 */
	setBaudrate(fd,Monitoring.settings_s.gps_bauds_u32);

	while (1)
	{
		ret = read(fd,&buffer,sizeof(buffer));
		if (ret > 0)
		{
			buffer_aux[i]=buffer;
			i++;

			if ((i==255)||(buffer == '\r')||(buffer == '\n'))
			{
				//printf("%s\n",buffer_aux);
				i=0;
				usleep(GPS_INTERVAL_US);
			}
		} else
		{
			usleep(GPS_INTERVAL_US);
		}
	}

	close(fd);
	return 0;
}

