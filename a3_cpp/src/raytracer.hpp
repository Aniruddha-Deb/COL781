#pragma once

#include <SDL2/SDL.h>
#include <vector>

#include "camera.hpp"
#include "object.hpp"

class RayTracer {

    SDL_Surface* framebuffer;
    Camera* camera;
    std::vector<Object*> objects;

    RayTracer(int w, int h);

    void render();
    void view();
};
