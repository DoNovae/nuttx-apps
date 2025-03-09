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
#include <sys/boardctl.h>
#include <sys/stat.h>

#include <nshlib/nshlib.h>
#include <errno.h>
#include <math.h>
#include <unistd.h>
#include <termios.h>

#include <nuttx/config.h>
#include <sys/stat.h>
#include <nuttx/arch.h>
#include <pthread.h>
#include <semaphore.h>
#include <nuttx/timers/timer.h>
#include <lvgl/lvgl.h>
#include <nuttx/input/touchscreen.h>



#if defined(CONFIG_FS_BINFS) && (CONFIG_BUILTIN)
#  include <nuttx/binfmt/builtin.h>
#endif

#if defined(CONFIG_LIBC_EXECFUNCS) && defined(CONFIG_EXECFUNCS_SYMTAB)
#  include <nuttx/binfmt/symtab.h>
#endif



#include "doais_mng.h"
#include "doais_serial.h"
#include "doais_db_update.h"
#include "doais_gps.h"
#include "ais_channels.h"
#include "ais_monitoring.h"

extern "C"
{
#include "display.h"
}

/*
 * ===================
 * Defines
 * -------------------
 */
// THREAD_STACK_MAX_SIZE (4096*16)
#define THREAD_STACK_SIZE 4096
#define THREAD_SERIAL_STACK_SIZE (4096*16)
#define THREAD_DB_UPDATE_STACK_SIZE (4096*16)
#define THREAD_GPS_STACK_SIZE (4096)
#define THREAD_PRIORITY ((sched_get_priority_max(SCHED_FIFO)+sched_get_priority_min(SCHED_FIFO))/2)
#define THREAD_DISPLAY_PRIORITY ((2*sched_get_priority_max(SCHED_FIFO)+sched_get_priority_min(SCHED_FIFO))/3)

#define ACCEL_TASK_INTERVAL_MS 1000
#define TASK_PRIORITY 120
#define TASK_STACK_SIZE 8192
#define TASK_SERIAL_STACK_SIZE 20000
#define USLEEP_50MS (50*1000)

#define TOUCHSCREEN_DEVPATH "/dev/input0"

#define DISPLAY_TIMER_MS 30


/* ==================
 * Globals
 * ------------------
 */
static pid_t gps_pid;
//static pid_t publisher_pid;
//static pid_t subscriber_pid;

/*
 * Cf lvgl/src/hal/lv_hal_indev.h
 */
static lv_indev_drv_t Touchscreen_drv_s;
static lv_indev_t * Touch_screen_s;
static lv_group_t * Grp_objects_s;

/*
 * Displays
 */
lv_updatable_display_t Displays_as[DISPLAY_NBR];
Display_id_e Display_id=DISPLAY_TARGET_ID;


/*
 * Mutex
 */
FAR mutex_t Gps_data_mutex_s;

#if defined(CONFIG_LIBC_EXECFUNCS) && defined(CONFIG_EXECFUNCS_SYMTAB)
const struct symtab_s CONFIG_EXECFUNCS_SYMTAB[1];
#endif




/*
 * ----------------
 * Prototypes
 * ----------------
 */
//static int gps_task(int argc, FAR char *argv[]);

static int display_init(void);
static FAR void *display_thread(pthread_addr_t arg);

static int publisher_task(int argc, char *argv[]);
static int subscriber_task(int argc, FAR char *argv[]);

static int mng_publisher_task(int argc, char *argv[]);
static int mng_subscriber_task(int argc, FAR char *argv[]);

static int mng_dev_publisher_task(int argc, char *argv[]);
static int mng_dev_subscriber_task(int argc, FAR char *argv[]);

void indev_click_cb(lv_indev_drv_t *indevDriver,uint8_t event_u8);
void indev_read_cb(lv_indev_drv_t *indevDriver,lv_indev_data_t *indevData);


/*
 * ==========================
 * Main
 * --------------------------
 *  Must be compiled in C for autostart
 *  NSH:
 *    - Without implies Custom board late initialization enable
 *  Ref:
 *    - https://nuttx.apache.org/docs/latest/applications/nsh/customizing.html
 *    - https://github.com/kaushalparikh/nuttx/blob/master/apps/examples/nsh/nsh_main.c
 *    - https://cwiki.apache.org/confluence/display/NUTTX/NuttX+Initialization+Sequence
 *    - https://github.com/kaushalparikh/nuttx/blob/master/apps/nshlib/nsh_consolemain.c
 *    - https://github.com/projectara/nuttx/blob/master/apps/nshlib/nsh_console.c
 *    - https://nuttx.apache.org/docs/10.0.0/components/nsh/installation.html
 * ==========================
 */
