/****************************************************************************
 * apps/examples/hello/hello_main.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <nuttx/mqueue.h>
#include <stdint.h>
#include <stdlib.h>

#include <errno.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/boardctl.h>
#include <sys/stat.h>

#include "nshlib/nshlib.h"

#include "ui_clock.h"

#include <sys/ioctl.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <termios.h>

#include <nuttx/config.h>

#include <errno.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/boardctl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "nshlib/nshlib.h"

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * hello_main
 ****************************************************************************/

/*int main(int argc, FAR char *argv[])
{
  printf("Hello, World!!\n");
  return 0;
}*/




/*
 * Printf test
 */

/*
#include <nuttx/config.h>
#include <time.h>

static double TimeSpecToSeconds(struct timespec* ts){
    return (double)ts -> tv_sec + (double)ts->tv_sec / 1000000000.0;
}
int doais_main(int argc, char *argv[])
{
    struct timespec start;
    struct timespec end;
    double elapsed_secs;

    int N = 1000;
    int i = 1;

    printf("Hello world! I am good at counting! Here I go... \n");
    clock_gettime(CLOCK_MONOTONIC, &start);

    //while (i <= N) {
    while (1) {
            printf("%d\n", i);
            i = i + 1;
            usleep(500000);

    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_secs = TimeSpecToSeconds(&end) - TimeSpecToSeconds(&start);
    printf("\n See... I counted &d numbers in %.3f seconds", N, elapsed_secs);

    return 0;
}
 */

/*
 * Serial test
 */

/*
int doais_main(int argc, char *argv[])
{
  int fd;
  char buffer;
  char buffer_aux[256] = {};
  int ret;
  int i = 0;

  fd = open("/dev/ttyS0", O_RDWR); //Opening the uart with read and write permission
  if (fd < 0) {
    printf("Error UART");
  }
  //This loop will check if there are any data available
  //That data it will be save in a buffer and when we detect a return
  //The data it will be send again
  while (1) {
    ret = read(fd, &buffer, sizeof(buffer));//It return only a char
    if (ret > 0) {
      buffer_aux[i] = buffer;//Saving in the auxiliary buffer
      i++;

      if (buffer == '\r') {//If the character if a return the data will be send
        ret = write(fd, buffer_aux, sizeof(char) * i);//You can send in this case up to 256 character
        if (ret > 0) {
          i = 0;
        }
      }
    }
  }
  return 0;
}
 */

int setBaudrate(int _serial_fd, unsigned baud)
 {
     /* process baud rate */
     int speed;
     struct termios uart_config;
     int termios_state;

     switch (baud) {
     case 9600:   speed = B9600;   break;

     case 19200:  speed = B19200;  break;

     case 38400:  speed = B38400;  break;

     case 57600:  speed = B57600;  break;

     case 115200: speed = B115200; break;

     case 230400: speed = B230400; break;

     default:
         printf("ERR: unknown baudrate: %d\n", baud);
         return -EINVAL;
     }

     /* fill the struct for the new configuration */
     tcgetattr(_serial_fd, &uart_config);

     /* properly configure the terminal (see also https://en.wikibooks.org/wiki/Serial_Programming/termios ) */

     //
     // Input flags - Turn off input processing
     //
     // convert break to null byte, no CR to NL translation,
     // no NL to CR translation, don't mark parity errors or breaks
     // no input parity check, don't strip high bit off,
     // no XON/XOFF software flow control
     //
     uart_config.c_iflag &= ~(IGNBRK | BRKINT | ICRNL |
                  INLCR | PARMRK | INPCK | ISTRIP | IXON);
     //
     // Output flags - Turn off output processing
     //
     // no CR to NL translation, no NL to CR-NL translation,
     // no NL to CR translation, no column 0 CR suppression,
     // no Ctrl-D suppression, no fill characters, no case mapping,
     // no local output processing
     //
     // config.c_oflag &= ~(OCRNL | ONLCR | ONLRET |
     //                     ONOCR | ONOEOT| OFILL | OLCUC | OPOST);
     uart_config.c_oflag = 0;

     //
     // No line processing
     //
     // echo off, echo newline off, canonical mode off,
     // extended input processing off, signal chars off
     //
     uart_config.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);

     /* no parity, one stop bit, disable flow control */
     uart_config.c_cflag &= ~(CSTOPB | PARENB | CRTSCTS);

     /* set baud rate */
     printf("Set: %d (cfsetispeed)\n", speed);
     if ((termios_state = cfsetispeed(&uart_config, speed)) < 0) {
         printf("ERR: %d (cfsetispeed)\n", termios_state);
         return -1;
     }

     printf("Set: %d (cfsetospeed)\n", speed);
     if ((termios_state = cfsetospeed(&uart_config, speed)) < 0) {
         printf("ERR: %d (cfsetospeed)", termios_state);
         return -1;
     }

     printf("Set: (tcsetattr)\n");
     if ((termios_state = tcsetattr(_serial_fd, TCSANOW, &uart_config)) < 0) {
         printf("ERR: %d (tcsetattr)", termios_state);
         return -1;
     }

     return 0;
 }







