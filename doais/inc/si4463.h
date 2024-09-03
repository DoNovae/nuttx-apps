/**
 * ********************
 * si446x1.h
 *  www.DoNovae.com
 *  Herve Bailly
 * *******************
 */
#ifndef HARDWARE_H1
#define HARDWARE_H1

//#include <Wire.h>
//#include <SPI.h>
//#include <driver/gpio.h>

#include "tx_packet.h"
#include "rx_packet.h"
#include "ais.h"
#include "ais_messages.h"

namespace SI4463
{

/*
 * Statistics IT
 */
typedef enum {
	STATS_CPT=0,
	CHIP_CMD_ERROR,
	MODEM_PREAMBLE_DETECT,
	MODEM_INVALID_PREAMBLE,
	MODEM_SYNC_DETECT,
	MODEM_INVALID_SYNC,
	PH_CRC_ERROR,
	PH_PACKET_SENT,
	PH_PACKET_RX,
	STATS_CRC_OK
}statistics_it_e;

#define HARDWARE_STATS_LEN 10



/*
 * ========================
 *  Si446x
 * ========================
 */
class Si446x {
private:
	int8_t _nIRQPin;
	int8_t _sdnPin;
	int8_t _nSELPin;
	// TODO SPIClass * _VSPI2;

private:
 void set_pin();
 static int16_t stats_it_i16[HARDWARE_STATS_LEN];

public:
	TXPacket tx_packet_s;
	RXPacket rx_packet_s;
    int32_t rx_packet_ct;
    int32_t tx_packet_ct;

	Si446x(int8_t nIRQPin,int8_t sdnPin,int8_t nSELPin);
	~Si446x();
	void init();
	void set_station_data(StationData * data_ps);

	inline int8_t get_nIRQPin(){return _nIRQPin;};
	void radio_init();
	float si446x_get_voltage();
	float si446x_get_tp();
	uint16_t si446x_get_info(uint8_t print_u8);
	void get_status();
	void get_fifos_status(uint8_t reset_u8);
	uint8_t get_state();
	void set_property(uint8_t group_u8, uint8_t num_prop_u8, uint8_t prop_u8);
	void version();
	void stats_init();
	void stats_do();
	void stats_printf();
	void check();
	inline void shutdown(){/*TODO dacWrite(_sdnPin,255);delay(200);*/};
	inline void switchon(){/*TODO dacWrite(_sdnPin,0);delay(200);*/};


	// TX
	void tx_send_ais_packet();
	void tx_build_ais_packet(AISMessage *msg);
	void tx_set_power(uint8_t pwr_u8);
	uint8_t *tx_buffer_u8();
	int16_t tx_size_i16();

	// RX
	bool rx_get_ais_decode();
	bool rx_get_packet();
	void rx_fastget_packet(uint8_t *data_pu8);
	uint8_t rx_get_fifo_count();
	void rx_clear_ph();
	bool rx_is_fifo_packet();
	void rx_get_ais_test(uint8_t *data_pu8, int16_t li16);
	bool rx_ais_decode(uint8_t ch_u8);
	bool rx_is_packet();
	void rx_start();
	void rx_faststart();
	void rx_get_modem_status(bool clear_it_b);
	int16_t rx_get_rssi();
	uint8_t *rx_buffer_u8();
	uint8_t rx_size_u8();
};





/*
 * ========================
 *  EXTERN
 * ========================
 */
extern Si446x si446x1_;
extern Si446x si446x2_;
} //SI4463


#endif // HARDWARE_H
