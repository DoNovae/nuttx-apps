/**
 * =====================================
 *  doais_db_update.cxx
 * -------------------------------------
 *  AIS DoNovae
 *  www.DoNovae.com
 *  Herve Bailly
 * =====================================
 *
 * =====================================
 */

#include <sys/ioctl.h>
#include <stdio.h>
#include <nuttx/config.h>
#include <stdio.h>
#include <fcntl.h>
#include <nuttx/mqueue.h>


#include "doais_db_update.h"
#include "version.h"
#include "configuration_store.h"
#include "ais_channels.h"
#include "circular_queue.h"
#include "ais_monitoring.h"
#include "types.h"
#include <pthread.h>

/*
 * --------------------------
 * Defines
 * --------------------------
 */


/*
 * --------------------------
 * Globals
 * --------------------------
 */
gps_data_t Gps_info_s;
StationData Station_data_s;
Ais_monitoring Monitoring(AIS_CHAINED_LIST_MAX_SZ,AIS_CHAINED_LABEL_MAX_SZ);

struct orb_timer_db_update_s
{
	uint64_t timestamp;
	uint8_t dummy_u8;
};

/*
#define ORB_ID(name)  &g_orb_##name
#define ORB_DEFINE(name, structure, cb) \
  const struct orb_metadata g_orb_##name = \
  { \
    #name, \
    sizeof(structure), \
  };
#endif
 */
static ORB_DEFINE(timer_db_update,struct orb_timer_db_update_s,0);
static ORB_DEFINE(ais_db_update,struct orb_ais_db_update_s,0);



/*
 * --------------------------
 * Prototypes
 * --------------------------
 */



/*
 * --------------------------
 * Externs
 * --------------------------
 */
extern StationData Station_data_s;
extern Ais_monitoring Monitoring;


/*
 * --------------------------
 * Functions
 * --------------------------
 */



/*
 * db_update_thread
 */
FAR void *db_update_thread(pthread_addr_t arg)
{
	struct pollfd fds[2];
	struct orb_timer_db_update_s timer_s;
	struct orb_ais_db_update_s ais_s;
	bool updated;
	int sfd;
	int ret;

	/*
	 * Subscribe timer_db_update
	 */
	if ((sfd = orb_subscribe(ORB_ID(timer_db_update)))<0)
	{
		LOG_E("db_update_thread: timer_db_update subscribe failed: %d\n", errno);
		return NULL;
	}

	/* Get all published messages,
	 * ensure that publish and subscribe message match
	 */
	do
	{
		// Check and get
		orb_check(sfd,&updated);
		if (updated)
		{
			orb_copy(ORB_ID(timer_db_update),sfd,&timer_s);
		}
	}
	while (updated);

	fds[0].fd     = sfd;
	fds[0].events = POLLIN;

	/*
	 * Subscribe ais_db_update_s
	 */
	if ((sfd=orb_subscribe(ORB_ID(ais_db_update)))<0)
	{
		LOG_E("db_update_thread: ais_db_update_s subscribe failed: %d\n", errno);
		return NULL;
	}

	/* Get all published messages,
	 * ensure that publish and subscribe message match
	 */
	do
	{
		// Check and get
		orb_check(sfd,&updated);
		if (updated)
		{
			orb_copy(ORB_ID(ais_db_update),sfd,&ais_s);
		}
	}
	while (updated);

	fds[1].fd     = sfd;
	fds[1].events = POLLIN;

	while(1){
		int poll_ret;
		const int nb_objects_i32=2;
		const int timeout_ms_i32=-1; // Infinite timeout

		/*
		 * Infinite timeout
		 */
		poll_ret = poll(fds,nb_objects_i32,timeout_ms_i32);

		if ((OK != orb_check(fds[0].fd,&updated)&& (OK != orb_check(fds[1].fd,&updated))))
		{
			LOG_E("db_update_thread: check failed\n");
			return NULL;
		} else if ((poll_ret<0) && (errno!=EINTR))
		{
			LOG_E("db_update_thread: poll error (%d, %d)\n", poll_ret, errno);
		}

		/*
		 * Timer processing
		 */
		if (fds[0].revents & POLLIN)
		{
			orb_copy(ORB_ID(timer_db_update),fds[0].fd,&timer_s);
			//LOG_D("db_update_thread: timer_s.dummy_u8(%d)\n",timer_s.dummy_u8);
		}

		/*
		 * AIS processing
		 */
		if (fds[1].revents & POLLIN)
		{
			RXPacket rx_packet_s(MAX_AIS_RX_PACKET_SIZE);

			// Get bit_payload_pu8 from uORB msg
			orb_copy(ORB_ID(ais_db_update),fds[1].fd,&ais_s);
			LOG_D("db_update_thread : ais_s.id_u8(%d)",ais_s.id_u8);
			memcpy(rx_packet_s.mPacket,ais_s.packet_au8,ORB_AIS_PACKET);
			LOG_D("db_update_thread : rx_packet_s");
			rx_packet_s.print_bytes();

			/*
			 * Decode
			 */
			rx_ais_decode(rx_packet_s,0xFF);
		}
	}

	// unsubscribe
	ret = orb_unsubscribe(fds[0].fd);
	if (ret != OK)
	{
		LOG_E("db_update_thread: orb_unsubscribe failed: %i", ret);
		return NULL;
	}

	ret = orb_unsubscribe(fds[1].fd);
	if (ret != OK)
	{
		LOG_E("db_update_thread: orb_unsubscribe failed: %i", ret);
		return NULL;
	}
	return NULL;
}






