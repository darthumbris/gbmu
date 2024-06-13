#include "PixelProcessingUnit.hpp"
#include "Cpu.hpp"
#include <cstring>

PixelProcessingUnit::PixelProcessingUnit(Cpu *cpu) : cpu(cpu)
{
    lcd_clock = 0;
    init_window();
    data.status = false;
    ctrl = {0};
    l_status = {0};
    l_status.mode = PPU_Modes::Pixel_Drawing;
    std::memset(oam, 0, sizeof(oam));
    std::memset(framebuffer, 0, sizeof(framebuffer));
    std::memset(vram, 0, sizeof(vram));
    std::memset(obj_0_colors, 0, sizeof(obj_0_colors));
    std::memset(obj_1_colors, 0, sizeof(obj_1_colors));
}

PixelProcessingUnit::~PixelProcessingUnit()
{
}

void PixelProcessingUnit::tick(uint8_t cycle)
{
    printf("tick: %u\t", cycle);
    if (ctrl.lcd_enable) {
        //TODO have a check for the dma (CGB only?) ?
        lcd_clock += (uint16_t)cycle;
        switch (l_status.mode)
        {
        case PPU_Modes::Horizontal_Blank:
            if (lcd_clock >= 204) {
                lcd_clock -= 204;
                ly +=1;
                if (ly == 144) {
                    l_status.mode = PPU_Modes::Vertical_Blank;
                    handle_interrupt(true);
                    draw_screen = true;
                    // render_screen(); //Renders the full screen when at the end of the 144 lines
                }
                else {
                    l_status.mode = PPU_Modes::OAM_Scan;
                    handle_interrupt(true);
                    if (ctrl.window_enable && window_y == ly) {
                        window_active = true;
                    }
                }
            }
            break;
        case PPU_Modes::Vertical_Blank:
            if (lcd_clock >= 456) {
                lcd_clock -= 456;
                ly +=1;
                if (ly > 153) {
                    window_line_active = 0;
                    ly = 0;
                    std::cout << "resetting ly " << std::endl;
                    l_status.mode = PPU_Modes::OAM_Scan;
                    handle_interrupt(true);
                    window_active = ctrl.window_enable && window_y == ly;
                }
            }
            break;
        case PPU_Modes::OAM_Scan:
            if (lcd_clock >= 80) {
                l_status.mode = PPU_Modes::Pixel_Drawing;
                lcd_clock -= 80;
            }
            break;
        case PPU_Modes::Pixel_Drawing:
            if (lcd_clock >= 172) {
                l_status.mode = PPU_Modes::Horizontal_Blank;
                lcd_clock -= 172;
                render_scanline();
                handle_interrupt(false);
            }
            break;

        default:
            break;
        }
    }
}

void PixelProcessingUnit::handle_interrupt(bool val) {
    if (l_status.mode == PPU_Modes::Vertical_Blank) {
        cpu->set_interrupt(InterruptType::Vblank);
    }
    if ((l_status.val & 0x8) && ly == lyc && val) {
        cpu->set_interrupt(InterruptType::Stat);
    }
	if (l_status.mode != PPU_Modes::Pixel_Drawing && (static_cast<uint8_t>(1 << l_status.mode) & l_status.val)) {
        cpu->set_interrupt(InterruptType::Stat);
    }
}

void PixelProcessingUnit::handle_sprites(std::vector<Sprite> sprites, uint32_t i, uint8_t tile_data_pos, uint32_t *framebuffer_ptr, size_t *spr_index) {
    bool drawn = false;
    for (auto spr = sprites.begin() + *spr_index ; !sprites.empty() && spr != sprites.end() ; ++spr) {
        if (!(spr->x_pos - 8 <= (int)(i) && spr->x_pos > (int)(i))) {
            break;
        }
        uint8_t tile_x = i - (spr->x_pos - 8);
        uint8_t tile_y = ly - (spr->y_pos - 16);

        if ((spr->att_flags >> 5) & 1)
            tile_x = 7 - tile_x;
        if ((spr->att_flags >> 6) & 1)
            tile_y = (ctrl.obj_size ? 15 : 7) - tile_y;

        uint8_t sprite_index = spr->tile_index;

        if (ctrl.obj_size) {
            sprite_index = (sprite_index & 0xFE) + (tile_y >= 8);
            tile_y %= 8;
        }

        uint8_t color_index = tile_data[vbank_select][sprite_index][tile_y * 8 + tile_x];

        bool background = ((spr->att_flags >> 7) & 1);
        if (color_index && !drawn && (!background || (background && !tile_data_pos))) {
            if ((spr->att_flags >> 4) & 1) {
                *framebuffer_ptr = obj_1_colors[color_index];
            }
            else {
                *framebuffer_ptr = obj_0_colors[color_index];
            }
            drawn = true;
            //TODO break here?
        }

        // Go to next sprite if this x marks the end of it
        if (i + 1 == spr->x_pos)
            (*spr_index)++;
    }
}

