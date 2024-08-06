#include "rom/Rom.hpp"
#include <cstdio>
#include <fstream>

Rom::Rom(const std::string rom_path, RomHeader rheader) : header(rheader) {
	std::ifstream ifs;
	ifs.open(rom_path.c_str(), std::ifstream::binary);
	// making rom and ram banks
	ifs.seekg(0, std::ios::beg);
	rom_banks.reserve(header.rom_size());
	ram_banks.reserve(header.ram_size());
	for (int i = 0; i < header.rom_size(); i++) {
		rom_banks.push_back({});
		ifs.read(reinterpret_cast<char *>(&rom_banks[i]), sizeof(rom_banks[i]));
	}
	for (int i = 0; i < header.ram_size(); i++) {
		ram_banks.push_back({0});
	}
	cgb_on = header.is_cgb_game();
	ifs.close();
}

Rom::~Rom() {}