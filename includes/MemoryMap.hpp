#ifndef MEMORYMAP_HPP
#define MEMORYMAP_HPP

#include <vector>
#include <array>
#include <cstdint>
#include <string>

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

struct Sprite {
    uint8_t y_pos;
    uint8_t x_pos;
    uint8_t tile_index;
    uint8_t att_flags;
};


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
    // TODO add a check for if the bios has been loaded in
    Mem16k rom{0};                            // 0x0000 - 0x3FFF
    std::vector<Mem16k> rom_banks{};          // 0x4000 - 0x7FFF   // From cartridge, switchable bank if any //2M max
    std::array<Mem8k, 2> vram{0};             // 0x8000 - 0x9FFF   // 0 for GB and 0-1 for Cgb (switchable banks)
    std::vector<Mem8k> ext_ram{};             // 0xA000 - 0xBFFF   // From cartridge, switchable bank if any //32K max
    std::array<Mem4k, 8> work_ram{0};         // 0xC000 - 0xDFFF   // In CGB mode, switchable bank 1–7
    std::array<Mem4k, 8> echo_ram{0};         // 0xE000 - 0xFDFF   //(mirror of C000–DDFF) use of this area is prohibited.
    std::array<uint8_t, 160> oam{0};          // 0xFE00 - 0xFE9F   // 40 * 4 bytes(Byte 0: ypos, Byte1: Xpos, Byte2: tile_index, Byte3: Attributes/flags)
    std::array<uint8_t, 96> not_usable{0};    // 0xFEA0 - 0xFEFF
    std::array<uint8_t, 128> io_registers{0}; // 0xFF00 - 0xFF7F
    std::array<uint8_t, 127> high_ram{0};     // 0xFF80 - 0xFFFE
    uint8_t interrupt = 0;                    // 0xFFFF - 0xFFFF

    std::array<uint8_t, 256> boot_rom = {0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E,
                                     0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3, 0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0,
                                     0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B,
                                     0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9,
                                     0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20,
                                     0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04,
                                     0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2,
                                     0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62, 0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06,
                                     0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xF2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20,
                                     0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17,
                                     0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
                                     0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
                                     0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
                                     0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E, 0x3c, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C,
                                     0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x20, 0xFE, 0x23, 0x7D, 0xFE, 0x34, 0x20,
                                     0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x20, 0xFE, 0x3E, 0x01, 0xE0, 0x50};



public:
    MemoryMap();
    MemoryMap(const std::string path);

    ~MemoryMap();

    uint8_t read_u8(uint16_t addr);
    uint16_t read_u16(uint16_t addr);
    void write_u8(uint16_t addr, uint8_t val);
    void write_u16(uint16_t addr, uint16_t val);

    inline uint8_t get_tile_index(uint8_t vbank, uint8_t x, uint8_t y, uint8_t tile_map) {return vram[vbank][0x1800 + (x + y * 32 + 32 * tile_map)];} //TODO check this
    inline uint8_t *get_tile_data(uint8_t vbank, uint8_t tile_index) {return &vram[vbank][tile_index];} //TODO check this


    inline bool is_boot_rom_enabled() {return io_registers[(std::size_t)(0xFF50 & 0x7F)];}
    inline uint8_t vram_bank_select() {return io_registers[(std::size_t)(0xFF4F & 0x7F)];} //only bit 0 matters
    inline uint8_t wram_bank_select() {return io_registers[(std::size_t)(0xFF70 & 0x7F)];} //TODO only bits 0-2 should be used (and if in DMG should return 0 or 1)
    

    Sprite get_sprite(size_t index);

    //PPU IO Registers
    inline uint8_t get_lcd_control() {return io_registers[(std::size_t)(0xFF40 & 0x7F)];}
    inline uint8_t get_lcd_status() {return io_registers[(std::size_t)(0xFF41 & 0x7F)];}
    inline uint8_t get_lcd_scrolling_y() {return io_registers[(std::size_t)(0xFF42 & 0x7F)];}
    inline uint8_t get_lcd_scrolling_x() {return io_registers[(std::size_t)(0xFF43 & 0x7F)];}
    inline uint8_t get_lcd_line_y() {return io_registers[(std::size_t)(0xFF44 & 0x7F)];}
    inline uint8_t get_lcd_line_y_compare() {return io_registers[(std::size_t)(0xFF45 & 0x7F)];}

    inline uint8_t get_dma_transfer_start_address() {return io_registers[(std::size_t)(0xFF46 & 0x7F)];}

    //Palettes
    inline uint8_t get_background_palette_data() {return io_registers[(std::size_t)(0xFF47 & 0x7F)];}
    inline uint8_t get_obj_0_palette_data() {return io_registers[(std::size_t)(0xFF48 & 0x7F)];}
    inline uint8_t get_obj_1_palette_data() {return io_registers[(std::size_t)(0xFF49 & 0x7F)];}
    
    inline uint8_t get_lcd_window_y() {return io_registers[(std::size_t)(0xFF4A & 0x7F)];}
    inline uint8_t get_lcd_window_x() {return io_registers[(std::size_t)(0xFF4B & 0x7F)];} //TODO x + 7 ? see https://gbdev.io/pandocs/Scrolling.html#lcd-position-and-scrolling

    //0xFF4C unkown register
    //0xFF4D KEY1 - CGB Mode Only - Prepare Speed Switch
    //0xFF4E unkown register
    //0xFF56 RP - CGB Mode Only - Infrared Communications Port
    //0xFF57-0xFF67 unkown register
    //0xFF6C-0xFF6F unkown register
    //0xFF71-0xFF7F unkown register

    //CGB Palettes
    inline uint8_t get_background_color_palette_index() {return io_registers[(std::size_t)(0xFF68 & 0x7F)];}
    inline uint8_t get_background_color_palette_data() {return io_registers[(std::size_t)(0xFF69 & 0x7F)];}
    inline uint8_t get_obj_color_palette_index() {return io_registers[(std::size_t)(0xFF6A & 0x7F)];}
    inline uint8_t get_obj_color_palette_data() {return io_registers[(std::size_t)(0xFF6B & 0x7F)];}

    inline uint8_t get_ppu_mode() {return ((get_lcd_status() & mask0) + (get_lcd_status() & mask1));}
    inline bool get_lcd_enable() { return get_lcd_control() & mask7; }
    inline bool get_window_tilemap() { return get_lcd_control() & mask6;}
    inline bool get_window_enable() { return get_lcd_control() & mask5; }
    inline bool get_bg_window_tiles() { return get_lcd_control() & mask4;}
    inline bool get_background_tilemap() { return get_lcd_control() & mask3;}
    inline bool get_obj_size() { return get_lcd_control() & mask2;}
    inline bool get_obj_enable() { return get_lcd_control() & mask1;}
    inline bool get_bg_window_enable_priority() { return get_lcd_control() & mask0;}

    void set_ppu_mode(uint8_t mode);
    inline void increase_lcd_line_y() {io_registers[(std::size_t)(0xFF44 & 0x7F)] += 1;}
    inline void reset_lcd_line_y() {io_registers[(std::size_t)(0xFF44 & 0x7F)] = 0;}
    inline void reset_lcd_window_y() {io_registers[(std::size_t)(0xFF4A & 0x7F)] = 0;}
    inline void set_bg_window_enable_priority(bool val) { io_registers[(std::size_t)(0xFF40 & 0x7F)] = ((-val)) ^ io_registers[(std::size_t)(0xFF40 & 0x7F)] & (1U << 0) ;}
};

#endif
