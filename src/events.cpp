#include "Cpu.hpp"

void Cpu::event_handler()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
            set_status(true);
        else if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)
        {
            handle_input(e);
        }
    }
    if (ppu.draw_screen) {
        ppu.render_screen();
        ppu.draw_screen = false;
    }
}