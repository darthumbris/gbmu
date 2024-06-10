#include "Cpu.hpp"

void Cpu::event_handler()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
            set_status(true);
        else if (e.type == SDL_KEYDOWN)
        {
            handle_input(e);
        }
    }
}