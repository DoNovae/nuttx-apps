/**
 * =====================================
 * gui_spk.cpp
 * -------------------------------------
 *  AIS DoNovae
 *  www.DoNovae.com
 *  Herve Bailly
 * =====================================
 */
#include <SPI.h>
#include <esp_log.h>
// #include <FreeRTOS.h>
#include <driver/i2s.h>
#include "gui_spk.h"



/*
 * --------------------
 * I2S
 * --------------------
 */
#define CONFIG_I2S_BCK_PIN 12
#define CONFIG_I2S_LRCK_PIN 0
#define CONFIG_I2S_DATA_PIN 2
#define CONFIG_I2S_DATA_IN_PIN 34

#define Speak_I2S_NUMBER I2S_NUM_0
#define MODE_MIC 0
#define MODE_SPK 1
#define I2S_DATA_LEN 60

/*
 * =======================
 * Globals
 * =======================
 */



/*
 * --------------------
 * I2S
 * --------------------
 */
#define CONFIG_I2S_BCK_PIN 12
#define CONFIG_I2S_LRCK_PIN 0
#define CONFIG_I2S_DATA_PIN 2
#define CONFIG_I2S_DATA_IN_PIN 34

#define Speak_I2S_NUMBER I2S_NUM_0
#define MODE_MIC 0
#define MODE_SPK 1
#define I2S_DATA_LEN 60


/*
 * =======================
 * Classes
 * =======================
 */

/*
 * =======================
 * Gui_spk
 * -----------------------
 */
uint8_t Gui_spk::beep_level_u8=1;

Gui_spk::Gui_spk(){
};

Gui_spk::~Gui_spk(){};

uint8_t Gui_spk::wave_tone_wav[GUI_SPK_WAVE_TONE_LEN];

/*
 * -----------------------
 * begin
 * -----------------------
 */
void Gui_spk::begin(){
	/*
	 * Tone 2k
	 */
	beep_level_u8=1;
	set_tone2k();

	// HBL 120622
	Gui_spk::set_spk(GUI_SPK_RATE);
	M5.Power.Axp192.bitOn(0x94, 0x04); // speaker on

	/*
	 * Set speaker handler
	 */
	//xTaskCreatePinnedToCore(Gui_spk::sound_task, "soundTask", 4096, &sound_param_s,0,&sound_param_s.handle, 0);
}

/*
 * -----------------------
 * set_spk
 * -----------------------
 */
