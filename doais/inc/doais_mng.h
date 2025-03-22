#include <errno.h>
#include <math.h>
#include <poll.h>
#include <string.h>
#include <stdarg.h>
#include <sys/param.h>
#include <uORB/uORB.h>
#include "ais.h"

#ifndef DOAIS_MNG_H
#define DOAIS_MNG_H

/*
 * Defines
 */
#define MNG_CMD_SIZE 64
#define ORB_AIS_PACKET ((MAX_AIS_RX_PACKET_SIZE+1)>>3)

#define ORB_AIS_DB_UPDATE_QUEUE_SIZE 2
#define ORB_MNG_MSG_QUEUE_SIZE 2
#define ORB_TIMER_DB_UPDATE_QUEUE_SIZE 2

/*
 * -----------------
 * Typedef
 * -----------------
 */


/*
 * -----------------
 * uOrb message
 * -----------------
 */

/**
 * **************
 * ORB_ID
 * **************
 * Generates a pointer to the uORB metadata structure for
 * a given topic.
 *
 * The topic must have been declared previously in scope
 * with ORB_DECLARE().
 *
 * @param name    The name of the topic.

#define ORB_ID(name)  &g_orb_##name
 */

/**
 * **************
 * ORB_DECLARE
 * **************
 * Declare the uORB metadata for a topic (used by code generators).
 *
 * @param name      The name of the topic.

#if defined(__cplusplus)
# define ORB_DECLARE(name) extern "C" const struct orb_metadata g_orb_##name
#else
# define ORB_DECLARE(name) extern const struct orb_metadata g_orb_##name
#endif
 */

/**
 * **************
 * ORB_DEFINE
 * **************
 * Define (instantiate) the uORB metadata for a topic.
 *
 * The uORB metadata is used to help ensure that updates and
 * copies are accessing the right data.
 *
 * Note that there must be no more than one instance of this macro
 * for each topic.
 *
 * @param name    The name of the topic.
 * @param struct  The structure the topic provides.
 * @param cb      The function pointer of output topic message.

#ifdef CONFIG_DEBUG_UORB
#define ORB_DEFINE(name, structure, cb) \
  const struct orb_metadata g_orb_##name = \
  { \
    #name, \
    sizeof(structure), \
    cb, \
  };
#else
#define ORB_DEFINE(name, structure, cb) \
  const struct orb_metadata g_orb_##name = \
  { \
    #name, \
    sizeof(structure), \
  };
#endif

#ifdef __cplusplus
extern "C"
{
#endif
*/


struct orb_test1_s
{
	uint64_t timestamp;
	int32_t val;
};

struct orb_mng_msg_s
{
	uint64_t timestamp;
	char cmd_cha[MNG_CMD_SIZE];
};

struct orb_ais_db_update_s
{
	uint8_t packet_au8[ORB_AIS_PACKET];
	uint64_t timestamp;
	uint8_t id_u8;
};

struct orb_timer_db_update_s
{
	uint64_t timestamp;
	uint8_t dummy_u8;
};



#endif //DOAIS_MNG_H
