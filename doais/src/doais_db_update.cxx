/**
 * =====================================
 *  doais_db_update_s.cxx
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
#include <pthread.h>


#include "doais_db_update.h"
#include "version.h"
#include "configuration_store.h"
#include "ais_channels.h"
#include "circular_queue.h"
#include "ais_monitoring.h"
#include "types.h"
#include "doais_gui.h"

/*
 * --------------------------
 * Defines
 * --------------------------
 */
#define TIMER_INTERVAL_1MS 1000
#define TIMER_INTERVAL_1S (1000*TIMER_INTERVAL_1MS)

/*
 * --------------------------
 * Globals
 * --------------------------
 */
gps_data_t Gps_info_s;
StationData Station_data_s;
Ais_monitoring Monitoring(AIS_CHAINED_LIST_MAX_SZ,AIS_CHAINED_LABEL_MAX_SZ);




/*
 * Cf doais_main.c
 */
extern "C" ORB_DEFINE(timer_db_update,struct orb_timer_db_update_s,0);
extern "C" ORB_DEFINE(ais_db_update,struct orb_ais_db_update_s,0);




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
 * --------------------------
 * db_update_thread
 * --------------------------
 */
#define ORB_DB_IMER_ID 0
#define ORB_DB_UPDATE_ID 1
#define ORB_DB_POLL_NB 2
#define ORB_DB_TIMEOUT (-1)
FAR void *db_update_thread(pthread_addr_t arg)
{
	struct pollfd fds[2];
	struct orb_timer_db_update_s timer_s;
	struct orb_ais_db_update_s ais_s;
	bool updated;
	int sfd;

//	/*
//	 * Subscribe timer_db_update
//	 */
//	if ((sfd=orb_subscribe(ORB_ID(timer_db_update)))<0)
//	{
//		LOG_E("db_update_thread: timer_db_update_s subscribe failed: %d\n", errno);
//		return NULL;
//	}
//
//		/* Get all published messages,
//		 * ensure that publish and subscribe message match
//		 */
//		do
//		{
//			// Check and get
//			orb_check(sfd,&updated);
//			if (updated)
//			{
//				orb_copy(ORB_ID(timer_db_update),sfd,&timer_s);
//			}
//		}
//
//		fds[0].fd     = sfd;
//		fds[0].events = POLLIN;
//
//	while(1)
//	{
//		int poll_ret;
//
//		// Timeout 500ms
//		poll_ret = poll(fds, 1,1000);
//		if (poll_ret == 0){
//			//printf("mng_dev_subscriber_task: poll timeout\n");
//		}
//
//		if (OK != orb_check(sfd, &updated))
//		{
//			printf("mng_dev_subscriber_task: check failed\n");
//			return 0;
//		}
//		else if (poll_ret < 0 && errno != EINTR)
//		{
//			printf("mng_dev_subscriber_task: poll error (%d, %d)\n", poll_ret, errno);
//		}
//
//				if (fds[ORB_DB_IMER_ID].revents & POLLIN)
//				{
//					orb_copy(ORB_ID(timer_db_update),fds[ORB_DB_IMER_ID].fd,&timer_s);
//					LOG_D("db_update_thread: timer_s.dummy_u8(%d)\n",timer_s.dummy_u8);
//				}
//	}

	/*
	 * Subscribe timer_db_update
	 */
	if ((sfd=orb_subscribe(ORB_ID(timer_db_update)))<0)
	{
		LOG_E("db_update_thread: timer_db_update_s subscribe failed: %d\n", errno);
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
	fds[ORB_DB_IMER_ID].fd     = sfd;
	fds[ORB_DB_IMER_ID].events = POLLIN;

	/*
	 * Advertise ais_db_update
	 */
	sfd=orb_advertise_queue(ORB_ID(ais_db_update),&ais_s,ORB_AIS_DB_UPDATE_QUEUE_SIZE);
	if (sfd<0)
	{
		printf("db_update_thread: advertise failed: %d",errno);
		return NULL;
	}

	/*
	 * Subscribe ais_db_update
	 */
	if ((sfd=orb_subscribe(ORB_ID(ais_db_update)))<0)
	{
		LOG_E("db_update_thread: ais_db_update_s_s subscribe failed: %d\n", errno);
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

	fds[ORB_DB_UPDATE_ID].fd     = sfd;
	fds[ORB_DB_UPDATE_ID].events = POLLIN;

	while(1)
	{
		int poll_ret;

		/*
		 * Infinite timeout
		 */
		poll_ret = poll(fds,ORB_DB_POLL_NB,ORB_DB_TIMEOUT);

		if ((OK!=orb_check(fds[ORB_DB_IMER_ID].fd,&updated)&&(OK!=orb_check(fds[ORB_DB_UPDATE_ID].fd,&updated))))
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
		if (fds[ORB_DB_IMER_ID].revents & POLLIN)
		{
			orb_copy(ORB_ID(timer_db_update),fds[ORB_DB_IMER_ID].fd,&timer_s);
			//LOG_D("db_update_thread: timer_s.dummy_u8(%d)\n",timer_s.dummy_u8);
		}

		/*
		 * AIS processing
		 */
		if (fds[ORB_DB_UPDATE_ID].revents & POLLIN)
		{
			RXPacket rx_packet_s(MAX_AIS_RX_PACKET_SIZE);

			// Get bit_payload_pu8 from uORB msg
			orb_copy(ORB_ID(ais_db_update),fds[ORB_DB_UPDATE_ID].fd,&ais_s);
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
	sfd = orb_unsubscribe(fds[ORB_DB_IMER_ID].fd);
	if (sfd != OK)
	{
		LOG_E("db_update_thread: orb_unsubscribe failed: %i", sfd);
		return NULL;
	}

	sfd = orb_unsubscribe(fds[ORB_DB_UPDATE_ID].fd);
	if (sfd != OK)
	{
		LOG_E("db_update_thread: orb_unsubscribe failed: %i", sfd);
		return NULL;
	}
	return NULL;
}






/*
 * --------------------------
 * rx_ais_decode
 * --------------------------
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
 * --------------------------
 * db_update_thread
 * --------------------------
 */
FAR void *timer_thread(pthread_addr_t arg)
{
	struct orb_timer_db_update_s timer_s;
	int sfd;

	// Reset
	memset(&timer_s,'\0',sizeof(timer_s));

	// Advertise
	sfd=orb_advertise_queue(ORB_ID(timer_db_update),&timer_s,ORB_TIMER_DB_UPDATE_QUEUE_SIZE);
	if (sfd < 0)
	{
		printf("timer_thread: advertise failed: %d",errno);
		return 0;
	}

	while(1)
	{
		// Publish
		timer_s.dummy_u8++;
		orb_publish(ORB_ID(timer_db_update),sfd,&timer_s);
		//LOG_D("timer_thread: timer_s.dummy_u8(%d)\n",timer_s.dummy_u8);
		usleep(TIMER_INTERVAL_1S);
	}

	orb_unadvertise(sfd);
	return NULL;
}

