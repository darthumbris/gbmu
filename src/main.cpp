#include <SDL2/SDL.h>
#include <stdio.h>
#include <fstream>
#include "graphics.hpp"
#include "gbmu.hpp"
#include "rom.hpp"
#include "Decoder.hpp"
#include "Cpu.hpp"
#include <cstddef>
#include <iostream>

int main(int argc, char *argv[])
{
    Sdl_Data data;
    data.status = false;
    std::cout << "trying to open: " << argv[1] << std::endl;
    std::string path = argv[1];
    Rom rom = Rom(path);
    rom.print_rom();

    std::string p = "Opcodes.json";
    Dict::Decoder decode = Dict::Decoder(p);

    decode.set_data(path);

    std::cout << "decoder rom size: " << decode.get_data().size() << std::endl;
    decode.disassemble(0x150, 32);

    Cpu cpu = Cpu(decode, path);

    cpu.tick();

    if (!init_window(&data))
    {
        printf("Failed to initialize window");
    }
    else
    {
        if (!load_rom(&data))
        {
            printf("Failed to load rom");
        }
        else
        {
            SDL_BlitSurface(data.image, NULL, data.surface, NULL);
            SDL_UpdateWindowSurface(data.window);

            event_handler(&data);
        }
    }

    close(&data);

    return 0;
}