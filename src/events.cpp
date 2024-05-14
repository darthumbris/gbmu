#include "gbmu.hpp"

void event_handler(Sdl_Data *data)
{
    SDL_Event e;
    while (!data->status)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                data->status = true;
            else if (e.type == SDL_KEYDOWN)
            {
                handle_input(data, e);
            }
        }
    }
}