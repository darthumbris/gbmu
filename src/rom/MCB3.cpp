#include "rom/MCB3.hpp"
#include "MemoryMap.hpp"
#include <SDL2/SDL_filesystem.h>
#include <cstdint>
#include <iostream>

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
		rom_bank &= ((uint8_t)rom_banks.size() - 1);
		break;
	case 0x4000 ... 0x5FFF:
		if (val < 0x04) {
			ram_bank = val;
			ram_bank &= (uint8_t)ram_banks.size() - 1;
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

void MCB3::reset() {
	save_ram();
	rom_bank = 1;
	ram_bank = 0;
	ram_timer_enable = false;
	seconds = 0;
	minutes = 0;
	hours = 0;
	days = 0;
	flags = 0;
	latched_seconds = 0;
	latched_minutes = 0;
	latched_hours = 0;
	latched_days = 0;
	latched_flags = 0;
	rtc =0;
	latch = 0;
	last_time = 0;
	last_time_cached = 0;
	timer = 0;
	rtc_counter = 0;
	time(&timer);
	for (size_t i = 0; i < ram_banks.size(); i++) {
		ram_banks[i] = {0};
	}
	load_ram();
}