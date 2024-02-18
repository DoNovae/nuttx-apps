/****************************************************************************
 * apps/examples/i2schar/i2schar_main.c
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
#include <nuttx/audio/audio.h>
#include <nuttx/audio/pcm.h>

#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <debug.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <assert.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>

#include "i2schar.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Types
 ****************************************************************************/

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Public Data
 ****************************************************************************/

struct i2schar_state_s g_i2schar;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: i2schar_devpath
 ****************************************************************************/

static void i2schar_devpath(FAR struct i2schar_state_s *i2schar,
		FAR const char *devpath)
{
	/* Get rid of any old device path */

	if (i2schar->devpath)
	{
		free(i2schar->devpath);
	}

	/* Then set-up the new device path by copying the string */

	i2schar->devpath = strdup(devpath);
}

/****************************************************************************
 * Name: i2schar_help
 ****************************************************************************/

static void i2schar_help(FAR struct i2schar_state_s *i2schar)
{
	printf("Usage: i2schar [OPTIONS]\n");
	printf("\nArguments are \"sticky\".\n");
	printf("For example, once the I2C character device is\n");
	printf("specified, that device will be re-used until it is changed.\n");
	printf("\n\"sticky\" OPTIONS include:\n");
	printf("  [-p devpath] selects the I2C character device path.  "
			"Default: %s Current: %s\n",
			CONFIG_EXAMPLES_I2SCHAR_DEVPATH,
			g_i2schar.devpath ? g_i2schar.devpath : "NONE");
#ifdef CONFIG_EXAMPLES_I2SCHAR_TX
	printf("  [-t count] selects the number of audio buffers to send.  "
			"Default: %d Current: %d\n",
			CONFIG_EXAMPLES_I2SCHAR_TXBUFFERS, i2schar->txcount);
#endif
#ifdef CONFIG_EXAMPLES_I2SCHAR_RX
	printf("  [-r count] selects the number of audio buffers to receive.  "
			"Default: %d Current: %d\n",
			CONFIG_EXAMPLES_I2SCHAR_RXBUFFERS, i2schar->rxcount);
#endif
	printf("  [-h] shows this message and exits\n");
}

/****************************************************************************
 * Name: arg_string
 ****************************************************************************/

static int arg_string(FAR char **arg, FAR char **value)
{
	FAR char *ptr = *arg;

	if (ptr[2] == '\0')
	{
		*value = arg[1];
		return 2;
	}
	else
	{
		*value = &ptr[2];
		return 1;
	}
}

/****************************************************************************
 * Name: arg_decimal
 ****************************************************************************/

static int arg_decimal(FAR char **arg, FAR long *value)
{
	FAR char *string;
	int ret;

	ret = arg_string(arg, &string);
	*value = strtol(string, NULL, 10);
	return ret;
}

/****************************************************************************
 * Name: parse_args
 ****************************************************************************/

static void parse_args(FAR struct i2schar_state_s *i2schar,
		int argc,
		FAR char **argv)
{
	FAR char *ptr;
	FAR char *str;
	long value;
	int index;
	int nargs;

	for (index = 1; index < argc; )
	{
		ptr = argv[index];
		if (ptr[0] != '-')
		{
			printf("Invalid options format: %s\n", ptr);
			exit(0);
		}

		switch (ptr[1])
		{
		case 'p':
			nargs = arg_string(&argv[index], &str);
			i2schar_devpath(i2schar, str);
			index += nargs;
			break;

#ifdef CONFIG_EXAMPLES_I2SCHAR_RX
		case 'r':
			nargs = arg_decimal(&argv[index], &value);
			if (value < 0)
			{
				printf("Count must be non-negative: %ld\n", value);
				exit(1);
			}

			i2schar->rxcount = (uint32_t)value;
			index += nargs;
			break;
#endif

#ifdef CONFIG_EXAMPLES_I2SCHAR_TX
		case 't':
			nargs = arg_decimal(&argv[index], &value);
			if (value < 0)
			{
				printf("Count must be non-negative: %ld\n", value);
				exit(1);
			}

			i2schar->txcount = (uint32_t)value;
			index += nargs;
			break;
#endif

		case 'h':
			i2schar_help(i2schar);
			exit(0);

		default:
			printf("Unsupported option: %s\n", ptr);
			i2schar_help(i2schar);
			exit(1);
		}
	}
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: i2schar_main
 ****************************************************************************/

int main(int argc, FAR char *argv[])
{
	pthread_attr_t attr;
	pthread_addr_t result;
	pthread_t transmitter;
	int fd;
	int ret;
	struct audio_caps_desc_s cap_desc;
	int ch;
	int samprate;
	uint8_t bpsamp;
	uint8_t chmap;

	/*
	 * Configure audio port
	 */
	fd = open("/dev/audio/pcm0", O_WRONLY);
	//fd = open("/dev/i2schar0", O_WRONLY);
	DEBUGASSERT(0 < fd);

	ch = 1;
	samprate = 16000;
	bpsamp=16;
	chmap=0;
	cap_desc.caps.ac_len            = sizeof(struct audio_caps_s);
	cap_desc.caps.ac_type           = AUDIO_TYPE_OUTPUT;
	cap_desc.caps.ac_channels       = ch;
	cap_desc.caps.ac_chmap          = chmap;
	cap_desc.caps.ac_controls.hw[0] = samprate;
	cap_desc.caps.ac_controls.b[3]  = samprate >> 16;
	cap_desc.caps.ac_controls.b[2]  = bpsamp;
	cap_desc.caps.ac_subtype        = AUDIO_FMT_WAV;
	ioctl(fd, AUDIOIOC_CONFIGURE, (unsigned long)&cap_desc);
	close(fd);

	sched_lock();

	/* Start the transmitter thread */
	printf("i2schar_main: Start transmitter thread\n");
	pthread_attr_init(&attr);

	/* Set the transmitter stack size */
	pthread_attr_setstacksize(&attr,CONFIG_EXAMPLES_I2SCHAR_TXSTACKSIZE);

	/* Start the transmitter */
	ret = pthread_create(&transmitter, &attr, i2schar_transmitter, NULL);
	if (ret != OK)
	{
		sched_unlock();
		printf("i2schar_main: ERROR: failed to Start transmitter thread: %d\n",
				ret);
		return EXIT_FAILURE;
	}

	pthread_setname_np(transmitter, "transmitter");

	sched_unlock();
	printf("i2schar_main: Waiting for the transmitter thread\n");
	ret = pthread_join(transmitter, &result);
	if (ret != OK)
	{
		printf("i2schar_main: ERROR: pthread_join failed: %d\n", ret);
	}

	return EXIT_SUCCESS;
}