extern "C" {
int main(int argc, FAR char *argv[])
{
	//struct sched_param param;
	int ret = 0;

#ifdef CONFIG_NSH_CONSOLE
	/* Initialize the NSH library */
	nsh_initialize();
#endif // CONFIG_NSH_CONSOLE

	/*
	 * Mutex
	 */
	nxmutex_init(&Gps_data_mutex_s);

	/*
	 * Tasks
	 */
	//  char *child_argv[2];
	//	gps_pid = task_create("GPS",TASK_PRIORITY,TASK_STACK_SIZE,gps_task,(char* const*)child_argv);
	//	if (gps_pid < 0) {
	//		printf("Failed to create GPS task\n");
	//	}

	/*
	 * Display Task
	 */
	display_init();

	/*
	 * Display thread
	 */
	{
		pthread_t pid;
		pthread_attr_t tattr;
		struct sched_param sparam;
		pthread_attr_init(&tattr);
		sparam.sched_priority=THREAD_DISPLAY_PRIORITY;
		pthread_attr_setschedparam(&tattr, &sparam);
		pthread_attr_setstacksize(&tattr,THREAD_STACK_SIZE);
		pthread_create(&pid, &tattr,display_thread,(pthread_addr_t)0);
		pthread_setname_np(pid, "display_thread");
	}


	/*
	 * Serial Thread
	 */
	{
		pthread_t pid;
		pthread_attr_t tattr;
		struct sched_param sparam;
		pthread_attr_init(&tattr);
		sparam.sched_priority=THREAD_PRIORITY;
		pthread_attr_setschedparam(&tattr, &sparam);
		pthread_attr_setstacksize(&tattr,THREAD_SERIAL_STACK_SIZE);
		pthread_create(&pid,&tattr,serial_thread,(pthread_addr_t)0);
		pthread_setname_np(pid, "serial_thread");
	}

	/*
	 * Timer
	 */
	{
		pthread_t pid;
		pthread_attr_t tattr;
		struct sched_param sparam;

		pthread_attr_init(&tattr);
		sparam.sched_priority=THREAD_PRIORITY;
		pthread_attr_setschedparam(&tattr, &sparam);
		pthread_attr_setstacksize(&tattr,THREAD_STACK_SIZE);
		pthread_create(&pid,&tattr,timer_thread,(pthread_addr_t)0);
		pthread_setname_np(pid, "timer_thread");
	}

	/*
	 * DB update
	 */
	{
		pthread_t pid;
		pthread_attr_t tattr;
		struct sched_param sparam;

		pthread_attr_init(&tattr);
		sparam.sched_priority=THREAD_PRIORITY;
		pthread_attr_setschedparam(&tattr, &sparam);
		pthread_attr_setstacksize(&tattr,THREAD_DB_UPDATE_STACK_SIZE);
		pthread_create(&pid,&tattr,db_update_thread,(pthread_addr_t)0);
		pthread_setname_np(pid, "db_update_thread");
	}

	/*
	 * GPS
	 */
	{
		pthread_t pid;
		pthread_attr_t tattr;
		struct sched_param sparam;

		pthread_attr_init(&tattr);
		sparam.sched_priority=THREAD_PRIORITY;
		pthread_attr_setschedparam(&tattr,&sparam);
		pthread_attr_setstacksize(&tattr,THREAD_GPS_STACK_SIZE);
		pthread_create(&pid,&tattr,gps_thread,(pthread_addr_t)0);
		pthread_setname_np(pid,"gps_thread");
	}


#ifdef CONFIG_NSH_CONSOLE
	ret = nsh_consolemain(argc, argv);
#else
	while (1)
	{
		usleep(USLEEP_50MS);
	}
#endif // CONFIG_NSH_CONSOLE

	return ret;
}
}


/*
 * ----------------
 * gps_task
 */
