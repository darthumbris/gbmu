#ifndef MEMORYMAP_HPP
#define MEMORYMAP_HPP

class Cpu;

#include <vector>
#include <array>
#include <cstdint>
#include <string>
#include <iostream>
#include <SDL2/SDL_keycode.h>

constexpr std::uint8_t mask0{ 0b0000'0001 }; // represents bit 0
constexpr std::uint8_t mask1{ 0b0000'0010 }; // represents bit 1
constexpr std::uint8_t mask2{ 0b0000'0100 }; // represents bit 2
constexpr std::uint8_t mask3{ 0b0000'1000 }; // represents bit 3
constexpr std::uint8_t mask4{ 0b0001'0000 }; // represents bit 4
constexpr std::uint8_t mask5{ 0b0010'0000 }; // represents bit 5
constexpr std::uint8_t mask6{ 0b0100'0000 }; // represents bit 6
constexpr std::uint8_t mask7{ 0b1000'0000 }; // represents bit 7

// https://gbdev.io/pandocs/Memory_Map.html

using Mem16k = std::array<uint8_t, 16384>;
using Mem8k = std::array<uint8_t, 8192>;
using Mem4k = std::array<uint8_t, 4096>;

#define INLINE_FN


/*IO_Registers
 * 0xFF00           Joypad input
 * 0xFF01-0xFF02    Serial transfer
 * 0xFF04-0xFF07    Timer and divider
 * 0xFF10-0xFF26    Audio
 * 0xFF30-0xFF3F    Wave Pattern
 * 0xFF40-0xFF4b    LCD Control, Status, Position, Scrolling, palettes
 * 0xFF4F           Vram bank select (CGB)
 * 0xFF50           Set to non-zero to disable boot rom
 * 0xFF51-0xFF55    VRAM DMA (CGB)
 * 0xFF68-0xFF6B    BG/OBJ Palettes (CGB)
 * 0xFF70           WRAM BANK SELECT (CGB)  
 */

 /* Timer and Divider
  * 0xFF04 Divider Register 
            (increases with 16384Hz(16779Hz CGB) or 32768Hz in double mode. 
            If written to this register resets it to 0x00)
            Stop instruction also resets it to 0x00
            Also resets during a speed switch
    
  * 0xFF05 Timer Counter (TIMA)
            Incremented at the clock frequency specified by FF07
            when the value overflows it is reset to the value specified in FF06
            and an interrupt is requested
  * 0xFF06 Timer Modulo (TMA)
            When TIMA overflows it is reset to the value in this register and an 
            interrupt is requested
  * 0xFF07 Timer Control (TAC)
            bits: bit 2 enable -> controls whether TIMA (0xFF05) is incremented
            bit 1 and 0 clock select:
                00: increment every 256 M-cycles   freq 4096(DMG)      4194(SGB1)      8192(CGB double speed)
                01: increment every 4 M-cycles     freq 262144(DMG)    268400(SGB1)    524288(CGB double speed)
                10: increment every 16 M-cycles    freq 65536(DMG)     67110(SGB1)     131072(CGB double speed)
                11: increment every 64 M-cycles    freq 16384(DMG)     16780(SGB1)     32768(CGB double speed)
 */

 /* VRAM DMA
  * 0xFF51-0xFF52 VRAM DMA SOURCE
  * 0xFF53-0xFF54 VRAM DMA Destination
  * 0xFF55        VRAM DMA length/mode/start
 */

 /* Interrupt
  * 0x40 Vblank interrupt
  * 0x48 STAT interrupt
  * 0x50 Timer interrupt
  * 0x58 Serial interrupt
  * 0x60 Joypad interrupt
 */

class MemoryMap
{
private:
    Mem16k rom{0};                            // 0x0000 - 0x3FFF
    
    std::vector<Mem8k> ext_ram{0};             // 0xA000 - 0xBFFF   // From cartridge, switchable bank if any //32K max
    std::array<Mem4k, 8> work_ram{0};         // 0xC000 - 0xDFFF   // In CGB mode, switchable bank 1–7
    std::array<Mem4k, 8> echo_ram{0};         // 0xE000 - 0xFDFF   //(mirror of C000–DDFF) use of this area is prohibited.
    std::vector<Mem16k> rom_banks{0};          // 0x4000 - 0x7FFF   // From cartridge, switchable bank if any //2M max
    std::array<uint8_t, 96> not_usable{0};    // 0xFEA0 - 0xFEFF
    std::array<uint8_t, 128> io_registers{0}; // 0xFF00 - 0xFF7F
    std::array<uint8_t, 127> high_ram{0};     // 0xFF80 - 0xFFFE
    uint8_t interrupt = 0;                    // 0xFFFF - 0xFFFF

    uint8_t rom_bank = 1;
    uint8_t ram_bank = 0;
    bool ram_enable = false;

    uint8_t joypad = 0;
    uint8_t joypad_dpad = 0x0F;
    uint8_t joypad_buttons = 0x0F;
    bool boot_rom_loaded = false;


    std::array<uint8_t, 256> gb_boot_rom = {
        0x31, 0xfe, 0xff, 0xaf, 0x21, 0xff, 0x9f, 0x32, 0xcb, 0x7c, 0x20, 0xfb,
        0x21, 0x26, 0xff, 0x0e, 0x11, 0x3e, 0x80, 0x32, 0xe2, 0x0c, 0x3e, 0xf3,
        0xe2, 0x32, 0x3e, 0x77, 0x77, 0x3e, 0xfc, 0xe0, 0x47, 0x11, 0x04, 0x01,
        0x21, 0x10, 0x80, 0x1a, 0xcd, 0x95, 0x00, 0xcd, 0x96, 0x00, 0x13, 0x7b,
        0xfe, 0x34, 0x20, 0xf3, 0x11, 0xd8, 0x00, 0x06, 0x08, 0x1a, 0x13, 0x22,
        0x23, 0x05, 0x20, 0xf9, 0x3e, 0x19, 0xea, 0x10, 0x99, 0x21, 0x2f, 0x99,
        0x0e, 0x0c, 0x3d, 0x28, 0x08, 0x32, 0x0d, 0x20, 0xf9, 0x2e, 0x0f, 0x18,
        0xf3, 0x67, 0x3e, 0x64, 0x57, 0xe0, 0x42, 0x3e, 0x91, 0xe0, 0x40, 0x04,
        0x1e, 0x02, 0x0e, 0x0c, 0xf0, 0x44, 0xfe, 0x90, 0x20, 0xfa, 0x0d, 0x20,
        0xf7, 0x1d, 0x20, 0xf2, 0x0e, 0x13, 0x24, 0x7c, 0x1e, 0x83, 0xfe, 0x62,
        0x28, 0x06, 0x1e, 0xc1, 0xfe, 0x64, 0x20, 0x06, 0x7b, 0xe2, 0x0c, 0x3e,
        0x87, 0xe2, 0xf0, 0x42, 0x90, 0xe0, 0x42, 0x15, 0x20, 0xd2, 0x05, 0x20,
        0x4f, 0x16, 0x20, 0x18, 0xcb, 0x4f, 0x06, 0x04, 0xc5, 0xcb, 0x11, 0x17,
        0xc1, 0xcb, 0x11, 0x17, 0x05, 0x20, 0xf5, 0x22, 0x23, 0x22, 0x23, 0xc9,
        0xce, 0xed, 0x66, 0x66, 0xcc, 0x0d, 0x00, 0x0b, 0x03, 0x73, 0x00, 0x83,
        0x00, 0x0c, 0x00, 0x0d, 0x00, 0x08, 0x11, 0x1f, 0x88, 0x89, 0x00, 0x0e,
        0xdc, 0xcc, 0x6e, 0xe6, 0xdd, 0xdd, 0xd9, 0x99, 0xbb, 0xbb, 0x67, 0x63,
        0x6e, 0x0e, 0xec, 0xcc, 0xdd, 0xdc, 0x99, 0x9f, 0xbb, 0xb9, 0x33, 0x3e,
        0x3c, 0x42, 0xb9, 0xa5, 0xb9, 0xa5, 0x42, 0x3c, 0x21, 0x04, 0x01, 0x11,
        0xa8, 0x00, 0x1a, 0x13, 0xbe, 0x20, 0xfe, 0x23, 0x7d, 0xfe, 0x34, 0x20,
        0xf5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xfb, 0x86, 0x20, 0xfe,
        0x3e, 0x01, 0xe0, 0x50
    };

    std::array<uint8_t, 2304>	cgb_boot_rom = {};

    Cpu *cpu;


public:
    MemoryMap(const std::string path, Cpu *cpu);

    ~MemoryMap();

    INLINE_FN uint8_t read_u8(uint16_t addr);
    INLINE_FN uint16_t read_u16(uint16_t addr);
    INLINE_FN void write_u8(uint16_t addr, uint8_t val);
    INLINE_FN void write_u16(uint16_t addr, uint16_t val);

    inline bool is_boot_rom_enabled() {return io_registers[(std::size_t)(0xFF50 - 0xFF00)];}
    uint8_t wram_bank_select(); //TODO only bits 0-2 should be used (and if in DMG should return 0 or 1)

    inline void set_interrupt(uint8_t i) {interrupt |= i;}

    void handle_keydown(SDL_Keycode key);
    void handle_keyup(SDL_Keycode key);

    void serialize(std::ofstream &f);
	void deserialize(std::ifstream &f);
};

#endif
