#ifndef ROM_HPP
#define ROM_HPP

#include "RomHeader.hpp"
#include <array>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

using mem_16k = std::array<uint8_t, 16384>;
using mem_8k = std::array<uint8_t, 8192>;

class Rom {
private:
	RomHeader header;
	bool cgb_on = false;

public:
	Rom(const std::string rom_path, RomHeader rheader);
	virtual ~Rom();

	template <typename MBC, typename... Args>
	static std::unique_ptr<Rom> make(Args... args) {
		return (std::make_unique<MBC>(args...));
	}

	virtual uint8_t read_u8(uint16_t addr) = 0;
	virtual void write_u8(uint16_t addr, uint8_t val) = 0;

	virtual void serialize(std::ofstream &f) = 0;
	virtual void deserialize(std::ifstream &f) = 0;

	void save_ram();
	void load_ram();

	std::string name() const {
		return header.name();
	}
	inline bool cgb_mode() const {
		return cgb_on;
	}

protected:
	std::vector<mem_16k> rom_banks{0};
	std::vector<mem_8k> ram_banks{0};
};

// TODO handle MCB1, MCB2, MCB3, MCB5

#endif