//static int gps_task(int argc, FAR char *argv[])
//{
//	int fd;
//	char buffer;
//	char buffer_aux[256] = {};
//	int ret;
//	int i = 0;
//
//	printf("Starting gps_task\n");
//
//	fd = open("/dev/ttyS1",O_RDWR);
//	if (fd < 0) {
//		printf("Error UART\n");
//	}
//
//	setBaudrate(fd,230400);
//
//	while (1)
//	{
//		ret = read(fd, &buffer, sizeof(buffer));
//		if (ret > 0) {
//			buffer_aux[i] = buffer;
//			i++;
//
//			if ((i==255)||(buffer == '\r')) {
//				printf("%s",buffer_aux);
//				i=0;
//			}
//		} else {
//			usleep(USLEEP_50MS);
//		}
//	}
//	return 0;
//}



/*
 * ----------------
 * display_init
 * ----------------
 */
//lv_obj_t *Vessels_s,*Target_s, *Settings_s;
//lv_obj_t *Dspl_wind_s,*Clock_s,*Compass_s,*Autopilot_s;

void lv_display_init(Display_id_e id_e)
{
	lv_obj_clean(Displays_as[id_e].display_ps);
	lv_scr_load(Displays_as[id_e].display_ps);
	Displays_as[id_e].init_cb(Displays_as[id_e].display_ps);
}

static int display_init(void)
{
	/* LVGL initialization */
	lv_init();

	/* LVGL port initialization */
	lv_port_init();

	/**
	 * Initialize an input device driver with default values.
	 */
	lv_indev_drv_init(&Touchscreen_drv_s);

	/* src/hal/lv_hal_indev.h */
	Touchscreen_drv_s.type =LV_INDEV_TYPE_POINTER;
	Touchscreen_drv_s.read_cb =indev_read_cb;
	// LV_INDEV_DEF_LONG_PRESS_TIME
	Touchscreen_drv_s.long_press_time = 10;

	/*
	 * Called when an action happened on the input device.
	 * The second parameter is the event from `lv_event_t`
	 * For example to play a sound asoociate to click.
	 * */
	//Touchscreen_drv_s.feedback_cb=indev_click_cb;

	/*	Touchscreen_drv_s.scroll_limit         = LV_INDEV_DEF_SCROLL_LIMIT;
	Touchscreen_drv_s.scroll_throw         = LV_INDEV_DEF_SCROLL_THROW;
	Touchscreen_drv_s.long_press_time      = LV_INDEV_DEF_LONG_PRESS_TIME;
	Touchscreen_drv_s.long_press_repeat_time  = LV_INDEV_DEF_LONG_PRESS_REP_TIME;
	Touchscreen_drv_s.gesture_limit        = LV_INDEV_DEF_GESTURE_LIMIT;
	Touchscreen_drv_s.gesture_min_velocity = LV_INDEV_DEF_GESTURE_MIN_VELOCITY;*/

	//Indev_drv.long_press_time=10;

	/**
	 * Register an initialized input device driver.
	 * @param driver pointer to an initialized 'lv_indev_drv_t' variable.
	 * Only pointer is saved, so the driver should be static or dynamically allocated.
	 * @return pointer to the new input device or NULL on error
	 * The main input device descriptor with driver, runtime data ('proc') and some additional information :
	 * 		typedef struct _lv_indev_t {
	 * 			struct _lv_indev_drv_t * driver;
	 * 			_lv_indev_proc_t proc;
	 * 			struct _lv_obj_t * cursor;     //< Cursor for LV_INPUT_TYPE_POINTER
	 * 			struct _lv_group_t * group;    //< Keypad destination group
	 * 			const lv_point_t * btn_points; //< Array points assigned to the button ()screen will be pressed here by the buttons
	 * 		} lv_indev_t;
	 */
	Touch_screen_s=lv_indev_drv_register(&Touchscreen_drv_s);


	/*
	 * Touchscreen association
	 */
	Grp_objects_s = lv_group_create();
	lv_group_set_default(Grp_objects_s);
	lv_indev_set_group(Touch_screen_s,Grp_objects_s);


	/*
	 * Clock
	 */
	/*	Clock_s=lv_obj_create(NULL);
	lv_clock_display(Clock_s);*/

	/*
	 * Target
	 */
	/*	Target_s=lv_obj_create(NULL);
	lv_obj_clean(Target_s);
	lv_scr_load(Target_s);
	lv_target_display(Target_s);*/

	/*
	 * Vessels
	 */
	/*	Vessels_s=lv_obj_create(NULL);
	lv_obj_clean(Vessels_s);
	lv_scr_load(Vessels_s);
	lv_vessels_display(Vessels_s);*/

	/*
	 * Settings
	 */
	/*	Settings_s=lv_obj_create(NULL);
	lv_obj_clean(Settings_s);
	lv_scr_load(Settings_s);
	lv_settings_display(Settings_s);*/

	/*
	 * Wind
	 */
	/*	Dspl_wind_s=lv_obj_create(NULL);
	//lv_palette_main(LV_PALETTE_CYAN)
	lv_obj_set_style_bg_color(Dspl_wind_s,lv_palette_main(LV_PALETTE_CYAN),0);
	lv_obj_clean(Dspl_wind_s);
	lv_scr_load(Dspl_wind_s);
	lv_wind_display(Dspl_wind_s);*/

	/*
	 * Compass
	 */
	/*	Compass_s=lv_obj_create(NULL);
	lv_obj_clean(Compass_s);
	lv_scr_load(Compass_s);
	lv_compass_display(Compass_s);*/

	/*
	 * Autopilot
	 */
/*	Autopilot_s=lv_obj_create(NULL);
	lv_obj_set_style_bg_color(Autopilot_s,lv_color_hex(DISPLAY_GOLD_RGB),0);
	lv_obj_clean(Autopilot_s);
	lv_scr_load(Autopilot_s);
	lv_autopilot_display(Autopilot_s);*/

	/*
	 * Init Displays_as
	 */
	Displays_as[DISPLAY_TARGET_ID].display_ps=lv_obj_create(NULL);
	Displays_as[DISPLAY_TARGET_ID].init_cb=lv_target_display;
	Displays_as[DISPLAY_TARGET_ID].update_cb=lv_target_update;

	Display_id=DISPLAY_TARGET_ID;
	lv_display_init(Display_id);

	return 0;
}


