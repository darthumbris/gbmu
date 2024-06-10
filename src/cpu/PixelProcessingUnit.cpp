#include "PixelProcessingUnit.hpp"
#include <cstring>

PixelProcessingUnit::PixelProcessingUnit()
{
    lcd_clock = 0;
    init_window();
    data.status = false;
    ctrl = {0};
    l_status = {0};
    // oam = {0};
    // std::memset(ctrl, 0, sizeof(ctrl));
    // std::memset(l_status, 0, sizeof(l_status));
    std::memset(oam, 0, sizeof(oam));
    std::memset(tile_data_0, 0, sizeof(tile_data_0));
    std::memset(tile_data_1, 0, sizeof(tile_data_1));
    std::memset(tile_map_0, 0, sizeof(tile_map_0));
    std::memset(tile_map_1, 0, sizeof(tile_map_1));
    std::memset(framebuffer, 0, sizeof(framebuffer));
    // std::cout << "made ppu" << std::endl;
}

PixelProcessingUnit::~PixelProcessingUnit()
{
}

void PixelProcessingUnit::tick(uint8_t cycle)
{
    if (ctrl.lcd_enable) {
        //TODO have a check for the dma (CGB only?) ?
        lcd_clock += (uint16_t)cycle;
        //TODO get this working with the switch case stuff 

        if (lcd_clock >= 456) {
            lcd_clock -= 456;
            ly = (ly + 1) % 154;
            // mmap->increase_lcd_line_y_mod();
            // uint8_t ly = mmap->get_lcd_line_y();
            if (l_status.mode != PPU_Modes::Vertical_Blank && ly >= 144) {
                l_status.mode = PPU_Modes::Vertical_Blank;
                // mmap->set_ppu_mode(PPU_Modes::Vertical_Blank);
                return;
            }
        }
        if (ly < 144) {
            if (lcd_clock <= 80) {
                if (l_status.mode != PPU_Modes::OAM_Scan) {
                    l_status.mode = PPU_Modes::OAM_Scan;
                    // mmap->set_ppu_mode(PPU_Modes::OAM_Scan);
                }
            }
            else if (lcd_clock <= 252) {
                if (l_status.mode != PPU_Modes::Pixel_Drawing) {
                    l_status.mode = PPU_Modes::Pixel_Drawing;
                    // mmap->set_ppu_mode(PPU_Modes::Pixel_Drawing);
                }
            }
            else if (l_status.mode != PPU_Modes::Horizontal_Blank)
                {
                    l_status.mode = PPU_Modes::Horizontal_Blank;
                    // mmap->set_ppu_mode(PPU_Modes::Horizontal_Blank);
                    render_scanline();
                }
        }
        if (ly == 144) {
            render_screen();
        }

        // switch (mmap.get_ppu_mode())
        // {
        // case PPU_Modes::Horizontal_Blank:
        //     if (lcd_clock >= 204) {
        //         lcd_clock -= 204;
        //         // std::cout << "increasing value in 0xFF44 for hor blank" << std::endl;
        //         // mmap.increase_lcd_line_y();
        //         uint8_t ly = mmap.get_lcd_line_y();
        //         if (ly == 144) {
        //             mmap.set_ppu_mode(PPU_Modes::Vertical_Blank);
        //             handle_interrupt(true, mmap);
        //             render_screen(); //Renders the full screen when at the end of the 144 lines
        //         }
        //         else {
        //             mmap.set_ppu_mode(PPU_Modes::OAM_Scan);
        //             handle_interrupt(true, mmap);
        //             if (mmap.get_window_enable() && mmap.get_lcd_window_y() == ly) {
        //                 mmap.set_bg_window_enable_priority(true);
        //             }
        //         }
        //     }
        //     break;
        // case PPU_Modes::Vertical_Blank:
        //     if (lcd_clock >= 456) {
        //         lcd_clock -= 456;
        //         // std::cout << "increasing value in 0xFF44 for vertical blank" << std::endl;
        //         mmap.increase_lcd_line_y();
        //         uint8_t ly = mmap.get_lcd_line_y();
        //         if (ly > 153) {
        //             mmap.reset_lcd_window_y();
        //             mmap.reset_lcd_line_y();
        //             mmap.set_ppu_mode(PPU_Modes::OAM_Scan);
        //             handle_interrupt(true, mmap);
        //             mmap.set_bg_window_enable_priority(mmap.get_window_enable() && mmap.get_lcd_window_y() == ly);
        //         }
        //     }
        //     break;
        // case PPU_Modes::OAM_Scan:
        //     if (lcd_clock >= 80) {
        //         mmap.set_ppu_mode(PPU_Modes::Pixel_Drawing);
        //         lcd_clock -= 80;
        //     }
        //     break;
        // case PPU_Modes::Pixel_Drawing:
        //     // std::cout << "ppu mode: " << (uint16_t)mmap.get_ppu_mode() << ", lcd_clock: " << lcd_clock << ", ly: "<< (uint16_t)mmap.get_lcd_line_y() << std::endl;
        //     if (lcd_clock >= 172) {
        //         mmap.set_ppu_mode(PPU_Modes::Horizontal_Blank);
        //         lcd_clock -= 172;
        //         render_scanline(mmap);
        //         handle_interrupt(false, mmap);
        //     }
        //     break;

        // default:
        //     break;
        // }
    }
    else {
        // std::cout << "lcd off" << std::endl;
    }
}

