#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <functional>
#include "camera.hpp"
#include "object.hpp"
#include "light.hpp"

class Scene
{
  public:
    int w, h;
    Camera camera;
    std::vector<std::reference_wrapper<Object>> objects;
    std::vector<std::reference_wrapper<LightSource>> lights;
    std::function<glm::vec3(HitRecord&, Scene&)> shader;

    Scene(int _w, int _h, Camera& _camera, std::function<glm::vec3(HitRecord&, Scene&)> _shader);
    Ray generate_ray(int px, int py);
    glm::vec4 trace_ray(Ray& r, int n_bounces = 10);
    glm::vec4 trace_path(Ray& r, int n_bounces = 10);
};

glm::vec3 normal_shader(HitRecord& rec, Scene& scene);
glm::vec3 diffuse_shader(HitRecord& rec, Scene& scene);
