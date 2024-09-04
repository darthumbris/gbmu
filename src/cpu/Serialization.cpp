#include "Cpu.hpp"
#include "Interruptor.hpp"
#include "debug.hpp"
#include "rom/MCB1.hpp"
#include "rom/MCB1M.hpp"
#include "rom/MCB2.hpp"
#include "rom/MCB3.hpp"
#include "rom/MCB5.hpp"
#include "rom/Rom.hpp"
#include "rom/RomOnly.hpp"

#define SERIALIZE(f, x) (f.write(reinterpret_cast<const char *>(&x), sizeof(x)))
#define DESERIALIZE(f, x) (f.read(reinterpret_cast<char *>(&x), sizeof(x)))

void Cpu::serialize(const std::string &file) {
	char *path = SDL_GetPrefPath("GBMU-42", "gbmu");
	std::string full_path = path + file;
	SDL_free(static_cast<void *>(path));
	std::ofstream f(full_path, std::ios::binary);
	if (!f.is_open()) {
		ERROR_MSG("Error: Failed to open file: %s for serialization.\n", full_path.c_str());
		return;
	}

	DEBUG_MSG("writing save state to: %s\n", full_path.c_str());
	SERIALIZE(f, u8_registers);
	SERIALIZE(f, sp);
	SERIALIZE(f, pc);
	SERIALIZE(f, halted);
	SERIALIZE(f, locked);
	SERIALIZE(f, m_cycle);
	SERIALIZE(f, t_cycle);
	SERIALIZE(f, halt_cycle);
	SERIALIZE(f, opcode);
	SERIALIZE(f, accurate_opcode_state);
	SERIALIZE(f, read_cache);
	SERIALIZE(f, branched);
	SERIALIZE(f, instruction);
	SERIALIZE(f, cgb_speed);
	SERIALIZE(f, speed_multiplier);
	SERIALIZE(f, debug_count);
	interruptor.serialize(f);
	mmap.serialize(f);
	ppu.serialize(f);
	apu.serialize(f);
	f.close();
	DEBUG_MSG("done serializing\n");
}

void Cpu::deserialize(const std::string &file) {
	char *path = SDL_GetPrefPath("GBMU-42", "gbmu");
	std::string full_path = path + file;
	SDL_free(static_cast<void *>(path));
	std::ifstream f(full_path, std::ios::binary);

	if (!f.is_open()) {
		ERROR_MSG("Error: Failed to open file: %s for deserialization.\n", full_path.c_str());
		return;
	}
	reset();

	DEBUG_MSG("loading save state from: %s\n", full_path.c_str());
	DESERIALIZE(f, u8_registers);
	DESERIALIZE(f, sp);
	DESERIALIZE(f, pc);
	DESERIALIZE(f, halted);
	DESERIALIZE(f, locked);
	DESERIALIZE(f, m_cycle);
	DESERIALIZE(f, t_cycle);
	DESERIALIZE(f, halt_cycle);
	DESERIALIZE(f, opcode);
	DESERIALIZE(f, accurate_opcode_state);
	DESERIALIZE(f, read_cache);
	DESERIALIZE(f, branched);
	DESERIALIZE(f, instruction);
	DESERIALIZE(f, cgb_speed);
	DESERIALIZE(f, speed_multiplier);
	DESERIALIZE(f, debug_count);
	interruptor.deserialize(f);
	mmap.deserialize(f);
	ppu.deserialize(f);
	apu.deserialize(f, mmap.is_cgb_rom());
	f.close();
	DEBUG_MSG("done deserializing\n");
}

