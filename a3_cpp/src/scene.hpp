#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "camera.hpp"
#include "object.hpp"

class Scene {
    public:
    int w, h;
    Camera camera;
    std::vector<Object> objects;

    Scene(int _w, int _h);
    Ray generate_ray(int px, int py);
    glm::vec4 trace_ray(Ray& r, int n_bounces=10);
    glm::vec4 trace_path(Ray& r, int n_bounces=10);
};
