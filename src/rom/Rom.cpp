#include "rom/Rom.hpp"
#include "debug.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_filesystem.h>
#include <SDL2/SDL_stdinc.h>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>

Rom::Rom(const std::string rom_path, RomHeader rheader) : header(rheader) {
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

void Rom::save_ram() {
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
		f.write(reinterpret_cast<const char *>(&ram_banks[i]), sizeof(ram_banks[i]));
	}
	f.close();
}

void Rom::load_ram() {
	// TODO have a check if the file_size is correct for the amount of ram expected
	char *path = SDL_GetPrefPath("GBMU-42", "gbmu");
	std::string full_path = path + name() + ".ram";
	SDL_free(static_cast<void *>(path));
	std::ifstream f(full_path, std::ios::binary);

	if (!f.is_open()) {
		DEBUG_MSG("Error: Failed to  open file for loading ram.\n");
		return;
	}
	// DEBUG_MSG("loading ram from: %s\n", full_path.c_str());
	for (size_t i = 0; i < ram_banks.size(); i++) {
		f.read(reinterpret_cast<char *>(&ram_banks[i]), sizeof(ram_banks[i]));
	}
	f.close();
}