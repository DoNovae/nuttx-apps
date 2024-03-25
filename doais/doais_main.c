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



/*
 * Two tasks test
 */

#define ACCEL_TASK_INTERVAL_MS 10
static pid_t serial_pid;
static pid_t display_pid;

static int serial_task(int argc, FAR char *argv[])
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

static int display_task(int argc, FAR char *argv[])
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
  while (1) {
	  printf("Display_task\n");
	  usleep(ACCEL_TASK_INTERVAL_MS * 1000L);
  }
  return 0;
}




int main(int argc, FAR char *argv[]) {
  printf("Starting Accelerometer Task Example\n");
  char *child_argv[2];
  struct mq_attr attr;

  serial_pid = task_create(
    "Serial Task",
    120,
    700,
	serial_task,
    (char* const*) child_argv);
  if (serial_pid < 0) {
    printf("Failed to create accelerometer task\n");
  }

  display_pid = task_create(
    "Display Task",
    110,
    700,
   display_task,
    (char* const*) child_argv);
  if (display_pid < 0) {
    printf("Failed to create offload task\n");
  }

  return 0;
}



