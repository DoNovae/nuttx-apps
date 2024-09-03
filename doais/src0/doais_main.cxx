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

extern "C" {
#include "ui_clock.h"
}

#include <sys/ioctl.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <termios.h>

#include "../inc0/doais_mng.h"

/*
 * Defines
 */
#define ACCEL_TASK_INTERVAL_MS 1000

/*
 * Globals
 */
static pid_t serial_pid;
static pid_t gps_pid;
static pid_t display_pid;
static pid_t publisher_pid;
static pid_t subscriber_pid;


/*
#define ORB_DEFINE(name, structure, cb) \
  const struct orb_metadata g_orb_##name = \
  { \
    #name, \
    sizeof(structure), \
  };
#endif
 */
ORB_DEFINE(orb_test1,struct orb_test1_s,0);

/*
 * Prototypes
 */
int setBaudrate(int _serial_fd, unsigned baud);
static int serial_task(int argc, FAR char *argv[]);
static int display_init(void);
static int display_task(int argc, FAR char *argv[]);
static int gps_task(int argc, FAR char *argv[]);

static int publisher_task(int argc, char *argv[]);
static int subscriber_task(int argc, FAR char *argv[]);

static int mng_publisher_task(int argc, char *argv[]);
static int mng_subscriber_task(int argc, FAR char *argv[]);

static int mng_dev_publisher_task(int argc, char *argv[]);
static int mng_dev_subscriber_task(int argc, FAR char *argv[]);


/*
 * Main
 */
extern "C"
{
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

	gps_pid = task_create(
			"GPS",
			120,
			7000,
			gps_task,
			(char* const*)child_argv);
	if (serial_pid < 0) {
		printf("Failed to create GPS task\n");
	}

	/*
	 * Display Task
	 */
	display_init();
	display_pid = task_create(
			"Display",
			120,
			7000,
			display_task,
			(char* const*)child_argv);
	if (display_pid < 0) {
		printf("Failed to create DISPLAY task\n");
	}

	publisher_pid = task_create(
			"Publisher",
			120,
			7000,
			publisher_task,
			(char* const*)child_argv);
	if (publisher_pid < 0) {
		printf("Failed to create Publisher task\n");
	}

	subscriber_pid = task_create(
			"Subscriber",
			120,
			7000,
			subscriber_task,
			(char* const*)child_argv);
	if (subscriber_pid < 0) {
		printf("Failed to create Subscriber task\n");
	}

#ifdef CONFIG_NSH_CONSOLE
	ret = nsh_consolemain(argc, argv);
#endif

	return ret;
}
}


/*
 * setBaudrate
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





/*
 * serial_task
 */
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


/*
 * display_init
 */
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

/*
 * display_task
 */
static int display_task(int argc, FAR char *argv[])
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


/*
 * gps_task
 */
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
 * uORB task
 */
static void print_mng_msg(FAR const struct orb_metadata *meta,FAR const void *buffer)
{
	FAR const struct mng_msg_s *message = (const struct mng_msg_s*)buffer;
	const orb_abstime now = orb_absolute_time();

	uorbinfo_raw("%s:\ttimestamp: %"PRIu64" (%"PRIu64" us ago) val: %s",
			meta->o_name, message->timestamp, now - message->timestamp,
			message->cmd_cha);
}



/*
 * publisher_tasks
 */
static int publisher_task0(int argc, FAR char *argv[])
{
	struct orb_test1_s sample;
	int instance = 0;
	int afd;
	int ret;

	// advertise
	sample.val = 0;
	afd = orb_advertise_multi_queue_persist(ORB_ID(orb_test1),&sample, &instance, 1);
	if (afd < 0)
	{
		printf("publisher_task: advertise failed: %d", errno);
		return ERROR;
	}

	while(1)
	{
		usleep(2000*1000);
		sample.val++;
		// Publish
		if (OK != orb_publish(ORB_ID(orb_test1), afd, &sample))
		{
			return printf("publisher_task: publish failed\n");
		}
	}
	// unadvertise
	ret = orb_unadvertise(afd);
	if (ret != OK)
	{
		return printf("publisher_task: orb_unadvertise failed: %i", ret);
	}
	return 0;
}


/*
 * publisher_task
 */