void PixelProcessingUnit::render_scanline() {
    std::vector<Sprite> sprites;

    sprites.reserve(10);
    if (ctrl.obj_enable) {
        for (int i = 0; i < 40; i++) {
            //Sprite stuff
            Sprite spr = get_sprite(i);
            if (ly < spr.y_pos && ly >= spr.y_pos - 16) { //obj can be 8*8 or 8*16 //TODO maybe do a obj_size check that returns 8 or 16
                if (ctrl.obj_size || ly < spr.y_pos - 8) {
                    sprites.push_back(spr);
                    //TODO maybe have a check for sprites.size == 10
                }
            }
        }
    }
    if (sprites.size()) {
        std::cout << "drawing sprites: " << sprites.size() << std::endl;
    }

    uint16_t tile_map_offset = ctrl.bg_tile_map_address ? 0x1c00 : 0x1800;
    uint16_t line_offset = scx >> 3;
    uint8_t y = (ly + scy) & 7;
    uint8_t x = scx & 7;

    tile_map_offset += (((ly + scy) & 255) >> 3) << 5;

    if (window_active && window_x == 7) {
        tile_map_offset = (ctrl.window_tile_map_address ? 0x1c00 : 0x1800) + ((window_line_active << 3) << 5);
        line_offset = 0;
        y = window_line_active & 7;
        x = 0;
    }

    uint32_t *framebuffer_ptr = framebuffer + ly * 160;
    uint16_t tile_index = vram[0][tile_map_offset + line_offset];

    if (!ctrl.bg_window_tile_data) {
        tile_index = static_cast<uint16_t>(256 + static_cast<uint8_t>(tile_index & 0xFF));
    }

    size_t spr_index = 0;

    for (uint32_t i = 0; i < 160; i++) {

        uint8_t tile_dat = tile_data[vbank_select][tile_index][y * 8 + x];
        *framebuffer_ptr = bg_colors[tile_dat];
        // if (cpu->get_mmap().boot_rom_loaded)
        //     printf("tile_index: %d, y: %d, x: %d, i: %d, scy: %d, sxy: %d\n", tile_dat, y, x, i, scy, scx);
        handle_sprites(sprites, i, tile_dat, framebuffer_ptr, &spr_index);
        framebuffer_ptr++;
        x++;
        if (x == 8) {
            x = 0;
            line_offset = (line_offset + 1) & 31;
            tile_index = vram[0][tile_map_offset + line_offset];
            if (!ctrl.bg_window_tile_data) {
                tile_index = static_cast<uint16_t>(256 + static_cast<uint8_t>(tile_index & 0xFF));
            }
        }
    }
    if (window_active) {
        window_line_active++;
    }
}

void PixelProcessingUnit::render_screen() {
    SDL_RenderPresent(data.renderer);
    SDL_UpdateTexture(data.texture, NULL, framebuffer, 160 * sizeof(uint32_t));
    SDL_RenderClear(data.renderer);
    SDL_RenderCopy(data.renderer, data.texture, NULL, NULL);
    SDL_RenderPresent(data.renderer);
}

bool PixelProcessingUnit::init_window()
{   
    bool success = true;
    // std::cout << "initting window" << std::endl;
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        success = false;
    }
    else
    {
    //     // Create window
        data.window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH * 4, SCREEN_HEIGHT * 4, SDL_WINDOW_SHOWN);
        if (data.window == NULL)
        {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
            success = false;
        }
        else
        {
            data.renderer = SDL_CreateRenderer(data.window, -1, 0);
            if (data.renderer == NULL) {
                printf("renderer could not be created! SDL_Error: %s\n", SDL_GetError());
                return false;
            }
            else {
                data.texture = SDL_CreateTexture(data.renderer, SDL_PIXELFORMAT_ABGR32, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
                if (data.texture == NULL) {
                    printf("texture could not be created! SDL_Error: %s\n", SDL_GetError());
                    return false;
                }
            }
        }
    }
    return success;
}

void PixelProcessingUnit::close() {
    SDL_DestroyTexture(data.texture);
    data.texture = NULL;
    SDL_DestroyRenderer(data.renderer);
    data.renderer = NULL;
    SDL_DestroyWindow(data.window);
    data.window = NULL;
    SDL_Quit();
}

uint8_t PixelProcessingUnit::read_u8_ppu(uint16_t addr) {
    switch (addr) {
        case 0x8000 ... 0x9FFF:
            return vram[vbank_select][addr& 0x1FFF];
        case 0xFF40: 
            return ctrl.val;
        case 0xFF41: 
            return l_status.val;
        case 0xFF42: 
            return scy;
        case 0xFF43: 
            return scx;
        case 0xFF44: 
            return ly;
        case 0xFF45: 
            return lyc;
        case 0xFF46: 
            return dma;
        case 0xFF47: 
            return bg_palette;
        case 0xFF48: 
            return obj_palette_0;
        case 0xFF49: 
            return obj_palette_1;
        case 0xFF4A: 
            return window_x;
        case 0xFF4B: 
            return window_y;
        case 0xFF4F:
            return vbank_select;
        case 0xFF70:
            return wram_bank_select;
        default:
            return 0;
    }
}