void PixelProcessingUnit::handle_interrupt(bool val) {
    if (l_status.mode == PPU_Modes::Vertical_Blank) {

    }
    // if (mmap.get_ppu_mode() == PPU_Modes::Vertical_Blank) {

    // }
    // if (mmap.get_ppu_mode() != PPU_Modes::Pixel_Drawing) {

    // }
}

uint8_t PixelProcessingUnit::get_pixel(uint8_t tile_index, uint8_t x, uint8_t y) {
    // uint8_t vbank = vbank_select ? 1 : 0;
    auto t = tile_data_0[tile_index];
    if (vbank_select) {
        t = tile_data_1[tile_index];
    }
    // else {
    //     t = tile_data_0[tile_index];
    // }
    // uint8_t *t = mmap->get_tile_data(vbank, tile_index);
    uint8_t bottom = t[(y * 2 + 1)];
    uint8_t top = t[(y * 2)];
    uint8_t pixel = ((top & (0x80 >> x)) == 0) ? 0x00 : 0b10;
    pixel |= ((bottom & (0x80 >> x)) == 0) ? 0x00 : 0b01;
    // std::cout << "pixel: " << (uint16_t)pixel << std::endl;
    return pixel;
}

//TODO remove the wholl fill_pixels stuff
void PixelProcessingUnit::fill_pixels(std::deque<std::array<uint8_t, 2>> &pixels, uint16_t x, uint8_t n, uint8_t y, uint16_t map_number) {
    uint8_t vbank = vbank_select ? 1 : 0; //TODO if this is correct

    // auto tile_map;

    // if (vbank_select) {
    //     tile_map = tile_map_1;
    // }
    // else {
    //     tile_map = tile_map_0;
    // }

    // uint16_t tile_map = ctrl.bg_tile_map_address ? 0x1c00 : 0x1800;
    for (uint16_t p_x = x; p_x < (x + (uint16_t)n); p_x++) {
        // if (ctrl.window_enable && window_x == 0 + 7) {
        //     tile_map = (ctrl.window_tile_map_address ? 0x1C00 : 0x1800) + ((window_y >> 3) << 5);
        // }
        // uint8_t tile_index = mmap->get_tile_index(vbank, (p_x % 255) / 8, y / 8, tile_map);
        // if (!mmap.get_bg_window_tiles()) {
            //TODO check if this is needed?
            // tile_map = (384 - tile_map as u16) as u8;
        // }
        auto tile_map = tile_map_0[map_number][(p_x % 255) / 8];
        if (vbank_select) {
            tile_map = tile_map_1[map_number][(p_x % 255) / 8];
        }
        // else {
        //     tile_map = tile_map_0[map_number][(p_x % 255) / 8];
        // }
        if (!ctrl.bg_window_tile_data) {
            tile_map = 384 - tile_map;
        }
        

        std::array<uint8_t, 2> pixel; 
        // pixel[0] = get_pixel(tile_index, (p_x % 255) % 8, y % 8, mmap);
        pixel[0] = get_pixel(tile_map, (p_x % 255) % 8, y % 8);
        pixel[1] = 0;
        pixels.push_back(pixel);
    }
}

