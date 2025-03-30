/**
 * =====================================
 *  doais_main.cxx
 * -------------------------------------
 *  AIS DoNovae
 *  www.DoNovae.com
 *  Herve Bailly
 * =====================================
 *
 */

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

#include "ais_monitoring.h"



#if defined(CONFIG_FS_BINFS) && (CONFIG_BUILTIN)
#  include <nuttx/binfmt/builtin.h>
#endif

#if defined(CONFIG_LIBC_EXECFUNCS) && defined(CONFIG_EXECFUNCS_SYMTAB)
#  include <nuttx/binfmt/symtab.h>
#endif


#include "ais.h"
#include "doais_mng.h"
#include "doais_serial.h"
#include "doais_db_update.h"
#include "doais_gps.h"
#include "ais_channels.h"
#include "ais_monitoring.h"
#include "doais_gui.h"
#include "display.h"


/*
 * ===================
 * Defines
 * -------------------
 */

/*
 *  TASK   |PIORITIY|STACK
 * 	---------------------
 * 	NSH    |  100   | 2048
 * 	UORB   |  160   | 4096
 * 	SERIAL |  100   | 2048
 * 	GPS    |  100   | 4096
 * 	TIMER  |  100   | 1024
 * 	DISPLAY|  100   | 4096
 * 	DB_UP  |  150   | 4096
 */
#define THREAD_TIMER_STACK_SIZE 2048
#define THREAD_DISPLAY_STACK_SIZE 4096
#define THREAD_SERIAL_STACK_SIZE 4096
#define THREAD_DB_UPDATE_STACK_SIZE 4096
#define THREAD_GPS_STACK_SIZE 4096
//#define THREAD_PRIORITY ((sched_get_priority_max(SCHED_FIFO)+sched_get_priority_min(SCHED_FIFO))/2)
//#define THREAD_DISPLAY_PRIORITY ((2*sched_get_priority_max(SCHED_FIFO)+sched_get_priority_min(SCHED_FIFO))/3)
#define THREAD_PRIORITY 100
#define THREAD_DISPLAY_PRIORITY 100
#define THREAD_DB_UPDATE_PRIORITY 150



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
static int Touch_screen_fd;
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
 *   No protection needed between GUI data
 *   in handler and update in display_thread.
 */
FAR mutex_t Gps_data_mutex_s; // Protect Gps_info_s
FAR mutex_t Monitoring_data_mutex_s;// Protect all ais_monitoring data


#if defined(CONFIG_LIBC_EXECFUNCS) && defined(CONFIG_EXECFUNCS_SYMTAB)
const struct symtab_s CONFIG_EXECFUNCS_SYMTAB[1];
#endif




/*
 * ----------------
 * Prototypes
 * ----------------
 */
//static int gps_task(int argc, FAR char *argv[]);
int display_set_power(uint32_t pw_u32);
uint32_t display_get_power(void);

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
	nxmutex_init(&Monitoring_data_mutex_s);

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
		pthread_attr_setstacksize(&tattr,THREAD_DISPLAY_STACK_SIZE);
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
		pthread_setname_np(pid,"serial_thread");
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
		pthread_attr_setstacksize(&tattr,THREAD_TIMER_STACK_SIZE);
		pthread_create(&pid,&tattr,timer_thread,(pthread_addr_t)0);
		pthread_setname_np(pid,"timer_thread");
	}

	/*
	 * DB update
	 */
	{
		pthread_t pid;
		pthread_attr_t tattr;
		struct sched_param sparam;

		pthread_attr_init(&tattr);
		sparam.sched_priority=THREAD_DB_UPDATE_PRIORITY;
		pthread_attr_setschedparam(&tattr, &sparam);
		pthread_attr_setstacksize(&tattr,THREAD_DB_UPDATE_STACK_SIZE);
		pthread_create(&pid,&tattr,db_update_thread,(pthread_addr_t)0);
		pthread_setname_np(pid, "db_update_thread");
	}

	/*
	 * GPS
	 */
/*	{
		pthread_t pid;
		pthread_attr_t tattr;
		struct sched_param sparam;

		pthread_attr_init(&tattr);
		sparam.sched_priority=THREAD_PRIORITY;
		pthread_attr_setschedparam(&tattr,&sparam);
		pthread_attr_setstacksize(&tattr,THREAD_GPS_STACK_SIZE);
		pthread_create(&pid,&tattr,gps_thread,(pthread_addr_t)0);
		pthread_setname_np(pid,"gps_thread");
	}*/


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
 * ====================
 * display_init
 * --------------------
 */
