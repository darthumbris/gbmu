#include "AudioProcessingUnit.hpp"
#include "MemoryMap.hpp"
#include "SDL_error.h"
#include <SDL.h>
#include <SDL2/SDL_mutex.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL_audio.h>
#include <SDL_mixer.h>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

AudioProcessingUnit::AudioProcessingUnit() : apu(gbapu::Apu(SAMPLERATE, 4800)) {
	sample_rate = SAMPLERATE;

	bufs = NULL;
	std::string platform = SDL_GetPlatform();
	printf("platform: %s\n", platform.c_str());
	if ((platform == "Linux")) {
		SDL_InitSubSystem(SDL_INIT_AUDIO);
		SDL_AudioInit("alsa");
	} else {
		SDL_Init(SDL_INIT_AUDIO);
	}
}

AudioProcessingUnit::~AudioProcessingUnit() {
	SDL_PauseAudio(true);
	SDL_CloseAudio();
	if (free_sem) {
		SDL_DestroySemaphore(free_sem);
		free_sem = NULL;
	}
	delete [] bufs;
	bufs = NULL;
}

void AudioProcessingUnit::init() {
	assert( !bufs );

	bufs = new sample_t [(long) SAMPLES * BUF_COUNT];
	if ( !bufs ) {
		std::cerr << "Out of Memory" << std::endl;
		exit(EXIT_FAILURE);
	}

	for (long l = 0; l < ((long) SAMPLES * BUF_COUNT); l++)
		bufs[0] = 0;

	write_buf = 0;
	write_pos = 0;
	read_buf = 0;
	free_sem = SDL_CreateSemaphore(BUF_COUNT - 1);
	std::cout << "sem_value start: " << SDL_SemValue(free_sem) << std::endl << std::flush;
	if (!free_sem) {
		printf("Couldn't create semaphore! SDL_Error: %s\n", SDL_GetError());
		return;
	}

	SDL_AudioSpec want;
	want.freq = SAMPLERATE;  // number of samples per second
	want.format = AUDIO_F32; // sample type
	want.channels = CHANNELS;
	want.samples = SAMPLES / CHANNELS; // buffer-size
	want.size = 0;
	want.silence = 0;
	want.callback = audio_callback_; // function SDL calls periodically to refill the buffer
	want.userdata = this;

	SDL_AudioSpec have;
	if (SDL_OpenAudio(&want, &have) != 0)
		SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to open audio: %s", SDL_GetError());
	if (want.format != have.format)
		SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to get the desired AudioSpec");

	SDL_PauseAudio(false); // start playing sound

	for (int reg = 0xFF10; reg <= 0xFF3F; reg++) {
		apu.writeRegister(reg - 0xFF00, initial_io_values_cgb[reg - 0xFF00], 0);
	}
	sample_count = 0;
}

void AudioProcessingUnit::audio_callback_(void *user_data, uint8_t *stream, int len) {
	((AudioProcessingUnit *)user_data)->audio_callback(stream, len);
}

inline float *AudioProcessingUnit::buf(int index) {
	assert((unsigned)index < BUF_COUNT);
	return bufs + (long)index * SAMPLES;
}

void AudioProcessingUnit::audio_callback(uint8_t *stream, int len) {
	if (SDL_SemValue(free_sem) < BUF_COUNT - 1) {
		std::memcpy(stream, buf(read_buf), len);
		read_buf = (read_buf + 1) % BUF_COUNT;
		SDL_SemPost(free_sem);
	} else {
		memset(stream, 0, len);
	}
}

void AudioProcessingUnit::set_volume(float volume) {
	apu.setVolume(volume);
}

void AudioProcessingUnit::tick(uint16_t cycle) {
	elapsed_cycles += cycle;
}

void AudioProcessingUnit::end_frame() {
	step(elapsed_cycles);
	apu.endFrame();
	size_t samples = apu.availableSamples();
	if (samples > SAMPLES) {
		samples = SAMPLES;
	}
	// assert(samples <= SAMPLES);
	sample_count = samples * 2;
	apu.readSamples(audio_buffer, samples);
}

void AudioProcessingUnit::clear_samples() {
	apu.clearSamples();
}

void AudioProcessingUnit::step(uint32_t cycles) {
	apu.step(cycles);
	elapsed_cycles = 0;
}

void AudioProcessingUnit::write() {
	if (sample_count > 0) {
		size_t count = sample_count;
		sample_t *in = audio_buffer;
		while (count) {
			int n = SAMPLES - write_pos;
			if (n > count) {
				n = count;
			}

			memcpy(buf(write_buf) + write_pos, in, n * sizeof(sample_t));
			in += n;
			write_pos += n;
			count -= n;

			if (write_pos >= SAMPLES) {
				write_pos = 0;
				write_buf = (write_buf + 1) % BUF_COUNT;

				SDL_SemWait(free_sem);
			}
		}
		sample_count = 0;
	}
}

uint8_t AudioProcessingUnit::read_u8(uint16_t addr) {
	//TODO check if the divide by 4 is needed?
	step(elapsed_cycles);
	return apu.readRegister(addr - 0xFF00, 0);
}

void AudioProcessingUnit::write_u8(uint16_t addr, uint8_t val) {
	//TODO check if the divide by 4 is needed?
	step(elapsed_cycles);
	apu.writeRegister(addr - 0xFF00, val, 0);
}