uint32_t get_color(uint8_t bg_palette, uint8_t palette_id) {
    uint8_t val = 0;
    switch (palette_id) {
        case 0b00:
            val = bg_palette & 0b11;
        break;
        case 0b01:
            val = (bg_palette & 0b1100) >> 2;
        break;
        case 0b10:
            val = (bg_palette & 0b110000) >> 4;
        break;
        case 0b11:
            val = (bg_palette & 0b11000000) >> 6;
        break;
    }
    switch (val) {
        case 0b00:    
            return 0xFFFFFFFF;
        case 0b01:
            return 0xAAAAAAFF;
        case 0b10:    
            return 0x555555FF;
        case 0b11:    
            return 0x00000000;
        default:       
            return 0x00000000;
    }
}

void PixelProcessingUnit::handle_sprites(std::vector<Sprite> sprites, std::deque<std::array<uint8_t, 2>> &pixels, uint32_t x) {
    uint8_t spr_num = 0;
    for (auto spr : sprites) {
            std::cout << "sprites" << std::endl;
                if ((spr.x_pos < 8 && x == 0) || spr.x_pos == x + 8) {
                    uint8_t size = 8;
                    if (spr.x_pos < 8) {
                        size = spr.x_pos;
                    }
                    else if (spr.x_pos >= 160 - 8) {
                        size = 160 - spr.x_pos;
                    }
                    std::cout << "size: " << size << std::endl;
                    std::deque<std::array<uint8_t, 2>> replacement;
                    for (uint8_t y = 0; y < size; y++) {
                        //TODO handle flipx/flipy
                        replacement.push_front(pixels[0]);
                        pixels.pop_front();
                    }
                    for (uint8_t y = 0; y < size; y++) {

                        std::array<uint8_t, 2> item = replacement[0];
                        replacement.pop_front();
                        if (item[1] != 0 || (item[0] != 0 && (spr.att_flags & 0b10000000) == 0)) {
                            pixels.push_front(item);
                        } else {
                            std::cout << "sx: " << spr.x_pos << " sy: " << spr.y_pos << " | x:" << x << " line:" << ly << std::endl;
                            std::array<uint8_t, 2> sprite_pixel;
                            sprite_pixel[0] = get_pixel(spr.tile_index, x - spr.x_pos + y - 8, (ly + 8) - spr.y_pos);
                            sprite_pixel[1] = spr_num + 1;
                            pixels.push_front(sprite_pixel);
                        }
                    }
                }
            spr_num++;
        }
}

