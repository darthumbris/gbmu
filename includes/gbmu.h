#ifndef GBMU_H
#define GBMU_H

#include <SDL2/SDL.h>
#include <SDL_image.h>

struct Sdl_Data
{
    SDL_Window *window;
    SDL_Surface *surface;
    SDL_Surface *image;
    bool status;
};

void event_handler(Sdl_Data *data);
void handle_input(Sdl_Data *data, SDL_Event &e);

#endif