/*
 * ----------------
 * display_thread
 * ----------------
 */
FAR void *display_thread(pthread_addr_t arg)
{
	while (1)
	{
		lv_timer_handler();
		usleep(DISPLAY_TIMER_MS*1000);
		lv_tick_inc(DISPLAY_TIMER_MS);
		//clock_update_cb();
		//wind_update_cb();
	}
	return 0;
}


/*
 * ----------------
 * indev_read_cb
 * ----------------
 */
void indev_read_cb(lv_indev_drv_t *indevDriver,lv_indev_data_t *indevData)
{
	int fd;
	ssize_t nbytes;
	int errval = 0;
	struct touch_sample_s sample;
	bool valid;


	// Init
	indevData->continue_reading=false;
	valid=false;

	fd = open(TOUCHSCREEN_DEVPATH, O_RDONLY);
	if (fd<0)
	{
		return;
	}
	nbytes=read(fd,&sample,sizeof(struct touch_sample_s));

	// Handle unexpected return values
	if (nbytes == sizeof(struct touch_sample_s))
	{
		valid=((sample.point[0].flags & TOUCH_POS_VALID) == TOUCH_POS_VALID);
		if (valid)
		{
			if (sample.point[0].flags & TOUCH_UP)
			{
				//indevData->state=LV_INDEV_STATE_REL;// Released
				indevData->state=LV_INDEV_STATE_RELEASED;// Released
			} else if (sample.point[0].flags & TOUCH_DOWN )
			{
				//indevData->state=LV_INDEV_STATE_PR; // Pressed
				indevData->state=LV_INDEV_STATE_PRESSED; // Pressed
			} else if (sample.point[0].flags & TOUCH_MOVE )
			{
				//indevData->state=LV_INDEV_STATE_PR; // Pressed
				indevData->state=LV_INDEV_STATE_PRESSED; // Pressed
			}else
			{
				valid=false;
			}
		}

		if (valid)
		{
			indevData->point.x=sample.point[0].x;
			indevData->point.y=sample.point[0].y;
			LOG_D("indev_read_cb: point.x(%d) - y(%d)",indevData->point.x,indevData->point.y);
		}
	}
	close(fd);
	return;
}



/*
 * ----------------
 * indev_click_cb
 * ----------------
 */
void indev_click_cb(lv_indev_drv_t *indevDriver,uint8_t event_u8)
{
	static uint8_t cnt_u8=0;
	if (event_u8==LV_EVENT_PRESSED)
	{
		LOG_D("indev_click_cb: %d",cnt_u8++);

	}
}