static int display_init(void)
{
	/* LVGL initialization */
	lv_init();

	/* LVGL port initialization */
	lv_port_init();

	/* Open touch screen */
	Touch_screen_fd = open(TOUCHSCREEN_DEVPATH, O_RDONLY);
	if (Touch_screen_fd<0)
	{
		int errcode = errno;
		LOG_E("display_init: ERR opening touch-screen - errno(%d)",errcode);
	}



	/**
	 * Initialize an input device driver with default values.
	 */
	lv_indev_drv_init(&Touchscreen_drv_s);

	/* src/hal/lv_hal_indev.h */
	Touchscreen_drv_s.type =LV_INDEV_TYPE_POINTER;
	Touchscreen_drv_s.read_cb =indev_read_cb;
	/*
	// LV_INDEV_DEF_LONG_PRESS_TIME
	Touchscreen_drv_s.scroll_limit         = 5; // Drag threshold in pixels : 10
	Touchscreen_drv_s.scroll_throw         = 20; // Drag throw slow-down in [%]. Greater value -> faster slow-down: 10
	Touchscreen_drv_s.long_press_time      = 30; // Long press time in milliseconds: 400
	Touchscreen_drv_s.long_press_repeat_time  = 10; // *Repeated trigger period in long press [ms]: 100
	Touchscreen_drv_s.gesture_limit        = 25; // Gesture threshold in pixels: 50
	Touchscreen_drv_s.gesture_min_velocity = LV_INDEV_DEF_GESTURE_MIN_VELOCITY; // Gesture min velocity at release before swipe (pixels): 3
	*/

	/*
	Touchscreen_drv_s.scroll_limit         = LV_INDEV_DEF_SCROLL_LIMIT; // Drag threshold in pixels : 10
	Touchscreen_drv_s.scroll_throw         = LV_INDEV_DEF_SCROLL_THROW; // Drag throw slow-down in [%]. Greater value -> faster slow-down: 10
	Touchscreen_drv_s.long_press_time      = LV_INDEV_DEF_LONG_PRESS_TIME; // 400
	Touchscreen_drv_s.long_press_repeat_time  = LV_INDEV_DEF_LONG_PRESS_REP_TIME;
	Touchscreen_drv_s.gesture_limit        = LV_INDEV_DEF_GESTURE_LIMIT; // 50
	Touchscreen_drv_s.gesture_min_velocity = LV_INDEV_DEF_GESTURE_MIN_VELOCITY;
	*/

	/*
	 * Called when an action happened on the input device.
	 * The second parameter is the event from `lv_event_t`
	 * For example to play a sound asoociate to click.
	 * */
	//Touchscreen_drv_s.feedback_cb=indev_click_cb;

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
	 * Wind
	 */
	/*	Dspl_wind_s=lv_obj_create(NULL);
	//lv_palette_main(LV_PALETTE_CYAN)
	lv_obj_set_style_bg_color(Dspl_wind_s,lv_palette_main(LV_PALETTE_CYAN),0);
	lv_obj_clean(Dspl_wind_s);
	lv_scr_load(Dspl_wind_s);
	lv_wind_display(Dspl_wind_s);*/

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
	lv_display_init();

	return 0;
}


/*
 * ----------------
 * display_thread
 * ----------------
 */
FAR void *display_thread(pthread_addr_t arg)
{
	uint8_t cpt_u8=0;
	while (1)
	{
		lv_timer_handler();
		usleep(DISPLAY_TIMER_MS*1000);
		lv_tick_inc(DISPLAY_TIMER_MS);

		/*
		 * Displays update
		 */
		if (cpt_u8>>3||(Refresh_b==DISPLAY_REFRESH_ASAP))
		{
			cpt_u8=0;
			Refresh_b=DISPLAY_REFRESH_PERIODIC;
			lv_displays_update();
		}
		cpt_u8++;
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
	struct touch_sample_s sample;
	ssize_t nbytes;
	int errval = 0;
	bool valid;


	// Init
	indevData->continue_reading=false;
	valid=false;

	nbytes=read(Touch_screen_fd,&sample,sizeof(struct touch_sample_s));

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
			//LOG_D("indev_read_cb: point.x(%d) - y(%d)",indevData->point.x,indevData->point.y);
		}
	}
	//close(fd);
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






