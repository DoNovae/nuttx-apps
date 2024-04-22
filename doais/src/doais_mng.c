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
#include "doais_mng.h"



/*
 * Globals
 */
ORB_DEFINE(mng_msg,struct mng_msg_s,print_mng_msg);


/*
 * Prototypes
 */
static int mng_dev_publish(char *msg_pc);


/*
 * Main
 */
int main(int argc, FAR char *argv[])
{
	if (argc != 2){
		printf("Usage : doais_mng <cmd>\n");
		return 0;
	}
	mng_dev_publish(argv[1]);

	return 0;
}






/*
 * /dev/uorb/mng_msg0
 */
static int mng_dev_publish(char *msg_pc)
{
	struct mng_msg_s sample;
	const int queue_size = 50;
	int instance = 0;
	int ptopic;
	int sfd;

	// Subscribe
	sfd=0;
	do
	{
		sfd=orb_open("mng_msg",0,O_RDWR);
		if (sfd<0) usleep(1000 * 1000);
	}while(sfd < 0);

	// Reset
	memset(&sample, '\0', sizeof(sample));


	memset(sample.cmd_cha,0,MNG_CMD_SIZE);
	snprintf(sample.cmd_cha,MNG_CMD_SIZE,"%s",msg_pc);
	// Publish
	orb_publish(ORB_ID(mng_msg),sfd,&sample);
	printf("mng_dev_publish: %s\n",sample.cmd_cha);
	close(sfd);

	return 0;
}