/*
 * rx_ais_decode
 */
bool rx_ais_decode(RXPacket &rx_packet_s, uint8_t ch_u8)
{
	bool newmsg_ok=false;
	Monitor_data* monit_p=0;
	if (ch_u8==0xFF) ch_u8=rx_packet_s.mChannel;
	rx_packet_s.ais_finalize();

	if (rx_packet_s.checkCRC())
	{
		LOG_I("rx_packet_s.ais_type(%d)",rx_packet_s.ais_type());
		switch (rx_packet_s.ais_type())
		{
		case MSG_1:
		case MSG_2:
		case MSG_3:
		{
			AISMessage123 msg123;
			if (msg123.decode(rx_packet_s,ch_u8))
			{
				newmsg_ok=Monitoring.update(&msg123,&monit_p,ch_u8);
				msg123.nmea_encode(rx_packet_s);
			}
			break;
		}
		case MSG_18:
		{
			AISMessage18 msg18;
			if (msg18.decode(rx_packet_s,ch_u8))
			{
				newmsg_ok=Monitoring.update(&msg18,&monit_p,ch_u8);
				msg18.nmea_encode(rx_packet_s);
			}
			break;
		}
		case MSG_24:
		{
			uint8_t partno=rx_packet_s.get_partno(rx_packet_s);
			switch(partno)
			{
			case PARTNO_24A:
			{
				AISMessage24A msg24A;
				LOG_D("PARTNO_24A");
				if (msg24A.decode(rx_packet_s,ch_u8)) {
					if (Monitoring.is_mmsi(msg24A.mmsi,&monit_p)){
						Monitoring.update_from_msg(&msg24A,monit_p);
						msg24A.nmea_encode(rx_packet_s);
						newmsg_ok=true;
					}
				}
				break;
			}
			case PARTNO_24B:
			{
				AISMessage24B msg24B;
				LOG_D("PARTNO_24B");
				if (msg24B.decode(rx_packet_s,ch_u8)) {
					if (Monitoring.is_mmsi(msg24B.mmsi,&monit_p)){
						Monitoring.update_from_msg(&msg24B,monit_p);
						msg24B.nmea_encode(rx_packet_s);
						newmsg_ok=true;
					}
				}
				break;
			}
			default:
			{
				LOG_W("AISMessage24A mmsi(%d) - error partno(%d)",rx_packet_s.ais_mmsi(),partno);
				return false;
			}
			}
			break;
		}
		default:
		{
			LOG_V("mmsi(%d) - type(%d) not processed",rx_packet_s.ais_mmsi(),rx_packet_s.ais_type());
		}
		}
	}
	return newmsg_ok;
}









/*
 * db_update_thread
 */
FAR void *timer_thread(pthread_addr_t arg)
{
	const int queue_size = 10;
	struct orb_timer_db_update_s timer_s;
	int instance = 0;
	int ptopic_timer;

	// Reset
	memset(&timer_s,'\0',sizeof(timer_s));


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

	ptopic_timer=orb_advertise_multi_queue(ORB_ID(timer_db_update),&timer_s,&instance,queue_size);
	if (ptopic_timer < 0)
	{
		LOG_E("timer_thread: timer_db_update advertise failed: %d", errno);
	}

	while(1)
	{
		// Publish
		timer_s.dummy_u8++;
		orb_publish(ORB_ID(timer_db_update),ptopic_timer,&timer_s);
		usleep(TIMER_INTERVAL_US);
	}

	orb_unadvertise(ptopic_timer);
	return NULL;
}