void Interruptor::serialize(std::ofstream &f) {
	SERIALIZE(f, interrupt_enable_register);
	SERIALIZE(f, interrupt);
	SERIALIZE(f, process_interrupts);
	SERIALIZE(f, interrupt_occured);
	SERIALIZE(f, ime_cycles);
	SERIALIZE(f, delay_cycles);
	SERIALIZE(f, halt_bug_triggered);
	SERIALIZE(f, serial_transfer_data);
	SERIALIZE(f, serial_transfer_control);
	SERIALIZE(f, timer_divider);
	SERIALIZE(f, timer_counter);
	SERIALIZE(f, timer_modulo);
	SERIALIZE(f, timer_enable);
	SERIALIZE(f, timer_clock_select);
	SERIALIZE(f, div_cycle);
	SERIALIZE(f, tima_cycle);
	SERIALIZE(f, serial_cycle);
	SERIALIZE(f, serial_count);
	SERIALIZE(f, input_cycles);
}
void Interruptor::deserialize(std::ifstream &f) {
	DESERIALIZE(f, interrupt_enable_register);
	DESERIALIZE(f, interrupt);
	DESERIALIZE(f, process_interrupts);
	DESERIALIZE(f, interrupt_occured);
	DESERIALIZE(f, ime_cycles);
	DESERIALIZE(f, delay_cycles);
	DESERIALIZE(f, halt_bug_triggered);
	DESERIALIZE(f, serial_transfer_data);
	DESERIALIZE(f, serial_transfer_control);
	DESERIALIZE(f, timer_divider);
	DESERIALIZE(f, timer_counter);
	DESERIALIZE(f, timer_modulo);
	DESERIALIZE(f, timer_enable);
	DESERIALIZE(f, timer_clock_select);
	DESERIALIZE(f, div_cycle);
	DESERIALIZE(f, tima_cycle);
	DESERIALIZE(f, serial_cycle);
	DESERIALIZE(f, serial_count);
	DESERIALIZE(f, input_cycles);
}

void MemoryMap::serialize(std::ofstream &f) {
	SERIALIZE(f, work_ram);
	SERIALIZE(f, echo_ram);
	SERIALIZE(f, not_usable);
	SERIALIZE(f, io_registers);
	SERIALIZE(f, high_ram);
	SERIALIZE(f, interrupt);
	SERIALIZE(f, joypad_register);
	SERIALIZE(f, joypad_pressed);
	SERIALIZE(f, boot_rom_loaded);
	SERIALIZE(f, gb_boot_rom);
	SERIALIZE(f, cgb_boot_rom);
	DEBUG_MSG("done serializing mmap\n");
	rom->serialize(f);
}

void MemoryMap::deserialize(std::ifstream &f) {
	DESERIALIZE(f, work_ram);
	DESERIALIZE(f, echo_ram);
	DESERIALIZE(f, not_usable);
	DESERIALIZE(f, io_registers);
	DESERIALIZE(f, high_ram);
	DESERIALIZE(f, interrupt);
	DESERIALIZE(f, joypad_register);
	DESERIALIZE(f, joypad_pressed);
	DESERIALIZE(f, boot_rom_loaded);
	DESERIALIZE(f, gb_boot_rom);
	DESERIALIZE(f, cgb_boot_rom);
	DEBUG_MSG("done deserializing mmap\n");
	rom->deserialize(f);
}

void PixelProcessingUnit::serialize(std::ofstream &f) {
	SERIALIZE(f, lcd_clock);
	SERIALIZE(f, lcd_clock_vblank);
	SERIALIZE(f, ctrl);
	SERIALIZE(f, l_status);
	SERIALIZE(f, scy);
	SERIALIZE(f, scx);
	SERIALIZE(f, ly);
	SERIALIZE(f, lyc);
	SERIALIZE(f, vblank_line);
	SERIALIZE(f, dma);
	SERIALIZE(f, bg_palette);
	SERIALIZE(f, obj_palette_0);
	SERIALIZE(f, obj_palette_1);
	SERIALIZE(f, bg_palette_cgb);
	SERIALIZE(f, bg_color_cgb);
	SERIALIZE(f, obj_palette_cgb);
	SERIALIZE(f, obj_color_cgb);
	SERIALIZE(f, window_y);
	SERIALIZE(f, window_x);
	SERIALIZE(f, vbank_select);
	SERIALIZE(f, wram_bank_select);
	SERIALIZE(f, interrupt_signal);
	SERIALIZE(f, hdma_source);
	SERIALIZE(f, hdma_dest);
	SERIALIZE(f, hdma);
	SERIALIZE(f, hdma_enable);
	SERIALIZE(f, hdma_bytes);
	SERIALIZE(f, lcd_enabled);
	SERIALIZE(f, window_line_active);
	SERIALIZE(f, draw_screen);
	SERIALIZE(f, drawn_scanline);
	SERIALIZE(f, pixels_drawn);
	SERIALIZE(f, tile_drawn);
	SERIALIZE(f, screen_off_cycles);
	SERIALIZE(f, hide_screen);
	SERIALIZE(f, rgb555_framebuffer);
	SERIALIZE(f, mono_framebuffer);
	SERIALIZE(f, vram);
	SERIALIZE(f, sprite_cache_buffer);
	SERIALIZE(f, color_cache_buffer);
	SERIALIZE(f, sprites);
	SERIALIZE(f, tile_data);
	SERIALIZE(f, cgb_bg_colors);
	SERIALIZE(f, cgb_obj_colors);
	DEBUG_MSG("done serializing ppu\n");
}