void PixelProcessingUnit::write_u8_ppu(uint16_t addr, uint8_t val) {
    switch (addr) {
        case 0x8000 ... 0x9FFF:
            vram[vbank_select][addr& 0x1FFF] = val;
            if (addr < 0x9800) {
                set_tile_data(addr);
            }
            break;
        case 0xFF40:
            ctrl.set(val);
            break;
        case 0xFF41:
            l_status.set(val);
            break;
        case 0xFF42:
            // if (val == 241 && cpu->get_mmap().boot_rom_loaded) {
            //     std::cout << "Hello there" << std::endl;
            //     exit(1);
            // }
            scy = val;
            break;
        case 0xFF43: 
            scx = val;
            break;
        case 0xFF44:
            std::cout << "setting ly" << (uint16_t)val << std::endl;
            ly = val;
            break;
        case 0xFF45:
            lyc = val;
            break;
        case 0xFF46:
            dma = val;
            break;
        case 0xFF47:
            bg_palette = val;
            for (int i = 0; i < 4; i++)
            {
                switch ((val >> (i * 2)) & 3)
                {
                    case 0:
                        bg_colors[i] = 0xFFFFFFFF;
                        break;
                    case 1:
                        bg_colors[i] = 0xAAAAAAFF;
                        break;
                    case 2:
                        bg_colors[i] = 0x555555FF;
                        break;
                    case 3:
                        bg_colors[i] = 0x00000000;
                        break;
                }
            }
            break;
        case 0xFF48: 
            obj_palette_0 = val;
            for (int i = 0; i < 4; i++)
            {
                switch ((val >> (i * 2)) & 3)
                {
                    case 0:
                        obj_0_colors[i] = 0xFFFFFFFF;
                        break;
                    case 1:
                        obj_0_colors[i] = 0xAAAAAAFF;
                        break;
                    case 2:
                        obj_0_colors[i] = 0x555555FF;
                        break;
                    case 3:
                        obj_0_colors[i] = 0x00000000;
                        break;
                }
            }
            break;
        case 0xFF49: 
            obj_palette_1 = val;
            for (int i = 0; i < 4; i++)
            {
                switch ((val >> (i * 2)) & 3)
                {
                    case 0:
                        obj_1_colors[i] = 0xFFFFFFFF;
                        break;
                    case 1:
                        obj_1_colors[i] = 0xAAAAAAFF;
                        break;
                    case 2:
                        obj_1_colors[i] = 0x555555FF;
                        break;
                    case 3:
                        obj_1_colors[i] = 0x00000000;
                        break;
                }
            }
            break;
        case 0xFF4A: 
            window_x = val;
            break;
        case 0xFF4B: 
            window_y = val;
            break;
        case 0xFF4F:
            vbank_select = val;
        case 0xFF70:
            wram_bank_select = val;
        default:
            break;
    }
}

void PixelProcessingUnit::write_oam(uint16_t addr, uint8_t val) {
    oam[addr/4][addr%4] = val;
}

uint8_t PixelProcessingUnit::read_oam(uint16_t addr) {
    return oam[addr/4][addr%4];
}

Sprite PixelProcessingUnit::get_sprite(size_t index) {
    uint8_t y_pos = oam[index][0];
    uint8_t x_pos = oam[index][1];
    uint8_t tile_index = oam[index][2];
    uint8_t attr_flags = oam[index][3];
    return {y_pos, x_pos, tile_index, attr_flags};
}

void PixelProcessingUnit::set_tile_data(uint16_t addr) {
    addr &= 0x1FFE;

    int	tile_index    = (addr >> 4) & 0x1FF;
    int		y   = (addr >> 1) & 7;

	if (tile_index == 384)
		return ;

    for (int x = 0; x < 8; x++)
    {
        unsigned char bitmask = 1 << (7 - x);
        tile_data[vbank_select][tile_index][y * 8 + x] = static_cast<uint8_t>(((vram[vbank_select][addr] & bitmask) ? 1 : 0) +
                                                ((vram[vbank_select][addr + 1] & bitmask) ? 2 : 0));
    }
}

void LCD_CONTROL::set(uint8_t value) {
        lcd_enable =              (value >> 7)	& 1;
        window_tile_map_address = (value >> 6)	& 1;
        window_enable =           (value >> 5)	& 1;
        bg_window_tile_data =     (value >> 4)	& 1;
        bg_tile_map_address =     (value >> 3)	& 1;
        obj_size =                (value >> 2)	& 1;
        obj_enable =              (value >> 1)	& 1;
        bg_enable =               (value >> 0)	& 1;
        val = value;
}

void LCD_STATUS::set(uint8_t value) {
        ly_interrupt =             (value & 0x40) != 0;
        mode_2_oam_interrupt =     (value & 0x20) != 0;
        mode_1_vblank_interrupt =  (value & 0x10) != 0;
        mode_0_hblank_interrupt =  (value & 0x08) != 0;
        ly_flag =                  (value & 0x04) != 0;
        mode =                     value & 0x3;
        val  =                     value;
}