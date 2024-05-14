#include "gbmu.hpp"

bool load_rom(Sdl_Data *data)
{
    bool success = true;
    int img_flags = IMG_INIT_PNG;
    if (!(IMG_Init(img_flags) & img_flags))
    {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        success = false;
    }
    else
    {
        SDL_Surface *loaded_image = IMG_Load("Cannon_Exterior.png");
        if (loaded_image == NULL)
        {
            printf("Unable to load image %s! SDL Error: %s\n", "Cannon_Exterior.png", IMG_GetError());
            success = false;
        }

        data->image = SDL_ConvertSurface(loaded_image, data->surface->format, 0);
        if (data->image == NULL)
        {
            printf("Unable to load image %s! SDL Error: %s\n", "Cannon_Exterior.png", SDL_GetError());
            success = false;
        }
        SDL_FreeSurface(loaded_image);
    }

    return success;
}