void PixelProcessingUnit::render_scanline() {
    std::deque<std::array<uint8_t, 2>> pixels;
    std::vector<Sprite> sprites;

    uint8_t start_y = (ly + scy) % 255;
    uint32_t start_x = scx;
    uint16_t map_number = start_y / 8;
    if (ctrl.bg_tile_map_address) {
        map_number = 64 - start_y / 8;
    }
    fill_pixels(pixels, start_x, 16, start_y, map_number);
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

    uint8_t y = (ly + scy) & 7;
    uint8_t x = scx & 7;

    if (window_x == 7 ) //&& should_display_window
    {
        y = window_y & 7; //TODO should be something else (window_line?)
        x = 0;
    }

    uint32_t *framebuffer_ptr = framebuffer + ly * 160;

    for (uint32_t i = 0; i < 160; i++) {

        // *framebuffer_ptr = bg_colors[get_pixel(map_number, x, y)];
        // std::cout << "bg_color: " << (uint16_t)bg_colors[get_pixel(map_number, x, y)] << std::endl;

        while (pixels.size() <= 8) {
            start_x = (i + scx + 8);
            fill_pixels(pixels, start_x, 8, start_y, map_number);
        }
        handle_sprites(sprites, pixels, i);
        *framebuffer_ptr = get_color(bg_palette, pixels[0][0]);
        // framebuffer[x + ly * 160] = get_color(bg_palette, pixels[0][0]);
        pixels.pop_front();
        framebuffer_ptr++;
        x++;
        if (x==8) {
            x = 0;
            // map_number +=1;
        }
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
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        success = false;
    }
    else
    {
        // Create window
        data.window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (data.window == NULL)
        {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
            success = false;
        }
        else
        {
            data.renderer = SDL_CreateRenderer(data.window, -1, SDL_RENDERER_ACCELERATED);
            if (data.renderer == NULL) {
                printf("renderer could not be created! SDL_Error: %s\n", SDL_GetError());
                return false;
            }
            data.texture = SDL_CreateTexture(data.renderer, SDL_PIXELFORMAT_ABGR32, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
            if (data.texture == NULL) {
                printf("texture could not be created! SDL_Error: %s\n", SDL_GetError());
                return false;
            }
        }
    }
    return success;
}

void PixelProcessingUnit::close() {
    SDL_DestroyTexture(data.texture);

    SDL_DestroyRenderer(data.renderer);

    // Destroy window
    SDL_DestroyWindow(data.window);
    data.window = NULL;

    // Quit SDL subsystems
    SDL_Quit();
}

uint8_t PixelProcessingUnit::read_u8_ppu(uint16_t addr) {
    switch (addr) {
        case 0x000 ... 0x17FF:
            return tile_data_0[(addr / 16)][(addr % 16)];
        case 0x1800 ... 0x2000:
            return tile_map_0[(addr % 0x1800) / 32][(addr % 0x1800) % 32];
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
        case 0x000 ... 0x17FF:
            tile_data_0[(addr / 16)][(addr % 16)] = val;
            break;
        case 0x1800 ... 0x2000:
            tile_map_0[(addr % 0x1800) / 32][(addr % 0x1800) % 32] = val;
            break;
        case 0xFF40:
            ctrl.set(val);
            break;
        case 0xFF41:
            l_status.set(val);
            break;
        case 0xFF42:
            scy = val;
            break;
        case 0xFF43: 
            scx = val;
            break;
        case 0xFF44:
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
                        bg_colors[i] = 0xffe7cdff;
                        break;
                    case 1:
                        bg_colors[i] = 0xe4a39fff;
                        break;
                    case 2:
                        bg_colors[i] = 0x629098ff;
                        break;
                    case 3:
                        bg_colors[i] = 0x4c3457ff;
                        break;
                }
            }
            break;
        case 0xFF48: 
            obj_palette_0 = val;
            break;
        case 0xFF49: 
            obj_palette_1 = val;
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

void LCD_CONTROL::set(uint8_t value) {
        lcd_enable =              (value & 0x80) != 0;
        window_tile_map_address = (value & 0x40) != 0;
        window_enable =           (value & 0x20) != 0;
        bg_window_tile_data =     (value & 0x10) != 0;
        bg_tile_map_address =     (value & 0x08) != 0;
        obj_size =                (value & 0x04) != 0;
        obj_enable =              (value & 0x02) != 0;
        bg_enable =               (value & 0x01) != 0;
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