#include "rom/MCB3.hpp"
#include "MemoryMap.hpp"
#include "debug.hpp"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <SDL2/SDL_filesystem.h>

#define SERIALIZE(f, x) (f.write(reinterpret_cast<const char *>(&x), sizeof(x)))
#define DESERIALIZE(f, x) (f.read(reinterpret_cast<char *>(&x), sizeof(x)))

uint8_t MCB3::read_u8(uint16_t addr) {
	switch (addr) {
	case 0x0000 ... 0x3FFF:
		return rom_banks[0][addr];
	case 0x4000 ... 0x7FFF:
		return rom_banks[rom_bank][addr - 0x4000];
	case 0xA000 ... 0xBFFF:
		if (ram_bank >= 0) {
			if (ram_timer_enable && ram_banks.size()) {
				return ram_banks[ram_bank][addr - 0xA000];
			}
		} else if (has_rtc && ram_timer_enable) {
			switch (rtc) {
			case (uint8_t)rtc_registers::seconds:
				return latched_seconds;
			case (uint8_t)rtc_registers::minutes:
				return latched_minutes;
			case (uint8_t)rtc_registers::hours:
				return latched_hours;
			case (uint8_t)rtc_registers::days:
				return latched_days;
			case (uint8_t)rtc_registers::flags:
				return latched_flags;
			default:
				return 0xFF;
			}
		}
		return 0xFF;
	default:
		std::cerr << "should not reach this" << std::endl;
		return 0xFF;
	}
}

void MCB3::write_u8(uint16_t addr, uint8_t val) {
	switch (addr) {
	case 0x0000 ... 0x1FFF:
		ram_timer_enable = ((val & 0x0F) == 0x0A);
		break;
	case 0x2000 ... 0x3FFF:
		rom_bank = val ? (val & 0x7F) : 1;
		rom_bank &= (rom_banks.size() - 1); // TODO check if this behaves
		break;
	case 0x4000 ... 0x5FFF:
		if (val < 0x04) {
			ram_bank = val;
			ram_bank &= ram_banks.size() - 1; // TODO check if this behaves
		} else if (val >= 0x08 && val <= 0x0C) {
			if (has_rtc && ram_timer_enable) {
				rtc = val;
				ram_bank = -1;
			}
		}
		break;
	case 0x6000 ... 0x7FFF:
		if (has_rtc) {
			if (latch == 0x00 && val == 0x01) {
				update_rtc();
				latched_seconds = seconds;
				latched_minutes = minutes;
				latched_hours = hours;
				latched_days = days;
				latched_flags = flags;
			}
			latch = val;
		}
		break;
	case 0xA000 ... 0xBFFF:
		if (!ram_timer_enable) {
			return;
		}
		if (ram_bank >= 0 && ram_banks.size()) {
			DEBUG_MSG("ram_bank: %u addr: %#06x val: %u\n", ram_bank, addr, val);
			ram_banks[ram_bank][addr - 0xA000] = val;
		} else if (has_rtc) {
			switch (rtc) {
			case (uint8_t)rtc_registers::seconds:
				seconds = val;
				break;
			case (uint8_t)rtc_registers::minutes:
				minutes = val;
				break;
			case (uint8_t)rtc_registers::hours:
				hours = val;
				break;
			case (uint8_t)rtc_registers::days:
				days = val;
				break;
			case (uint8_t)rtc_registers::flags:
				flags = (flags & 0b10000000) | (val & 0b11000001);
				break;
			}
		}
		break;
	default:
		std::cerr << "should not reach this" << std::endl;
		break;
	}
}

void MCB3::update_clock() {
	rtc_counter += 1;
	if (rtc_counter == 20) {
		rtc_counter = 0;
		time(&timer);
	}
}

void MCB3::update_rtc() {
	int32_t current_time = timer;
	if (!(flags & mask6) && last_time_cached != current_time) {
		last_time_cached = current_time;
		int32_t delta_time = current_time - last_time;
		last_time = current_time;

		if (delta_time <= 0) {
			return;
		}
		seconds += delta_time % 60;
		if (seconds > 59) {
			seconds -= 60;
			minutes += 1;
		}
		minutes += (delta_time / 60) % 60;
		if (minutes > 59) {
			minutes -= 60;
			hours += 1;
		}
		hours += (delta_time / 3600) % 24;
		if (hours > 23) {
			hours -= 24;
			days += 1;
		}
		days += (delta_time / 86400) & 0xFFFFFFFF;
		if (days > 255) {
			flags = (flags & 0b11000001) | 0x01;
			if (days > 511) {
				days %= 512;
				flags |= 0b10000000;
				flags &= 0b11000000;
			}
		}
	}
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
		DEBUG_MSG("Error: Failed to  open file for saving ram.\n");
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
	// TODO have a check if the file_size is correct for the amount of ram expected
	if (!battery) {
		return;
	}
	char *path = SDL_GetPrefPath("GBMU-42", "gbmu");
	std::string full_path = path + name() + ".ram";
	SDL_free(static_cast<void *>(path));
	std::ifstream f(full_path, std::ios::binary);

	if (!f.is_open()) {
		DEBUG_MSG("Error: Failed to  open file for loading ram.\n");
		return;
	}
	for (size_t i = 0; i < ram_banks.size(); i++) {
		DESERIALIZE(f, ram_banks[i]);
	}
	if (has_rtc) {
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
}

void MCB3::serialize(std::ofstream &f) {
	for (size_t i = 0; i < rom_banks.size(); i++) {
		SERIALIZE(f, rom_banks[i]);
	}
	for (size_t i = 0; i < ram_banks.size(); i++) {
		SERIALIZE(f, ram_banks[i]);
	}
	f.write(reinterpret_cast<const char *>(&rom_bank), sizeof(rom_bank));
	f.write(reinterpret_cast<const char *>(&ram_bank), sizeof(ram_bank));
	f.write(reinterpret_cast<const char *>(&ram_timer_enable), sizeof(ram_timer_enable));
	f.write(reinterpret_cast<const char *>(&seconds), sizeof(seconds));
	f.write(reinterpret_cast<const char *>(&minutes), sizeof(minutes));
	f.write(reinterpret_cast<const char *>(&hours), sizeof(hours));
	f.write(reinterpret_cast<const char *>(&days), sizeof(days));
	f.write(reinterpret_cast<const char *>(&flags), sizeof(flags));
	DEBUG_MSG("done serializing rom");
}

void MCB3::deserialize(std::ifstream &f) {
	for (size_t i = 0; i < rom_banks.size(); i++) {
		f.read(reinterpret_cast<char *>(&rom_banks[i]), sizeof(rom_banks[i]));
	}
	for (size_t i = 0; i < ram_banks.size(); i++) {
		f.read(reinterpret_cast<char *>(&ram_banks[i]), sizeof(ram_banks[i]));
	}
	f.read(reinterpret_cast<char *>(&rom_bank), sizeof(rom_bank));
	f.read(reinterpret_cast<char *>(&ram_bank), sizeof(ram_bank));
	f.read(reinterpret_cast<char *>(&ram_timer_enable), sizeof(ram_timer_enable));
	f.read(reinterpret_cast<char *>(&seconds), sizeof(seconds));
	f.read(reinterpret_cast<char *>(&minutes), sizeof(minutes));
	f.read(reinterpret_cast<char *>(&hours), sizeof(hours));
	DEBUG_MSG("done deserializing rom");
}