static int publisher_task(int argc, char *argv[])
{
	const int queue_size = 10;
	struct orb_test1_s sample;
	int instance = 0;
	int ptopic;

	// Reset
	memset(&sample, '\0', sizeof(sample));


	/****************************************************************************
	 * Name: orb_advertise_multi_queue
	 *
	 * Description:
	 *   This performs the initial advertisement of a topic; it creates the topic
	 *   node in /dev/uorb and publishes the initial data.
	 *
	 * Input Parameters:
	 *   meta         The uORB metadata (usually from the ORB_ID() macro)
	 *   data         A pointer to the initial data to be published.
	 *   instance     Pointer to an integer which yield the instance ID,
	 *                (has default 0 if pointer is NULL).
	 *   queue_size   Maximum number of buffered elements.
	 *
	 * Returned Value:
	 *   -1 on error, otherwise returns an file descriptor
	 *   that can be used to publish to the topic.
	 *   If the topic in question is not known (due to an
	 *   ORB_DEFINE with no corresponding ORB_DECLARE)
	 *   this function will return -1 and set errno to ENOENT.
	 ****************************************************************************/
	//#define ORB_ID(name)  &g_orb_##name

	ptopic = orb_advertise_multi_queue(ORB_ID(orb_test1), &sample, &instance, queue_size);
	if (ptopic < 0)
	{
		printf("publisher_task: advertise failed: %d", errno);
	}

	while(1)
	{
		// Publish
		orb_publish(ORB_ID(orb_test1), ptopic, &sample);
		sample.val++;
		usleep(200 * 1000);
	}

	orb_unadvertise(ptopic);

	return 0;
}


/*
 * subscriber_task
 */
static int subscriber_task(int argc, FAR char *argv[])
{
	struct pollfd fds[1];
	struct orb_test1_s sample;
	bool updated;
	int sfd;
	int ret;

	// Subscribe
	if ((sfd = orb_subscribe(ORB_ID(orb_test1))) < 0)
	{
		return printf("subscriber_task: subscribe failed: %d\n", errno);
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
			orb_copy(ORB_ID(orb_test1), sfd, &sample);
		}
	}
	while (updated);

	fds[0].fd     = sfd;
	fds[0].events = POLLIN;

	while(1){
		int poll_ret;
		int nb_objects=1;

		// Timeout 2s
		poll_ret = poll(fds, nb_objects,2000*1000);
		if (poll_ret == 0){
			printf("subscriber_task: poll timeout\n");
		}

		if (OK != orb_check(sfd, &updated))
		{
			return printf("subscriber_task: check failed\n");
		}

		else if (poll_ret < 0 && errno != EINTR)
		{
			printf("subscriber_task: poll error (%d, %d)\n", poll_ret, errno);
		}

		if (fds[0].revents & POLLIN)
		{
			orb_copy(ORB_ID(orb_test1), sfd, &sample);

			printf("subscriber_task: sub_sample.val(%d)\n",sample.val);
		}
		usleep(250 * 1000);
	}

	// unsubscribe
	ret = orb_unsubscribe(sfd);
	if (ret != OK)
	{
		return printf("subscriber_task: orb_unsubscribe failed: %i", ret);
	}
	return 0;
}


/*
 * mng_publisher_task
 */
static int mng_publisher_task(int argc, char *argv[])
{
	const int queue_size = 50;
	struct mng_msg_s sample;
	int instance = 0;
	int ptopic;
	uint16_t cpt_u16=0;

	// Reset
	memset(&sample, '\0', sizeof(sample));

	// Advertise
	ptopic = orb_advertise_multi_queue_persist(ORB_ID(mng_msg), &sample, &instance, queue_size);
	if (ptopic < 0)
	{
		printf("mng_publisher_task: advertise failed: %d", errno);
		return 0;
	}

	while(1)
	{
		cpt_u16++;
		memset(sample.cmd_cha,0,MNG_CMD_SIZE);
		snprintf(sample.cmd_cha,MNG_CMD_SIZE,"msg(%d)",cpt_u16);
		// Publish
		orb_publish(ORB_ID(mng_msg), ptopic, &sample);
		usleep(2000 * 1000);
	}

	orb_unadvertise(ptopic);

	return 0;
}


