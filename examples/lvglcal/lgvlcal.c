/****************************************************************************
 * apps/examples/lvgldemo/lvgldemo.c
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

#include <sys/boardctl.h>
#include <sys/param.h>
#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <debug.h>

#include <lvgl/lvgl.h>
#include <port/lv_port.h>
#include <nuttx/input/touchscreen.h>

#include "lv_tc.h"
#include "lv_tc_screen.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Should we perform board-specific driver initialization?  There are two
 * ways that board initialization can occur:  1) automatically via
 * board_late_initialize() during bootupif CONFIG_BOARD_LATE_INITIALIZE
 * or 2).
 * via a call to boardctl() if the interface is enabled
 * (CONFIG_BOARDCTL=y).
 * If this task is running as an NSH built-in application, then that
 * initialization has probably already been performed otherwise we do it
 * here.
 */

#undef NEED_BOARDINIT

#if defined(CONFIG_BOARDCTL) && !defined(CONFIG_NSH_ARCHINIT)
#  define NEED_BOARDINIT 1
#endif


/****************************************************************************
 * Private Type Declarations
 ****************************************************************************/


/****************************************************************************
 * Private Data
 ****************************************************************************/


/****************************************************************************
 * Private Functions
 ****************************************************************************/



/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: main or lvgldemo_main
 *
 * Description:
 *
 * Input Parameters:
 *   Standard argc and argv
 *
 * Returned Value:
 *   Zero on success; a positive, non-zero value on failure.
 *
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{
	static lv_indev_drv_t indevDrv;

#ifdef CONFIG_LIBUV
  uv_loop_t ui_loop;
#endif


#ifdef NEED_BOARDINIT
  /* Perform board-specific driver initialization */

  boardctl(BOARDIOC_INIT, 0);

#ifdef CONFIG_BOARDCTL_FINALINIT
  /* Perform architecture-specific final-initialization (if configured) */

  boardctl(BOARDIOC_FINALINIT, 0);
#endif
#endif

  /* LVGL initialization */

  lv_init();

  /* LVGL port initialization */
  lv_port_init();
  /*
          Initialize the calibrated touch driver.
          Sets its type to LV_INDEV_TYPE_POINTER,
          uses its user_data field. DO NOT OVERRIDE

          Also provide a read callback to interface with your touch controller!
      */
      lv_tc_indev_drv_init(&indevDrv, your_indev_read_cb);

      /*
          Register the driver.
      */
      lv_indev_drv_register(&indevDrv);

      /*
          If using NVS:
          Register a calibration coefficients save callback.
      */
      lv_tc_register_coeff_save_cb(your_nvs_coeff_save_cb);

      /*
          Create the calibration screen.
      */
      lv_obj_t *tCScreen = lv_tc_screen_create();

      /*
          Register a callback for when the calibration finishes.
          An LV_EVENT_READY event is triggered.
      */
      lv_obj_add_event_cb(tCScreen, your_tc_finish_cb, LV_EVENT_READY, NULL);

      /*
          If using NVS:
          Init NVS and check for existing calibration data.
      */
      if(your_nvs_init_and_check()) {
          /*
              Data exists: proceed with the normal application without
              showing the calibration screen
          */
          your_start_application(); /* Implement this */
      } else {
          /*
              There is no data: load the calibration screen, perform the calibration
          */
          lv_disp_load_scr(tCScreen);
          lv_tc_screen_start(tCScreen);
      }

  return EXIT_SUCCESS;
}



/*
    Your touch panel driver read callback
*/
void your_indev_read_cb(lv_indev_drv_t *indevDriver,lv_indev_data_t *indevData) {
	int fd;
	ssize_t nbytes;
	int errval = 0;
	struct touch_sample_s sample;

	fd = open(TOUCHSCREEN_DEVPATH, O_RDONLY);
	if (fd<0)
	{
		errval = 2;
		return;
	}
	nbytes=read(fd,&sample,sizeof(struct touch_sample_s));

	/* Handle unexpected return values */
	if (nbytes<0||(nbytes!=sizeof (struct touch_sample_s)))
	{
		errval = errno;
		if (errval != EINTR)
		{
			//LV_LOG_ERROR("tc_main: read %s failed: %d\n",TOUCHSCREEN_DEVPATH,errval);
			errval = 3;
		}
	} else if (nbytes != sizeof(struct touch_sample_s)) {
		LV_LOG_WARN("tc_main: Unexpected read size=%zd, expected=%zd, Ignoring\n",nbytes,sizeof(struct touch_sample_s));
	} else /* Print the sample data on successful return */
	{
		indevData->state=(sample.point[0].flags&TOUCH_ID_VALID) ? LV_INDEV_STATE_REL : LV_INDEV_STATE_PR;
		indevData->point.x=sample.point[0].x;
		indevData->point.y=sample.point[0].y;
		//data->key = state->key_val;
		LV_LOG_INFO("point.x(%d) - y(%d)",indevData->point.x,indevData->point.y);
	}
	close(fd);
}

/*
    If using NVS:
    Your callback for writing the new coefficients to NVS
*/
void your_nvs_coeff_save_cb(lv_tc_coeff_t coeff) {

    /* Implement this */
}

/*
    Your callback for when the calibration finishes
*/
void your_tc_finish_cb(lv_event_t *event) {
    /*
        Load the application
    */
    your_start_application(); /* Implement this */
}

/*
    If using NVS:
    Your function for initializing NVS
    and checking wheter calibration data is stored
    @returns true if it exists, false if not
*/
bool your_nvs_init_and_check() {
    /*
        Initialize NVS
    */

    /* Implement this */

    /*
        Check for existing data
    */

    /* Implement this */
}