/*// cf port/lv_port_touchpad.c
 static void touchpad_read(FAR lv_indev_drv_t *drv, FAR lv_indev_data_t *data)
{
  FAR struct touchpad_obj_s *touchpad_obj = drv->user_data;
  struct touch_sample_s sample;

  // Read one sample
  int nbytes = read(touchpad_obj->fd, &sample,
                    sizeof(struct touch_sample_s));

  // Handle unexpected return values
  if (nbytes == sizeof(struct touch_sample_s))
    {
      uint8_t touch_flags = sample.point[0].flags;

      if (touch_flags & TOUCH_DOWN || touch_flags & TOUCH_MOVE)
        {
          const FAR lv_disp_drv_t *disp_drv = drv->disp->driver;
          lv_coord_t ver_max = disp_drv->ver_res - 1;
          lv_coord_t hor_max = disp_drv->hor_res - 1;

          touchpad_obj->last_x = LV_CLAMP(0, sample.point[0].x, hor_max);
          touchpad_obj->last_y = LV_CLAMP(0, sample.point[0].y, ver_max);
          touchpad_obj->last_state = LV_INDEV_STATE_PR;
        }
      else if (touch_flags & TOUCH_UP)
        {
          touchpad_obj->last_state = LV_INDEV_STATE_REL;
        }
    }

  // Update touchpad data
  data->point.x = touchpad_obj->last_x;
  data->point.y = touchpad_obj->last_y;
  data->state = touchpad_obj->last_state;
}*/


/* =================
 * uORB tasks
 * =================
 */

/*
 * ----------------
 * print_mng_msg
 * ----------------
 */
static void print_mng_msg(FAR const struct orb_metadata *meta,FAR const void *buffer)
{
	FAR const struct mng_msg_s *message = (const struct mng_msg_s*)buffer;
	const orb_abstime now = orb_absolute_time();

	uorbinfo_raw("%s :\ttimestamp: %llu (%llu us ago) val: %s",meta->o_name, message->timestamp, now - message->timestamp,message->cmd_cha);
}



/*
 * ----------------
 * publisher_task
 * ----------------
 */
static int publisher_task(int argc, char *argv[])
{
	const int queue_size = 10;
	struct orb_test1_s sample;
	int instance = 0;
	int ptopic;

	// Reset
	memset(&sample,'\0',sizeof(sample));


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

	ptopic=orb_advertise_multi_queue(ORB_ID(orb_test1),&sample,&instance,queue_size);
	if (ptopic < 0)
	{
		LOG_E("publisher_task: advertise failed: %d", errno);
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
 * ----------------
 * subscriber_task
 * ----------------
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
		LOG_E("subscriber_task: subscribe failed: %d", errno);
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
			LOG_D("subscriber_task: poll timeout");
		}

		if (OK != orb_check(sfd, &updated))
		{
			LOG_W("subscriber_task: check failed");
		} else if (poll_ret < 0 && errno != EINTR)
		{
			printf("subscriber_task: poll error (%d, %d)\n", poll_ret, errno);
		}

		if (fds[0].revents & POLLIN)
		{
			orb_copy(ORB_ID(orb_test1), sfd, &sample);

			LOG_D("subscriber_task: sub_sample.val(%d)",sample.val);
		}
		usleep(250 * 1000);
	}

	// unsubscribe
	ret = orb_unsubscribe(sfd);
	if (ret != OK)
	{
		LOG_E("subscriber_task: orb_unsubscribe failed: %i", ret);
	}
	return 0;
}




/*
 * ----------------
 * mng_dev_publisher_task
 * ----------------
 */
#define MNG_UORB_DEV_PATH "/dev/uorb/mng_msg0"

static int mng_dev_publisher_task(int argc, char *argv[])
{
	struct mng_msg_s sample;
	//const int queue_size = 50;
	//int instance = 0;
	//int ptopic;
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
		snprintf(sample.cmd_cha,MNG_CMD_SIZE,"msg(%d)",cpt_u16);
		// Publish
		orb_publish(ORB_ID(mng_msg),sfd,&sample);
		printf("mng_dev_publisher_task: %s\n",sample.cmd_cha);
		usleep(2000 * 1000);
	}
	close(sfd);

	return 0;
}



/*
 * ----------------
 * mng_dev_subscriber_task
 * ----------------
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



