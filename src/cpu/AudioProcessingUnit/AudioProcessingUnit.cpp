#include "AudioProcessingUnit/AudioProcessingUnit.hpp"
#include "MemoryMap.hpp"
#include "SDL2/SDL_error.h"
#include "debug.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mutex.h>
#include <SDL2/SDL_stdinc.h>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

AudioProcessingUnit::AudioProcessingUnit() : apu(nullptr), stereo_buffer(nullptr) {
	sample_rate = SAMPLERATE;

	bufs = nullptr;
	std::string platform = SDL_GetPlatform();
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
		free_sem = nullptr;
	}
	delete[] bufs;
	bufs = nullptr;
	delete apu;
	delete[] sample_buffer;
	delete stereo_buffer;
}

void AudioProcessingUnit::init() {
	assert(!bufs);

	bufs = new sample_t[(long)SAMPLES * BUF_COUNT];
	if (!bufs) {
		std::cerr << "Out of Memory" << std::endl;
		exit(EXIT_FAILURE);
	}

	for (long l = 0; l < ((long)SAMPLES * BUF_COUNT); l++)
		bufs[0] = 0;

	write_buf = 0;
	write_pos = 0;
	read_buf = 0;
	free_sem = SDL_CreateSemaphore(BUF_COUNT - 1);
	if (!free_sem) {
		printf("Couldn't create semaphore! SDL_Error: %s\n", SDL_GetError());
		return;
	}

	SDL_AudioSpec want;
	want.freq = SAMPLERATE;     // number of samples per second
	want.format = AUDIO_S16SYS; // sample type
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

	// audio_buffer = new sample_t[AUDIO_BUFFER_SIZE];
	sample_buffer = new sample_t[AUDIO_BUFFER_SIZE];
	apu = new Gb_Apu();
	stereo_buffer = new Stereo_Buffer();

	stereo_buffer->clock_rate(CLOCKRATE);
	stereo_buffer->set_sample_rate(SAMPLERATE);

	apu->set_output(stereo_buffer->center(), stereo_buffer->left(), stereo_buffer->right());

	Gb_Apu::mode_t mode = is_cgb ? Gb_Apu::mode_cgb : Gb_Apu::mode_dmg;
	apu->reset(mode);
	stereo_buffer->clear();

	for (int addr = 0xFF10; addr <= 0xFF3F; addr++) {
		uint8_t value = is_cgb ? initial_io_values_cgb[addr - 0xFF00] : initial_io_values_dmg[addr - 0xFF00];
		apu->write_register(0, addr, value);
	}
	apu->volume((1.0f));
	sample_count = 0;
}

void AudioProcessingUnit::audio_callback_(void *user_data, uint8_t *stream, int len) {
	((AudioProcessingUnit *)user_data)->audio_callback(stream, len);
}

inline AudioProcessingUnit::sample_t *AudioProcessingUnit::buf(int index) {
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
	apu->volume(volume);
}

void AudioProcessingUnit::tick(uint16_t cycle) {
	elapsed_cycles += cycle;
}

void AudioProcessingUnit::end_frame() {
	DEBUG_MSG("elapsed_cycles: %u\n", elapsed_cycles);
	apu->end_frame(elapsed_cycles);
	stereo_buffer->end_frame(elapsed_cycles);

	int32_t samples = static_cast<int>(stereo_buffer->read_samples(sample_buffer, AUDIO_BUFFER_SIZE));

	for (int i = 0; i < samples; i++) {
		audio_buffer[i] = sample_buffer[i];
	}
	sample_count = samples;
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
	uint8_t val = apu->read_register(elapsed_cycles, addr);
	DEBUG_MSG("apu: %u addr: %#06X elapsed_cycles: %u\n", val, addr, elapsed_cycles);
	return val;
}

void AudioProcessingUnit::write_u8(uint16_t addr, uint8_t val) {
	DEBUG_MSG("write apu: %u addr: %#06X\n", val, addr);
	apu->write_register(elapsed_cycles, addr, val);
}
