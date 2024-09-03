#include "rom/Rom.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_filesystem.h>
#include <SDL2/SDL_stdinc.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>

Rom::Rom(const std::string rom_path, RomHeader rheader, bool battery) : header(rheader), battery(battery) {
	std::ifstream ifs;
	ifs.open(rom_path.c_str(), std::ifstream::binary);
	ifs.seekg(0, std::ios::beg);
	rom_banks.reserve(header.rom_size());
	ram_banks.reserve(header.ram_size());
	for (uint16_t i = 0; i < header.rom_size(); i++) {
		rom_banks.push_back({});
		ifs.read(reinterpret_cast<char *>(&rom_banks[i]), sizeof(rom_banks[i]));
	}
	for (uint8_t i = 0; i < header.ram_size(); i++) {
		ram_banks.push_back({0});
	}
	cgb_on = header.is_cgb_game();
	ifs.close();
}

Rom::~Rom() {}

void Rom::reset() {
	save_ram();
	for (size_t i = 0; i < ram_banks.size(); i++) {
		ram_banks[i] = {0};
	}
	load_ram();
}