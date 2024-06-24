#ifndef MCB5_HPP
#define MCB5_HPP

#include "Rom.hpp"

class MCB5 : public Rom {
    private:
    uint8_t rom_bank = 1;
	uint8_t ram_bank = 0;
	bool ram_enable = false;
    bool rom_ram_mode = false;
    bool battery = false;

    public:
    uint8_t read_u8(uint16_t addr);
    uint16_t read_u16(uint16_t addr);
    void write_u8(uint16_t addr, uint8_t val);
    void write_u16(uint16_t addr, uint16_t val);
};

#endif