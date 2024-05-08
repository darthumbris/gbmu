#ifndef ROM_HPP
# define ROM_HPP

#include <cstdint>
#include <string>
#include <vector>

enum CGB_FLAGS
{
    Non_CGB_Mode,
    CGB_Enhanced,
    CGB_ONLY,
    PGB_Mode
};

class Rom
{
private:
    /* data */
    uint32_t entry_point; //0x100-0x103 
    std::vector<uint8_t> logo; // 0x104-0x133

    char _name[15];            // 0x134-0x142
    uint8_t cgb_flag;       // 0x143
    uint16_t license_code;    // 0x144-0x145
    bool sgb_flag;            // 0x146
    uint8_t cartridge_type;   // 0x147
    uint8_t rom_size;         // 0x148
    uint8_t ram_size;         // 0x149
    uint8_t dest_code;        // 0x14A
    uint8_t old_license_code; // 0x14B
    uint8_t version;          // 0x14C
    uint8_t header_checksum;  // 0x14D
    uint16_t global_checksum; // 0x14E-0x14F

public:
    Rom(std::string argv);
    ~Rom();

    void print_rom() const;
};

#endif