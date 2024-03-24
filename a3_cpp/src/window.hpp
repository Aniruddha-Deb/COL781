#pragma once

#include <SDL2/SDL.h>
#include <string>

class Window {

    public:
    SDL_Window* win;
    int w, h;

    Window(int _w, int _h, std::string title);

    bool should_quit();
    bool blit_surface(SDL_Surface* surf);
};
