#pragma once

#include <SDL2/SDL.h>
#include <vector>

#include "camera.hpp"
#include "object.hpp"
#include "window.hpp"
#include "scene.hpp"

class Renderer {

    SDL_Surface* framebuffer;
    Scene& scene;
    Window& win;
    int spp;

    public:
    Renderer(Window& w, Scene& s, int _spp=1);
    ~Renderer();

    void render();
    void view();
};