void Gui_spk::set_spk(int sampleRate=GUI_SPK_RATE)
{
	/*
	 * I2S driver
	 */
	i2s_driver_uninstall(Speak_I2S_NUMBER);

	i2s_config_t i2s_config = {
			.mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
			.sample_rate          = (uint32_t)sampleRate,
			.bits_per_sample      = I2S_BITS_PER_SAMPLE_16BIT,
			.channel_format       = I2S_CHANNEL_FMT_ONLY_RIGHT,
			//.communication_format = I2S_COMM_FORMAT_I2S,
			.communication_format = I2S_COMM_FORMAT_STAND_I2S,
			.intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
			.dma_buf_count        = 2,
			//.dma_buf_len          = I2S_DATA_LEN,
			.dma_buf_len          = 128,
			.use_apll             = false,
			.tx_desc_auto_clear   = true,
			.fixed_mclk           = 0
	};

	auto res = i2s_driver_install(Speak_I2S_NUMBER, &i2s_config, 0, nullptr);
	ESP_LOGI("main", "i2s_driver_install:%d", res);

	i2s_pin_config_t tx_pin_config = {
			.bck_io_num     = CONFIG_I2S_BCK_PIN,
			.ws_io_num      = CONFIG_I2S_LRCK_PIN,
			.data_out_num   = CONFIG_I2S_DATA_PIN,
			.data_in_num    = CONFIG_I2S_DATA_IN_PIN,
	};
	res = i2s_set_pin(Speak_I2S_NUMBER, &tx_pin_config);
	res = i2s_set_clk(Speak_I2S_NUMBER, sampleRate, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
	res = i2s_zero_dma_buffer(Speak_I2S_NUMBER);
}

/*
 * -----------------------
 * sound_task
 * -----------------------
 */
void IRAM_ATTR Gui_spk::sound_task(void* sound_p)
{
	sound_param_t *param_ps=(sound_param_t*)sound_p;
	param_ps->rate = GUI_SPK_RATE;
	//int prevSampleRate = 0;
	// I2S
	int16_t data_i16[I2S_DATA_LEN];
	for (;;)
	{
		/*
		if (prevSampleRate!=param_ps->rate)
		{
			prevSampleRate = param_ps->rate;
			Gui_spk::set_spk(param_ps->rate);
			M5.Power.Axp192.bitOn(0x94, 0x04); // speaker on
		}*/

		// Write Speaker
		size_t bytes_written = 0;

		int index = 0;

		/*
		 * Reset I2S data_i16 and send data_i16
		 */
		memset(data_i16,0,I2S_DATA_LEN*2);
		for (int i = 0; i < 2; ++i)
		{
			i2s_write(Speak_I2S_NUMBER, data_i16, I2S_DATA_LEN*2, &bytes_written, portMAX_DELAY);
		}

		for (int i=0;i<param_ps->len;i++)
		{
			int16_t val_i16=(int16_t)param_ps->data[i];
			data_i16[index]=((val_i16-(int16_t)128)<<6)*param_ps->level_u8;
			index+= 1;
			/*
			 * Send data_i16
			 */
			if (I2S_DATA_LEN<=index)
			{
				index=0;
				i2s_write(Speak_I2S_NUMBER,data_i16, I2S_DATA_LEN*2, &bytes_written, portMAX_DELAY);
			}
		}
		/*
		 * Send remaining data in data_i16
		 */
		memset(&data_i16[index],0,(I2S_DATA_LEN-index)<<1);
		i2s_write(Speak_I2S_NUMBER,data_i16,I2S_DATA_LEN*2, &bytes_written, portMAX_DELAY);
		/*
		 * Reset data_i16 completely and send data_i16 4 times
		 */
		if (index<=I2S_DATA_LEN)
		{
			memset(data_i16,0,index<<1);
		}

		for (int i=0;i<4; ++i)
		{
			i2s_write(Speak_I2S_NUMBER,data_i16, I2S_DATA_LEN*2, &bytes_written, portMAX_DELAY);
		}
		ulTaskNotifyTake( pdTRUE, portMAX_DELAY );
	}
}


void Gui_spk::sound_play(sound_param_t* param_ps,gui_spk_wav_e wav_s)
{
	// Write Speaker
	size_t bytes_written = 0;
	int16_t data_i16[I2S_DATA_LEN];

	int16_t index=0,idx_i16=0;

	/*
	 * Reset I2S data_i16 and send data_i16
	 */
	memset(data_i16,0,I2S_DATA_LEN*2);
	for (int i = 0; i < 2; ++i)
	{
		i2s_write(Speak_I2S_NUMBER, data_i16, I2S_DATA_LEN*2, &bytes_written, portMAX_DELAY);
	}

	index=0;
	idx_i16=0;
	while (idx_i16<param_ps->len)
	{
		if (wav_s==GUI_SPK_WAV_U8)
		{
			data_i16[index]=(((int16_t)param_ps->data[idx_i16]-(int16_t)128)<<6)*param_ps->level_u8;
			idx_i16+=1;
		} else
		{
			data_i16[index]=(((int16_t)param_ps->data[idx_i16+1]<<8)|(int16_t)param_ps->data[idx_i16])*param_ps->level_u8;
			idx_i16+=2;
		}
		index+=1;
		/*
		 * Send data_i16
		 */
		if (I2S_DATA_LEN<=index)
		{
			index=0;
			i2s_write(Speak_I2S_NUMBER,data_i16, I2S_DATA_LEN*2, &bytes_written, portMAX_DELAY);
		}
	}
	/*
	 * Send remaining data in data_i16
	 */
	memset(&data_i16[index],0,(I2S_DATA_LEN-index)<<1);
	i2s_write(Speak_I2S_NUMBER,data_i16,I2S_DATA_LEN*2, &bytes_written, portMAX_DELAY);
	/*
	 * Reset data_i16 completely and send data_i16 4 times
	 */
	if (index<=I2S_DATA_LEN)
	{
		memset(data_i16,0,index<<1);
	}

	for (int i=0;i<4; ++i)
	{
		i2s_write(Speak_I2S_NUMBER,data_i16, I2S_DATA_LEN*2, &bytes_written, portMAX_DELAY);
	}
	//i2s_write(Speak_I2S_NUMBER,param_ps->data,param_ps->len,&bytes_written,portMAX_DELAY);
}

/*
 * ----------------------
 * play_five_mn
 * ----------------------
 */
void Gui_spk::play_five()
{
	sound_param_s.data = (uint8_t*)five_minutes_wav;
	sound_param_s.len  = GUI_SPK_WAVE_FIVE_MN_LEN;
	sound_param_s.rate = GUI_SPK_RATE;
	sound_param_s.level_u8=beep_level_u8;
	sound_play(&sound_param_s,GUI_SPK_WAV_I16);
}

void Gui_spk::play_ten()
{
	sound_param_s.data = (uint8_t*)ten_minutes_wav;
	sound_param_s.len  = GUI_SPK_WAVE_TEN_MN_LEN;
	sound_param_s.rate = GUI_SPK_RATE;
	sound_param_s.level_u8=beep_level_u8;
	sound_play(&sound_param_s,GUI_SPK_WAV_I16);
}

void Gui_spk::play_fifteen()
{
	sound_param_s.data = (uint8_t*)fifteen_minutes_wav;
	sound_param_s.len  = GUI_SPK_WAVE_FIFTEEN_MN_LEN;
	sound_param_s.rate = GUI_SPK_RATE;
	sound_param_s.level_u8=beep_level_u8;
	sound_play(&sound_param_s,GUI_SPK_WAV_I16);
}

void Gui_spk::play_twenty()
{
	sound_param_s.data = (uint8_t*)twenty_minutes_wav;
	sound_param_s.len  = GUI_SPK_WAVE_TWENTY_MN_LEN;
	sound_param_s.rate = GUI_SPK_RATE;
	sound_param_s.level_u8=beep_level_u8;
	sound_play(&sound_param_s,GUI_SPK_WAV_I16);
}

void Gui_spk::play_thirty()
{
	sound_param_s.data = (uint8_t*)thirty_minutes_wav;
	sound_param_s.len  = GUI_SPK_WAVE_THIRTY_MN_LEN;
	sound_param_s.rate = GUI_SPK_RATE;
	sound_param_s.level_u8=beep_level_u8;
	sound_play(&sound_param_s,GUI_SPK_WAV_I16);
}



/*
 * ----------------------
 * play_tone2k
 * ----------------------
 */
void Gui_spk::play_tone2k()
{
	sound_param_s.data=wave_tone_wav;
	sound_param_s.len=GUI_SPK_WAVE_TONE_LEN;
	sound_param_s.rate=GUI_SPK_RATE;
	sound_param_s.level_u8=beep_level_u8;
	//xTaskNotifyGive(sound_param_s.handle);
	sound_play(&sound_param_s,GUI_SPK_WAV_U8);
}

/*
 * ----------------------
 * set_tone2k
 * ----------------------
 */
void Gui_spk::set_tone2k()
{
	for (int i=0;i<GUI_SPK_WAVE_TONE_LEN;i++){
		wave_tone_wav[i]=(uint16_t)(sin(2*PI*(float)i/(float)16.0)*(float)128.0+(float)128.0);
	}
}

/*
 * ----------------------
 * play_click
 * ----------------------
 */
void Gui_spk::play_click()
{
	sound_param_s.data = gui_spk_click_wav;
	sound_param_s.len  = GUI_SPK_WAVE_CLICK_LEN;
	sound_param_s.rate = GUI_SPK_RATE;
	sound_param_s.level_u8=1;
	//xTaskNotifyGive(sound_param_s.handle);
	sound_play(&sound_param_s,GUI_SPK_WAV_U8);
}