static int serial_task(int argc, FAR char *argv[])
{
	int fd;
	char buffer;
	char buffer_aux[256] = {};
	int ret;
	int i = 0;

	printf("Starting serial_task\n");

	fd = open("/dev/ttyS0", O_RDWR);
	if (fd < 0) {
		printf("Error UART");
	}

	while (1) {
		ret = read(fd, &buffer, sizeof(buffer));
		if (ret > 0) {
			buffer_aux[i] = buffer;
			i++;

			if (buffer == '\r') {
				ret = write(fd, buffer_aux, sizeof(char) * i);
				if (ret > 0) {
					i = 0;
				}
			}
		}
	}
	return 0;
}

static int display_init(void)
{
	/* LVGL initialization */
	lv_init();

	/* LVGL port initialization */
	lv_port_init();

	clock_display = lv_obj_create(NULL);  // Creates a Screen object

	lv_clock_display(clock_display);
	clock_update_cb();

	return 0;
}

static int display_task(void)
{
while (1)
	{
		lv_timer_handler();
		usleep(5*1000);
		lv_tick_inc(5);
		clock_update_cb();
	}
return 0;
}

static int gps_task(int argc, FAR char *argv[])
{
	int fd;
	char buffer;
	char buffer_aux[256] = {};
	int ret;
	int i = 0;

	printf("Starting gps_task\n");

	fd = open("/dev/ttyS1", O_RDWR);
	if (fd < 0) {
		printf("Error UART\n");
	}

	 setBaudrate(fd, 115200);

	while (1) {
		ret = read(fd, &buffer, sizeof(buffer));
		if (ret > 0) {
			buffer_aux[i] = buffer;
			i++;

			if ((i==255)||(buffer == '\r')) {
				printf("%s",buffer_aux);
				i=0;
			}
		}
	}
	return 0;
}

/*
 * Two tasks test
 */

#define ACCEL_TASK_INTERVAL_MS 1000
static pid_t serial_pid;
static pid_t display_pid;

/*
int main(int argc, FAR char *argv[]) {
	char *child_argv[2];

#ifdef ENTRY_POINT
  // Perform architecture-specific initialization
  boardctl(BOARDIOC_INIT, 0);
#endif //ENTRY_POINT

	printf("Starting DoAis\n");


	serial_pid = task_create(
			"Serial Task",
			120,
			7000,
			serial_task,
			(char* const*)child_argv);
	if (serial_pid < 0) {
		printf("Failed to create Serial task\n");
	}

	serial_pid = task_create(
			"GPS Task",
			120,
			7000,
			gps_task,
			(char* const*)child_argv);
	if (serial_pid < 0) {
		printf("Failed to create GPS task\n");
	}

	display_init();

	while (1)
	{
		lv_timer_handler();
		usleep(5*1000);
		lv_tick_inc(5);
		clock_update_cb();
	}

	return 0;
}
*/


int main(int argc, FAR char *argv[])
{
  struct sched_param param;
  int ret = 0;
  char *child_argv[2];

  /* Check the task priority that we were started with */

  sched_getparam(0, &param);
  if (param.sched_priority != CONFIG_SYSTEM_NSH_PRIORITY)
    {
      /* If not then set the priority to the configured priority */

      param.sched_priority = CONFIG_SYSTEM_NSH_PRIORITY;
      sched_setparam(0, &param);
    }

  /* Initialize the NSH library */
  nsh_initialize();

	serial_pid = task_create(
			"GPS Task",
			120,
			7000,
			gps_task,
			(char* const*)child_argv);
	if (serial_pid < 0) {
		printf("Failed to create GPS task\n");
	}

	display_init();

	display_pid = task_create(
			"Display Task",
			120,
			7000,
			display_task,
			(char* const*)child_argv);
	if (display_pid < 0) {
		printf("Failed to create DISPLAY task\n");
	}


#ifdef CONFIG_NSH_CONSOLE
  ret = nsh_consolemain(argc, argv);
#endif


  return ret;
}



