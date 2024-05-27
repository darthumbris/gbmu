#include "Cpu.hpp"

void Cpu::handle_input(SDL_Event &e)
{
    switch (e.key.keysym.sym)
    {
    case SDLK_ESCAPE:
        ppu.set_status(true);
        break;

    default:
        break;
    }
}