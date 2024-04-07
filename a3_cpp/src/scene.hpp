#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <functional>
#include "camera.hpp"
#include "object.hpp"
#include "light.hpp"

struct HitRecord;

class Scene
{
  public:
    int w, h;
    Camera camera;
    std::vector<std::reference_wrapper<Object>> objects;
    std::vector<std::reference_wrapper<LightSource>> lights;
    int max_bounces;

    Scene(int _w, int _h, Camera& _camera, int _max_bounces);
    Ray generate_ray(int px, int py, bool jitter=false);
    glm::vec3 trace_ray(Ray& r);
    glm::vec3 trace_path(Ray& r);
    glm::vec3 trace_ray_rec(Ray& r, int n_bounces_left);
};
