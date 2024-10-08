#ifndef AUDIOPROCESSINGUNIT_HPP
#define AUDIOPROCESSINGUNIT_HPP

#include "AudioProcessingUnit/Gb_Apu.h"
#include "AudioProcessingUnit/Multi_Buffer.h"
#include "SDL2/SDL_mutex.h"
#include <fstream>

// Using gb apu from: https://github.com/stoneface86/gbapu

constexpr unsigned SAMPLERATE = 44100;
constexpr long CLOCKRATE = 4194304;
constexpr uint16_t SAMPLES = 2048;
constexpr uint16_t CHANNELS = 2;
constexpr uint16_t BUF_COUNT = 3;
constexpr uint16_t AUDIO_BUFFER_SIZE = SAMPLES * 2;

// https://gbdev.io/pandocs/Audio_Registers.html#audio-registers

class AudioProcessingUnit {
private:
	typedef int16_t sample_t;

	Gb_Apu *apu;
	Stereo_Buffer *stereo_buffer;
	sample_t audio_buffer[AUDIO_BUFFER_SIZE];
	sample_t *sample_buffer;
	sample_t *volatile bufs;
	double sample_rate;
	SDL_sem *free_sem;
	volatile int read_buf;
	int32_t sample_count;
	int write_buf;
	int write_pos;
	int32_t elapsed_cycles = 0;

public:
	AudioProcessingUnit();
	~AudioProcessingUnit();

	void init(bool cgb_mode);
	void set_volume(float volume);
	void tick(uint16_t cycle);
	uint8_t read_u8(uint16_t addr);
	void write_u8(uint16_t addr, uint8_t val);
	void end_frame();
	static void audio_callback_(void *user_data, uint8_t *stream, int len);
	void audio_callback(uint8_t *stream, int len);
	sample_t *buf(int index);
	void write();

	void reset(bool cgb_mode);

	void serialize(std::ofstream &f);
	void deserialize(std::ifstream &f, bool cgb_mode);
};

#endif