/*
 * mng_subscriber_task
 */
static int mng_subscriber_task(int argc, FAR char *argv[])
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
		return 0;
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

		// Timeout 1000ms
		poll_ret = poll(fds, 1,1000*1000);
		if (poll_ret == 0){
			printf("mng_subscriber_task: poll timeout\n");
		}

		if (OK != orb_check(sfd, &updated))
		{
			return printf("mng_subscriber_task: check failed\n");
		}
		else if (poll_ret < 0 && errno != EINTR)
		{
			printf("mng_subscriber_task: poll error (%d, %d)\n", poll_ret, errno);
		}

		if (fds[0].revents & POLLIN)
		{
			orb_copy(ORB_ID(mng_msg),sfd,&sample);

			printf("mng_subscriber_task: %s\n",sample.cmd_cha);
		}
	}

	// unsubscribe
	ret = orb_unsubscribe(sfd);
	if (ret != OK)
	{
		return printf("mng_subscriber_task: orb_unsubscribe failed: %i", ret);
	}
	return 0;
}




/*
 * /dev/uorb/mng_msg0
 */
#define MNG_UORB_DEV_PATH "/dev/uorb/mng_msg0"

static int mng_dev_publisher_task(int argc, char *argv[])
{
	struct mng_msg_s sample;
	const int queue_size = 50;
	int instance = 0;
	int ptopic;
	uint16_t cpt_u16=0;
	int sfd;

	// Subscribe
	sfd=-1;
	while (sfd < 0)
	{
		sfd = orb_open("mng_msg",0,O_WRONLY);
		printf("mng_dev_publisher_task: subscribe failed: %d\n",errno);
		usleep(1000 * 1000);
	}

	// Reset
	memset(&sample, '\0', sizeof(sample));

	while(1)
	{
		cpt_u16++;
		memset(sample.cmd_cha,0,MNG_CMD_SIZE);
		snprintf(sample.cmd_cha,MNG_CMD_SIZE,"msg(%d)\0",cpt_u16);
		// Publish
		orb_publish(ORB_ID(mng_msg),sfd,&sample);
		printf("mng_dev_publisher_task: %s\n",sample.cmd_cha);
		usleep(2000 * 1000);
	}
	close(sfd);

	return 0;
}



/*
 * mng_dev_subscriber_task
 */
static int mng_dev_subscriber_task(int argc, FAR char *argv[])
{
	struct pollfd fds[1];
	struct mng_msg_s sample;
	const int queue_size = 20;
	int instance = 0;
	bool updated;
	int sfd;
	int ret;

	// Advertise
	sfd = orb_advertise_multi_queue_persist(ORB_ID(mng_msg), &sample, &instance, queue_size);
	if (sfd < 0)
	{
		printf("mng_dev_publisher_task: advertise failed: %d", errno);
		return 0;
	}

	// Subscribe
	if ((sfd = orb_subscribe(ORB_ID(mng_msg))) < 0)
	{
		printf("mng_dev_subscriber_task: subscribe failed: %d\n", errno);
		return 0;
	}

	fds[0].fd     = sfd;
	fds[0].events = POLLIN;

	while(1){
		int poll_ret;

		// Timeout 500ms
		poll_ret = poll(fds, 1,1000);
		if (poll_ret == 0){
			//printf("mng_dev_subscriber_task: poll timeout\n");
		}

		if (OK != orb_check(sfd, &updated))
		{
			printf("mng_dev_subscriber_task: check failed\n");
			return 0;
		}
		else if (poll_ret < 0 && errno != EINTR)
		{
			printf("mng_dev_subscriber_task: poll error (%d, %d)\n", poll_ret, errno);
		}

		if (fds[0].revents & POLLIN)
		{
			orb_copy(ORB_ID(mng_msg),sfd,&sample);
			printf("mng_dev_subscriber_task: %s\n",sample.cmd_cha);
			/*
			 * TODO
			 * Parse cmd_cha :
			 * 	cf get_serial_commands in doais_serial.cpp
			 */
		}
	}

	// unsubscribe
	ret = orb_unsubscribe(sfd);
	if (ret != OK)
	{
		return printf("mng_dev_subscriber_task: orb_unsubscribe failed: %i", ret);
	}
	return 0;
}



