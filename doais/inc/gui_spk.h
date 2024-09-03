/**
 * ********************
 *  gui_spk.h
 *  www.DoNovae.com
 *  Herve Bailly
 * *******************
 */

//#include <M5Unified.h>
//#include <M5GFX.h>
#ifndef __GUI_SPK_H
#define __GUI_SPK_H


/*
 * =======================
 * Defines
 * =======================
 */
#define GUI_SPK_WAVE_TONE_LEN (6000)
#define GUI_SPK_WAVE_CLICK_LEN 112
#define GUI_SPK_RATE 16000
#define GUI_SPK_WAVE_FIVE_MN_LEN 13426
#define GUI_SPK_WAVE_TEN_MN_LEN 11880
#define GUI_SPK_WAVE_FIFTEEN_MN_LEN 17050
#define GUI_SPK_WAVE_TWENTY_MN_LEN 15214
#define GUI_SPK_WAVE_THIRTY_MN_LEN 9550

typedef enum {
	GUI_SPK_WAV_U8=0,
	GUI_SPK_WAV_I16=1
}gui_spk_wav_e;

/*
 * =======================
 * Types
 * =======================
 */
struct sound_param_t
{
//	xTaskHandle handle = nullptr;
//	const uint8_t* data = nullptr;
	size_t len = 0;
	size_t rate = 0;
	uint8_t level_u8;
};

/*
 * =======================
 * Extern speaker
 * =======================
 */
extern const uint8_t gui_spk_click_wav[];
extern const uint8_t five_minutes_wav[];
extern const uint8_t ten_minutes_wav[];
extern const uint8_t fifteen_minutes_wav[];
extern const uint8_t twenty_minutes_wav[];
extern const uint8_t thirty_minutes_wav[];

/*
 * =======================
 * Globals
 * =======================
 */
class Gui_spk{
public:
	Gui_spk();
	~Gui_spk();
	void begin();
	void play_click();
	void play_tone2k();
	void play_five();
	void play_ten();
	void play_fifteen();
	void play_twenty();
	void play_thirty();
	static uint8_t beep_level_u8;
private:
	static uint8_t wave_tone_wav[GUI_SPK_WAVE_TONE_LEN];
	sound_param_t sound_param_s;
	// static void IRAM_ATTR sound_task(void* sound_param);
	static void sound_play(sound_param_t* sound_param,gui_spk_wav_e wav_s);
	static void set_spk(int sampleRate);
	static void set_tone2k();
};

#endif //__GUI_SPK_H




