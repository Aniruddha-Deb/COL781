#include "window.hpp"
#include <iostream>

Window::Window(int _w, int _h, std::string title): w{_w}, h{_h} {

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cout << "SDL could not initialize. SDL_Error: %s\n" << SDL_GetError() << std::endl;
        return;
    }
    else {
        win = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w,
                                  h, SDL_WINDOW_SHOWN);
        if (win == NULL) {
            std::cout << "Window could not be created. SDL_Error: %s\n" << SDL_GetError() << std::endl;
            return;
        }
    }
}

bool Window::should_quit() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            return true;
        }
    }
    return false;
}

bool Window::blit_surface(SDL_Surface* surf) {

    auto win_surf = SDL_GetWindowSurface(win);
    if (SDL_BlitSurface(surf, NULL, win_surf, NULL) < 0) {
        std::cout << "Could not blit surface to window. SDL_Error: %s\n" << SDL_GetError() << std::endl;
        return false;
    };
    if (SDL_UpdateWindowSurface(win) < 0) {
        std::cout << "Could not update window surface. SDL_Error: %s\n" << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}