void PixelProcessingUnit::deserialize(std::ifstream &f) {
	DESERIALIZE(f, lcd_clock);
	DESERIALIZE(f, lcd_clock_vblank);
	DESERIALIZE(f, ctrl);
	DESERIALIZE(f, l_status);
	DESERIALIZE(f, scy);
	DESERIALIZE(f, scx);
	DESERIALIZE(f, ly);
	DESERIALIZE(f, lyc);
	DESERIALIZE(f, vblank_line);
	DESERIALIZE(f, dma);
	DESERIALIZE(f, bg_palette);
	DESERIALIZE(f, obj_palette_0);
	DESERIALIZE(f, obj_palette_1);
	DESERIALIZE(f, bg_palette_cgb);
	DESERIALIZE(f, bg_color_cgb);
	DESERIALIZE(f, obj_palette_cgb);
	DESERIALIZE(f, obj_color_cgb);
	DESERIALIZE(f, window_y);
	DESERIALIZE(f, window_x);
	DESERIALIZE(f, vbank_select);
	DESERIALIZE(f, wram_bank_select);
	DESERIALIZE(f, interrupt_signal);
	DESERIALIZE(f, hdma_source);
	DESERIALIZE(f, hdma_dest);
	DESERIALIZE(f, hdma);
	DESERIALIZE(f, hdma_enable);
	DESERIALIZE(f, hdma_bytes);
	DESERIALIZE(f, lcd_enabled);
	DESERIALIZE(f, window_line_active);
	DESERIALIZE(f, draw_screen);
	DESERIALIZE(f, drawn_scanline);
	DESERIALIZE(f, pixels_drawn);
	DESERIALIZE(f, tile_drawn);
	DESERIALIZE(f, screen_off_cycles);
	DESERIALIZE(f, hide_screen);
	DESERIALIZE(f, rgb555_framebuffer);
	DESERIALIZE(f, mono_framebuffer);
	DESERIALIZE(f, vram);
	DESERIALIZE(f, sprite_cache_buffer);
	DESERIALIZE(f, color_cache_buffer);
	DESERIALIZE(f, sprites);
	DESERIALIZE(f, tile_data);
	DESERIALIZE(f, cgb_bg_colors);
	DESERIALIZE(f, cgb_obj_colors);
	DEBUG_MSG("done deserializing ppu\n");
}

void AudioProcessingUnit::serialize(std::ofstream &f) {
	gb_apu_state_t apu_state;

	apu->save_state(&apu_state);
	SERIALIZE(f, elapsed_cycles);
	SERIALIZE(f, audio_buffer);
	SERIALIZE(f, apu_state);
	DEBUG_MSG("done serializing apu\n");
}

void AudioProcessingUnit::deserialize(std::ifstream &f, bool cgb_mode) {
	gb_apu_state_t apu_state;

	DESERIALIZE(f, elapsed_cycles);
	DESERIALIZE(f, audio_buffer);
	DESERIALIZE(f, apu_state);

	Gb_Apu::mode_t mode = cgb_mode ? Gb_Apu::mode_cgb : Gb_Apu::mode_dmg;
	apu->reset(mode);
	apu->load_state(apu_state);
	stereo_buffer->clear();
	DEBUG_MSG("done deserializing apu\n");
}

void Rom::serialize(std::ofstream &f) {
	for (size_t i = 0; i < rom_banks.size(); i++) {
		SERIALIZE(f, rom_banks[i]);
	}
	for (size_t i = 0; i < ram_banks.size(); i++) {
		SERIALIZE(f, ram_banks[i]);
	}
}

void Rom::deserialize(std::ifstream &f) {
	for (size_t i = 0; i < rom_banks.size(); i++) {
		DESERIALIZE(f, rom_banks[i]);
	}
	for (size_t i = 0; i < ram_banks.size(); i++) {
		DESERIALIZE(f, ram_banks[i]);
	}
}

