/**
 * =====================================
 * si446x_1.cpp
 * -------------------------------------
 *  AIS DoNovae
 *  www.DoNovae.com
 *  Herve Bailly
 * =====================================
 */
#include <Wire.h>
#include <Arduino.h>
#include <math.h>

#include "si4463.h"
#include "bsp.h"
#include "ais_messages.h"
#include "ais_utils.h"
#include "ais_channels.h"
#include "utilities.h"
#include "ais_utils.h"
#include "wifi_.h"
#include "ais_monitoring.h"


/*
 * ========================
 * MULTIPLE CONFIGURATION
 * ========================
 * Create
 * 	  radio_config_XXX.h
 * 	  radio_xxx.h -> namespace XXX
 * 	  	#define RADIO_H_XXX
 * 	  radio_xxx.cpp -> namespace XXX
 * ulilities.h
 *    TX configuration
 *    	#define PREAMBLE_FLAGCC
 *    	cf AISmessages.cpp, tx_packet.cpp
 * ========================
 */

/*
 * Just for RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH
 */
#include "radio_config_Si4463.h"
#include "radio_Si4463.h"

using namespace SI4463_NSTDPREAMBLE_NOFLAG;


namespace SI4463
{

/*
 * ========================
 *  GLOBAL
 * ========================
 */

/*
 * ========================
 * si446x
 * -----------------------
 *  IRQP
 *      Gpio 34-39 are only input, no pullups/pulldowns
 * -----------------------
 */
//HBL070723 #define RF4463_IRQ1  GPIO_NUM_32
//#define RF4463_IRQ1  GPIO_NUM_35 /*ADC*/
#define RF4463_IRQ1  GPIO_NUM_32 /*GPIO*/
#define RF4463_SDN1  GPIO_NUM_26 /*DAC*/
//#define RF4463_NSEL1 GPIO_NUM_25 /*DAC*/
#define RF4463_NSEL1 GPIO_NUM_27 /*GPIO*/
Si446x si446x1_(RF4463_IRQ1,RF4463_SDN1,RF4463_NSEL1);

//HBL070723 #define RF4463_IRQ2  GPIO_NUM_32
//#define RF4463_IRQ2  GPIO_NUM_35 /*ADC*/
#define RF4463_IRQ2  GPIO_NUM_32 /*GPIO*/
//#define RF4463_SDN2  GPIO_NUM_34 /*ADC*/
#define RF4463_SDN2  GPIO_NUM_25 /*DAC*/
#define RF4463_NSEL2 GPIO_NUM_33 /*GPIO*/
Si446x si446x2_(RF4463_IRQ2,RF4463_SDN2,RF4463_NSEL2);


/*
 * ========================
 *  Si446x
 * ========================
 *   -IRQPin: input pin,interrupt of RF4463
 *   -sdnPin: output pin,enter shutdown mode when driving high
 *   -nSELPin: output pin,slave select pin
 *
 * ========================
 */
#define BOAT_NAME "Tete d'Artichaut"
Si446x::Si446x(int8_t nIRQPin,int8_t sdnPin,int8_t nSELPin)
: tx_packet_s(MAX_AIS_RX_PACKET_SIZE),rx_packet_s(MAX_AIS_RX_PACKET_SIZE)
{
	_nIRQPin=nIRQPin;
	_sdnPin=sdnPin;
	_nSELPin=nSELPin;
	tx_packet_s.reset();
	rx_packet_s.reset();

	rx_packet_ct=0;
	tx_packet_ct=0;

	SI4463_SDN_PIN=0XFF;
	SI4463_IRQ_PIN=0XFF;
	SI4463_SEL_PIN=0XFF;
	VSPI2=NULL;
	_VSPI2=NULL;
}

Si446x::~Si446x()
{
}


void Si446x::set_pin(){
	SI4463_SDN_PIN=_sdnPin;
	SI4463_IRQ_PIN=_nIRQPin;
	SI4463_SEL_PIN=_nSELPin;
	VSPI2=_VSPI2;
}

/*
 * *********************
 * init
 * *********************
 *  Set
 *  	Pins _nSELPin, _nIRQPin, _sdnPin
 *  	Interrupt handler si446x_irq
 * ********************
 */
void Si446x::init() {
	// Pull down
	//pinMode(_nIRQPin,INPUT_PULLDOWN);
	pinMode(_nIRQPin,INPUT_PULLUP);
	pinMode(_nSELPin,OUTPUT);
	// pinMode(_sdnPin,OUTPUT); // DAC

	/*
	 * SPI
	 */
	SPI_DESELECT(_nSELPin);

	/*
	 * SPI
	 */
	_VSPI2=new SPIClass(VSPI);
	//_VSPI2->setFrequency(10000000);
	_VSPI2->begin(VSPI_SCLK,VSPI_MISO,VSPI_MOSI,_nSELPin);
}

void Si446x::radio_init()
{
	set_pin();
	vRadio_Init();
}

void Si446x::set_property(uint8_t group_u8, uint8_t num_prop_u8, uint8_t prop_u8)
{
	set_pin();
	Pro2Cmd[0] = SI446X_CMD_ID_SET_PROPERTY;
	Pro2Cmd[1] = group_u8;
	Pro2Cmd[2] = num_prop_u8;
	Pro2Cmd[3] = prop_u8;

	radio_comm_SendCmd(4,Pro2Cmd);
}


void Si446x::version()
{
	set_pin();
	vRadio_Version();
}



/*
 * ==============================
 * Statistics
 * ==============================
 */
void Si446x::stats_init(){
	for (int16_t i=0;i<(int16_t)HARDWARE_STATS_LEN;i++)
	{
		stats_it_i16[i]=0;
	}
}

int16_t Si446x::stats_it_i16[HARDWARE_STATS_LEN];
void Si446x::stats_do(){
	set_pin();
	/* Read ITs, leave interrupt pending */
	si446x_get_int_status(0xff,0xff,0xff);

	stats_it_i16[STATS_CPT]++;
	/*
	 * Check for CHIP IT
	 */

	/*
	 * CHIP_PEND_CMD_ERROR
	 */
	if (Si446xCmd.GET_INT_STATUS.CHIP_PEND & SI446X_CMD_GET_CHIP_STATUS_REP_CHIP_PEND_CMD_ERROR_PEND_BIT)
	{
		LOG_D("%d: CHIP_PEND_CMD_ERROR",stats_it_i16[STATS_CPT]);
		stats_it_i16[CHIP_CMD_ERROR]++;
	}

	/*
	 * Check for MODEM IT
	 */

	/*
	 * MODEM_STATUS_PREAMBLE_DETECT / MODEM_STATUS_INVALID_PREAMBLE
	 */
	if (Si446xCmd.GET_INT_STATUS.MODEM_PEND & SI446X_CMD_GET_MODEM_STATUS_REP_MODEM_STATUS_PREAMBLE_DETECT_BIT)
	{
		LOG_D("%d: MODEM_STATUS_PREAMBLE_DETECT",stats_it_i16[STATS_CPT]);
		stats_it_i16[MODEM_PREAMBLE_DETECT]++;
	}

	if (Si446xCmd.GET_INT_STATUS.MODEM_PEND & SI446X_CMD_GET_MODEM_STATUS_REP_MODEM_STATUS_INVALID_PREAMBLE_BIT)
	{
		LOG_D("%d: MODEM_STATUS_INVALID_PREAMBLE",stats_it_i16[STATS_CPT]);
		stats_it_i16[MODEM_INVALID_PREAMBLE]++;
	}

	/*
	 * MODEM_STATUS_SYNC_DETECT / MODEM_STATUS_INVALID_SYNC
	 */
	if (Si446xCmd.GET_INT_STATUS.MODEM_PEND & SI446X_CMD_GET_MODEM_STATUS_REP_MODEM_STATUS_SYNC_DETECT_BIT)
	{
		LOG_D("%d: MODEM_STATUS_SYNC_DETECT",stats_it_i16[STATS_CPT]);
		stats_it_i16[MODEM_SYNC_DETECT]++;
	}

	if (Si446xCmd.GET_INT_STATUS.MODEM_PEND & SI446X_CMD_GET_MODEM_STATUS_REP_MODEM_STATUS_INVALID_SYNC_BIT)
	{
		LOG_D("%d: MODEM_STATUS_INVALID_SYNC",stats_it_i16[STATS_CPT]);
		stats_it_i16[MODEM_INVALID_SYNC]++;
	}

	/*
	 * Check for MODEM IT
	 */

	/*
	 * PH_STATUS_CRC_ERROR
	 */
	if (Si446xCmd.GET_INT_STATUS.PH_PEND & SI446X_CMD_GET_INT_STATUS_REP_PH_STATUS_CRC_ERROR_BIT)
	{
		LOG_D("%d: PH_STATUS_CRC_ERROR",stats_it_i16[STATS_CPT]);
		stats_it_i16[PH_CRC_ERROR]++;
	}

	/*
	 * PH_PEND_PACKET_SENT
	 */
	if(Si446xCmd.GET_INT_STATUS.PH_PEND & SI446X_CMD_GET_INT_STATUS_REP_PH_PEND_PACKET_SENT_PEND_BIT)
	{
		LOG_D("%d: PH_PEND_PACKET_SENT",stats_it_i16[STATS_CPT]);
		stats_it_i16[PH_PACKET_SENT]++;
	}

	/*
	 * PH_PEND_PACKET_RX
	 */
	if(Si446xCmd.GET_INT_STATUS.PH_PEND & SI446X_CMD_GET_INT_STATUS_REP_PH_PEND_PACKET_RX_PEND_BIT)
	{
		LOG_D("%d: PH_PEND_PACKET_RX",stats_it_i16[STATS_CPT]);
		stats_it_i16[PH_PACKET_RX]++;
	}
}

void Si446x::stats_printf(){
	ais_wifi::wifi_printf("stats_printf: STATS_CPT=%d\n",stats_it_i16[STATS_CPT]);
	ais_wifi::wifi_printf("| CPT |CHIP_CMD_ERROR|MODEM_PREAMBLE_DETECT|MODEM_INVALID_PREAMBLE|MODEM_SYNC_DETECT|MODEM_INVALID_SYNC|PH_CRC_ERROR|PH_PACKET_SENT|PH_PACKET_RX| CRC_OK |\n");
	ais_wifi::wifi_printf("|%05d|    %05d     |       %05d         |         %05d        |       %05d     |       %05d      |    %05d   |    %05d     |   %05d    | %05d  |\n",
			stats_it_i16[STATS_CPT],
			stats_it_i16[CHIP_CMD_ERROR],
			stats_it_i16[MODEM_PREAMBLE_DETECT],
			stats_it_i16[MODEM_INVALID_PREAMBLE],
			stats_it_i16[MODEM_SYNC_DETECT],
			stats_it_i16[MODEM_INVALID_SYNC],
			stats_it_i16[PH_CRC_ERROR],
			stats_it_i16[PH_PACKET_SENT],
			stats_it_i16[PH_PACKET_RX],
			stats_it_i16[STATS_CRC_OK]);
}


void Si446x::check(){
	set_pin();
	bRadio_Check_Tx_RX();
};




/*
 * =============================
 * TX
 * =============================
 */
uint8_t *Si446x::tx_buffer_u8(){return tx_packet_s.mPacket;};
int16_t Si446x::tx_size_i16(){return tx_packet_s.bit_size_i16;};

void Si446x::tx_send_ais_packet()
{
	set_pin();
	uint8_t len_u8;
	set_pin();
	len_u8=(tx_packet_s.index_i16+7)>>3;
	LOG_I("RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH=%d - len_u8=%d",RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH,len_u8);
#if DEV_MODE
	ASSERT(len_u8<=RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH);
#endif
	tx_packet_ct++;
	get_fifos_status(1);
	vRadio_StartTx_Variable_Packet(AIS_CHANNELS[tx_packet_s.mChannel].ordinal,tx_packet_s.mPacket,RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH);
	get_fifos_status(0);
}



void Si446x::tx_build_ais_packet(AISMessage *msg)
{
	tx_packet_s.reset();
	switch (msg->type){
	case MSG_1:
	case MSG_2:
	case MSG_3:
	case MSG_18:
	case MSG_4:
		msg->encode(Station_data_s,Gps_info_s,tx_packet_s);
		break;
	case MSG_24:
	case MSG_12:
		msg->encode(Station_data_s,tx_packet_s);
		break;
	default:
		LOG_E("Unknown message type %d",msg->type);
		return;
	}
	tx_packet_s.ais_finalize();
}

/*
 * =============================
 * set_tx_power
 * =============================
 * Group Nbr    Name                    Default Summary
 * 0x22	0x00	PA_MODE	                0x08	Selects the PA operating mode, and selects resolution of PA power adjustment (i.e., step size).
 * 0x22	0x01	PA_PWR_LVL	            0x7f	Configuration of PA output power level.
 * 0x22	0x02	PA_BIAS_CLKDUTY	        0x00	Configuration of the PA Bias and duty cycle of the TX clock source.
 * 0x22	0x03	PA_TC	                0x5d	Configuration of PA ramping parameters.
 * 0x22	0x04	PA_RAMP_EX	            0x80	Select the time constant of the external PA ramp signal.
 * 0x22	0x05	PA_RAMP_DOWN_DELAY	    0x23	Delay from the start of the PA ramp down to disabling of the PA output.
 * 0x22	0x06	PA_DIG_PWR_SEQ_CONFIG	0x03	Configuration for digital power sequencing.
 *
 * PA_PWR_LVL:
 * 		DDAC[6:0]Up to Register View
 * 		Description: Selects the number of enabled output device fingers,
 * 			with a larger value resulting in increased output power.
 * 			Min: 0x0
 * 			Max: 0x7f
 * 			Default: 0x7f
 * =============================
 */

void Si446x::tx_set_power(uint8_t pwr_u8)
{
	set_pin();
	LOG_I("pwr_u8: %d",pwr_u8);
	if ((int8_t)pwr_u8 > (int8_t)127) pwr_u8=127;
	set_property((uint8_t)0x22,(uint8_t)SI446X_PROP_GRP_INDEX_PA_PWR_LVL,pwr_u8 );
}

/*
 * =============================
 * RX
 * =============================
 */

uint8_t *Si446x::rx_buffer_u8(){return rx_packet_s.mPacket;};
uint8_t Si446x::rx_size_u8(){return rx_packet_s.bit_size_i16;};

void Si446x::rx_start()
{
	set_pin();
	LOG_I("mChannel(%d) - PACKET_LENGTH(%d)",AIS_CHANNELS[rx_packet_s.mChannel].ordinal,RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH);
	vRadio_StartRX(AIS_CHANNELS[rx_packet_s.mChannel].ordinal,RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH);
}

void Si446x::rx_faststart()
{
	set_pin();
	si446x_start_rx_fast();
}


/*
 * =============================
 * rx_is_packet
 * =============================
 *  -Clear pending IT
 * =============================
 */
bool Si446x::rx_is_packet()
{
	set_pin();
	bool is_packet;
	is_packet= (bRadio_Check_Tx_RX()==SI446X_CMD_GET_INT_STATUS_REP_PH_PEND_PACKET_RX_PEND_BIT);
	return is_packet;
}

/*
 * =============================
 * rx_get_ais_decode
 * =============================
 *  Clear pending IT
 *  Packet from SI446X
 *  	Start after flag 0x7F
 *  	CRC included
 *  	Up to end flag 0x7F included
 *  	Data NRZI encoded +bit stuffing
 *  Get
 * =============================
 */
bool Si446x::rx_get_ais_decode()
{
	set_pin();
	bool is_packet;
	is_packet=false;

	is_packet=rx_get_packet();
	if (is_packet){
		rx_ais_decode(rx_packet_s.mChannel);
	}

	/*
	 * Restart RX listening
	 */
	rx_start();
	return is_packet;
}


bool Si446x::rx_get_packet()
{
	set_pin();
	bool is_packet;
	is_packet=false;

	rx_packet_s.reset();
	// Get data via spi
	if(bRadio_Check_Tx_RX()&SI446X_CMD_GET_INT_STATUS_REP_PH_PEND_PACKET_RX_PEND_BIT)
	{
		is_packet=true;
		rx_packet_ct++;
		// Copy data into rx_packet
		LOG_I("copy_in");
		rx_packet_s.copy_in((uint8_t*)customRadioPacket,RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH);
		//rx_packet_s.print_bytes();
		rx_packet_s.ais_setRSSI(rx_get_rssi());
	}
	return is_packet;
}

void Si446x::rx_fastget_packet(uint8_t *data_pu8)
{
	set_pin();
	uint8_t *dest_pu8=0;
	dest_pu8=(data_pu8==0)?rx_packet_s.mPacket:data_pu8;

	//rx_packet_s.reset();
	si446x_read_rx_fifo(RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH,dest_pu8);
	rx_packet_ct++;
}

void Si446x::rx_clear_ph()
{
	set_pin();
	// Clear pending PH
	si446x_get_ph_status(0);
	LOG_D("PH_PEND=%#02x",Si446xCmd.GET_INT_STATUS.PH_PEND);

}

uint8_t Si446x::rx_get_fifo_count()
{
	set_pin();
	/* Get payload length */
	si446x_fifo_info(0);
	LOG_D("RX_FIFO_COUNT: %d",Si446xCmd.FIFO_INFO.RX_FIFO_COUNT);
	return Si446xCmd.FIFO_INFO.RX_FIFO_COUNT;
}

bool Si446x::rx_is_fifo_packet()
{
	set_pin();
	bool rtn=true;
	/* Get payload length */
	si446x_fifo_info(0);
	LOG_D("RX_FIFO_COUNT=%d",Si446xCmd.FIFO_INFO.RX_FIFO_COUNT);
	if ((Si446xCmd.FIFO_INFO.RX_FIFO_COUNT<RADIO_CONFIGURATION_DATA_RADIO_PACKET_LENGTH)||(Si446xCmd.FIFO_INFO.RX_FIFO_COUNT==0xFF))
	{
		rtn=false;
		//HBL-220422 si446x_fifo_info(SI446X_CMD_FIFO_INFO_ARG_FIFO_RX_BIT);
		si446x_fifo_info(1); // Reset RX FIFO
		LOG_D("rx_is_fifo_packet: reset");
	}
	return rtn;
}


bool Si446x::rx_ais_decode(uint8_t ch_u8)
{
	bool newmsg_ok=false;
	Monitor_data* monit_p=0;
	if (ch_u8==0xFF) ch_u8=rx_packet_s.mChannel;
	rx_packet_s.ais_finalize();

	if (rx_packet_s.checkCRC())
	{
		stats_it_i16[STATS_CRC_OK]++;
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


void Si446x::rx_get_ais_test(uint8_t *data_pu8, int16_t li16)
{
	LOG_I("rx_packet_s.reset");
	rx_packet_s.reset();
	LOG_I("rx_packet_s.copy");
	rx_packet_s.copy_in(data_pu8,(li16+1)>>3);
	rx_ais_decode(rx_packet_s.mChannel);
}


/*
 * ========================
 * get_get_modem_status
 * ========================
 *
 * ========================
 */
void Si446x::rx_get_modem_status(bool clear_it_b)
{
	set_pin();

	si446x_get_modem_status(0);
	printf("get_get_modem_status.MODEM_PEND: %d\n",Si446xCmd.GET_MODEM_STATUS.MODEM_PEND);
	printf("get_get_modem_status.MODEM_STATUS: %d\n",Si446xCmd.GET_MODEM_STATUS.MODEM_STATUS);

	printf("get_get_modem_status.LATCH_RSSI: %d\n", Si446xCmd.GET_MODEM_STATUS.LATCH_RSSI);

	printf("get_get_modem_status.CURR_RSSI: %d\n", Si446xCmd.GET_MODEM_STATUS.CURR_RSSI);
	si446x_get_modem_status(clear_it_b?0xFF:0x00);
}

/*
 * ========================
 * get_rx_rssi
 * ========================
 * The chip provides for returning the Received Signal Strength Indication (RSSI) value
 * through the GET_MODEM_STATUS command.
 * The returned RSSI value is an 8-bit unsigned word whose value is proportional
 * to the strength of the received signal.
 * The RSSI value returned by the GET_MODEM_STATUS command is in increments
 * of approximately 0.5 dB per bit.
 * ========================
 */

int16_t Si446x::rx_get_rssi()
{
	set_pin();
	int16_t rssi_i16;

	// Get data without any IT clearings
	si446x_get_modem_status(0);
	rssi_i16=(int16_t)(Si446xCmd.GET_MODEM_STATUS.LATCH_RSSI>>1);
	LOG_D("get_rx_rssi - LATCH_RSSI: %d dB",rssi_i16);
	return rssi_i16;
}


/*
 * ========================
 * get_status
 * ========================
 * 				7	6	5	                            4	            3	        2	        1	        0
 * CHIP_STATUS	X	CAL	FIFO_UNDERFLOW_OVERFLOW_ERROR	STATE_CHANGE	CMD_ERROR	CHIP_READY	LOW_BATT	WUT
 *
 * CMD_ERROR: Self clearing signal that pulses high to indicate
 * 		an error has occurred in the processing of a command.
 * 		For example, an incorrect command/property ID is sent,
 * 		or an attempt was made to write a property that is outside of the given property group.
 * CHIP_READY: Set to indicate the chip (upon completion of the POWER_UP sequence)
 * 		has reached the state where it is ready to accept commands,
 * 		or when the IR calibration process is completed.
 * 		Cleared when the chip is shutdown or when IR calibration has begun.
 * STATE_CHANGE: Set to indicate a successful transition from one state to another operating state.
 * 		Cleared when a state transition is in progress.
 * Examples:
 *		0x14 -> STATE_CHANGE + CHIP_READY
 * ========================
 */
void Si446x::get_status()
{
	set_pin();

	si446x_get_chip_status(0);
	ais_wifi::wifi_printf("GET_CHIP_STATUS.CHIP_STATUS(%#04x)\n",Si446xCmd.GET_CHIP_STATUS.CHIP_STATUS);
}




/*
 * ========================
 * get_fifos_status
 * ========================
 *  GET_FIFO_STATUS:
 *  	0: No reset
 *  	1: Reset TX
 *  	2: Reset RX
 *  	3: Reset TX&RX
 * ========================
 */
void Si446x::get_fifos_status(uint8_t reset_u8) {
	set_pin();

	si446x_fifo_info(reset_u8);
	LOG_D("RX_FIFO_COUNT(%d)",Si446xCmd.FIFO_INFO.RX_FIFO_COUNT);
	LOG_D("TX_FIFO_SPACE(%d)",Si446xCmd.FIFO_INFO.TX_FIFO_SPACE);
}


/*
 * ========================
 * get_state
 * ========================
 * CURR_STATE:
 *		1: SLEEP (Not Applicable)
 *		2: SPI_ACTIVE state.
 *		3: READY state.
 *		4: Another enumeration for READY state.
 *		5: TX_TUNE state.
 *		6: RX_TUNE state.
 *		7: TX state.
 *		8: RX state.
 * CURRENT_CHANNEL: current tuned channel.
 * Examples:
 *      8 -> Another enumeration for READY state.
 * ========================
 */
uint8_t Si446x::get_state()
{
	set_pin();
	si446x_request_device_state();
	ais_wifi::wifi_printf("state(%d) - channel(%d)\n",Si446xCmd.REQUEST_DEVICE_STATE.CURR_STATE&0xf, Si446xCmd.REQUEST_DEVICE_STATE.CURRENT_CHANNEL);
	return Si446xCmd.REQUEST_DEVICE_STATE.CURR_STATE&0xf;
}


uint16_t Si446x::si446x_get_info(uint8_t print_u8)
{
	set_pin();

	si446x_part_info();
	if(print_u8) LOG_W("%0#4x - %d",Si446xCmd.PART_INFO.PART,Si446xCmd.PART_INFO.ROMID);
	return Si446xCmd.PART_INFO.PART;
}


float Si446x::si446x_get_tp()
{
	set_pin();
	float tp_f,tp_adc;

	si446x_get_adc_reading(SI446X_CMD_GET_ADC_READING_ARG_ADC_EN_TEMPERATURE_EN_BIT);
	tp_adc=(float)Si446xCmd.GET_ADC_READING.TEMP_ADC;

	tp_f=tp_adc-273.0;
	LOG_I("°C: %f",tp_f);
	return tp_f;
}

float Si446x::si446x_get_voltage()
{
	set_pin();
	float bat_f;

	// 3*BATTERY_ADC/1280
	si446x_get_adc_reading(SI446X_CMD_GET_ADC_READING_ARG_ADC_EN_BATTERY_VOLTAGE_EN_BIT);
	bat_f=3.0*Si446xCmd.GET_ADC_READING.BATTERY_ADC/1280.0;

	LOG_I("V: %f",bat_f);
	return bat_f;
}

} //SI4463




