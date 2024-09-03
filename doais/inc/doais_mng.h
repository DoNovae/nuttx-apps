#include <errno.h>
#include <math.h>
#include <poll.h>
#include <string.h>
#include <stdarg.h>
#include <sys/param.h>
#include "uORB/uORB.h"

#ifndef DOAIS_MNG_H
#define DOAIS_MNG_H

/*
 * Defines
 */
#define MNG_UORB_DEV_PATH "/dev/uorb/mng_msg0"
#define MNG_CMD_SIZE 256

/*
 * Typedef
 */
struct orb_test1_s
{
	uint64_t timestamp;
	int32_t val;
};

struct mng_msg_s
{
	uint64_t timestamp;
	char cmd_cha[MNG_CMD_SIZE];
};

//#define ORB_DECLARE(name) extern const struct orb_metadata g_orb_##name
ORB_DECLARE(mng_msg);
ORB_DECLARE(orb_test1);


#endif //DOAIS_MNG_H