void MCB3::serialize(std::ofstream &f) {
	Rom::serialize(f);
	SERIALIZE(f, rom_bank);
	SERIALIZE(f, ram_bank);
	SERIALIZE(f, ram_timer_enable);

	SERIALIZE(f, seconds);
	SERIALIZE(f, minutes);
	SERIALIZE(f, hours);
	SERIALIZE(f, days);
	SERIALIZE(f, flags);
	SERIALIZE(f, latched_seconds);
	SERIALIZE(f, latched_minutes);
	SERIALIZE(f, latched_hours);
	SERIALIZE(f, latched_days);
	SERIALIZE(f, latched_flags);

	SERIALIZE(f, rtc);
	SERIALIZE(f, latch);
	SERIALIZE(f, last_time);
	SERIALIZE(f, last_time_cached);
	DEBUG_MSG("done serializing rom");
}

void MCB3::deserialize(std::ifstream &f) {
	Rom::deserialize(f);
	DESERIALIZE(f, rom_bank);
	DESERIALIZE(f, ram_bank);
	DESERIALIZE(f, ram_timer_enable);

	DESERIALIZE(f, seconds);
	DESERIALIZE(f, minutes);
	DESERIALIZE(f, hours);
	DESERIALIZE(f, days);
	DESERIALIZE(f, flags);
	DESERIALIZE(f, latched_seconds);
	DESERIALIZE(f, latched_minutes);
	DESERIALIZE(f, latched_hours);
	DESERIALIZE(f, latched_days);
	DESERIALIZE(f, latched_flags);

	DESERIALIZE(f, rtc);
	DESERIALIZE(f, latch);
	DESERIALIZE(f, last_time);
	DESERIALIZE(f, last_time_cached);
	DEBUG_MSG("done deserializing rom");
}

void MCB1::serialize(std::ofstream &f) {
	Rom::serialize(f);
	SERIALIZE(f, rom_bank);
	SERIALIZE(f, secondary_rom_bank);
	SERIALIZE(f, ram_bank);
	SERIALIZE(f, ram_enable);
	SERIALIZE(f, rom_ram_mode);
	DEBUG_MSG("done serializing rom");
}

void MCB1::deserialize(std::ifstream &f) {
	Rom::deserialize(f);
	DESERIALIZE(f, rom_bank);
	DESERIALIZE(f, secondary_rom_bank);
	DESERIALIZE(f, ram_bank);
	DESERIALIZE(f, ram_enable);
	DESERIALIZE(f, rom_ram_mode);
	DEBUG_MSG("done deserializing rom");
}

void MCB1M::serialize(std::ofstream &f) {
	Rom::serialize(f);
	SERIALIZE(f, rom_bank);
	SERIALIZE(f, ram_bank);
	SERIALIZE(f, ram_enable);
	SERIALIZE(f, rom_ram_mode);
	DEBUG_MSG("done serializing rom");
}

void MCB1M::deserialize(std::ifstream &f) {
	Rom::deserialize(f);
	DESERIALIZE(f, rom_bank);
	DESERIALIZE(f, ram_bank);
	DESERIALIZE(f, ram_enable);
	DESERIALIZE(f, rom_ram_mode);
	DEBUG_MSG("done deserializing rom");
}

void MCB2::serialize(std::ofstream &f) {
	Rom::serialize(f);
	SERIALIZE(f, rom_bank);
	SERIALIZE(f, ram_enable);
	SERIALIZE(f, ram);
	DEBUG_MSG("done serializing rom");
}

void MCB2::deserialize(std::ifstream &f) {
	Rom::deserialize(f);
	DESERIALIZE(f, rom_bank);
	DESERIALIZE(f, ram_enable);
	DESERIALIZE(f, ram);
	DEBUG_MSG("done deserializing rom");
}

void MCB5::serialize(std::ofstream &f) {
	Rom::serialize(f);
	SERIALIZE(f, rom_bank);
	SERIALIZE(f, secondary_rom_bank);
	SERIALIZE(f, ram_bank);
	SERIALIZE(f, ram_enable);
	DEBUG_MSG("done serializing rom");
}

void MCB5::deserialize(std::ifstream &f) {
	Rom::deserialize(f);
	DESERIALIZE(f, rom_bank);
	DESERIALIZE(f, secondary_rom_bank);
	DESERIALIZE(f, ram_bank);
	DESERIALIZE(f, ram_enable);
	DEBUG_MSG("done deserializing rom");
}

void RomOnly::serialize(std::ofstream &f) {
	Rom::serialize(f);
	SERIALIZE(f, ram_enable);
	DEBUG_MSG("done serializing rom");
}

