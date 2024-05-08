#include "gbmu.h"

void close(Sdl_Data *data)
{
    SDL_FreeSurface(data->surface);
    data->surface = NULL;
    SDL_FreeSurface(data->image);
    data->image = NULL;

    // Destroy window
    SDL_DestroyWindow(data->window);
    data->window = NULL;

    // Quit SDL subsystems
    SDL_Quit();
}