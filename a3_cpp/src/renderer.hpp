#pragma once

#include <SDL2/SDL.h>
#include <vector>

#include "camera.hpp"
#include "object.hpp"
#include "window.hpp"
#include "scene.hpp"

class Renderer
{

    SDL_Surface* framebuffer;
    std::vector<glm::vec3> samplebuffer;
    int curr_sample_no;
    Scene& scene;
    Window& win;
    int spp;
    bool path_traced;

  public:
    Renderer(Window& w, Scene& s, int _spp = 1, bool path_traced = false);
    ~Renderer();

    void render();
    void view();

  private:
    void pan(float deltaPosX, float deltaPosY);
    void move(float delta);
    std::pair<int, int> handle_events(int lastxPos, int lastyPos);
    void reset_samplebuffer();
};
