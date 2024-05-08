#include "gbmu.h"

void handle_input(Sdl_Data *data, SDL_Event &e)
{
    switch (e.key.keysym.sym)
    {
    case SDLK_ESCAPE:
        data->status = true;
        break;

    default:
        break;
    }
}