void RomOnly::deserialize(std::ifstream &f) {
	Rom::deserialize(f);
	DESERIALIZE(f, ram_enable);
	DEBUG_MSG("done deserializing rom");
}

void Rom::save_ram() {
	if (!battery) {
		return;
	}
	char *path = SDL_GetPrefPath("GBMU-42", "gbmu");
	std::string full_path = path + name() + ".ram";
	SDL_free(static_cast<void *>(path));
	std::ofstream f(full_path, std::ios::binary);

	if (!f.is_open()) {
		ERROR_MSG("Error: Failed to  open file for saving ram.\n");
		return;
	}
	DEBUG_MSG("writing ram to: %s\n", full_path.c_str());
	for (size_t i = 0; i < ram_banks.size(); i++) {
		SERIALIZE(f, ram_banks[i]);
	}
	f.close();
}

void Rom::load_ram() {
	if (!battery) {
		return;
	}
	char *path = SDL_GetPrefPath("GBMU-42", "gbmu");
	std::string full_path = path + name() + ".ram";
	SDL_free(static_cast<void *>(path));
	std::ifstream f(full_path, std::ios::binary);

	if (!f.is_open() || f.fail()) {
		ERROR_MSG("Error: Failed to  open file for loading ram.\n");
		return;
	}
	f.seekg(0, f.end);
	size_t file_size = f.tellg();
	f.seekg(0, f.beg);
	if (file_size < 0x2000 * ram_banks.size()) {
		ERROR_MSG("Error: Ram Save file is too small.\n");
		return;
	}
	for (size_t i = 0; i < ram_banks.size(); i++) {
		DESERIALIZE(f, ram_banks[i]);
	}
	f.close();
}

void MCB3::save_ram() {
	if (!battery) {
		return;
	}
	char *path = SDL_GetPrefPath("GBMU-42", "gbmu");
	std::string full_path = path + name() + ".ram";
	SDL_free(static_cast<void *>(path));
	std::ofstream f(full_path, std::ios::binary);

	if (!f.is_open()) {
		ERROR_MSG("Error: Failed to  open file for saving ram.\n");
		return;
	}
	DEBUG_MSG("writing ram to: %s\n", full_path.c_str());
	for (size_t i = 0; i < ram_banks.size(); i++) {
		SERIALIZE(f, ram_banks[i]);
	}
	if (has_rtc) {
		SERIALIZE(f, seconds);
		SERIALIZE(f, minutes);
		SERIALIZE(f, hours);
		SERIALIZE(f, days);
		SERIALIZE(f, flags);
		SERIALIZE(f, latched_seconds);
		SERIALIZE(f, latched_minutes);
		SERIALIZE(f, latched_hours);
		SERIALIZE(f, latched_days);
		SERIALIZE(f, latched_flags);
		SERIALIZE(f, last_time);
	}
	f.close();
}

void MCB3::load_ram() {
	if (!battery) {
		return;
	}
	char *path = SDL_GetPrefPath("GBMU-42", "gbmu");
	std::string full_path = path + name() + ".ram";
	SDL_free(static_cast<void *>(path));
	std::ifstream f(full_path, std::ios::binary);

	if (!f.is_open() || f.fail()) {
		ERROR_MSG("Error: Failed to  open file for loading ram.\n");
		return;
	}

	f.seekg(0, f.end);
	size_t file_size = f.tellg();
	f.seekg(0, f.beg);

	bool rtc_load = true;
	if (file_size < 0x2000 * ram_banks.size()) {
		ERROR_MSG("Error: Ram Save file is too small.\n");
		return;
	} else if (file_size < 0x2000 * ram_banks.size() + 16) {
		ERROR_MSG("Error: Ram has no valid RTC data. Skipping loading RTC.\n");
		rtc_load = false;
	}

	for (size_t i = 0; i < ram_banks.size(); i++) {
		DESERIALIZE(f, ram_banks[i]);
	}
	if (has_rtc && rtc_load) {
		DESERIALIZE(f, seconds);
		DESERIALIZE(f, minutes);
		DESERIALIZE(f, hours);
		DESERIALIZE(f, days);
		DESERIALIZE(f, flags);
		DESERIALIZE(f, latched_seconds);
		DESERIALIZE(f, latched_minutes);
		DESERIALIZE(f, latched_hours);
		DESERIALIZE(f, latched_days);
		DESERIALIZE(f, latched_flags);
		DESERIALIZE(f, last_time);
	}
	f.close();
	DEBUG_MSG("Loading Ram finished.\n");
}