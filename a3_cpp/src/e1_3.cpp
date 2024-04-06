#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "window.hpp"
#include "renderer.hpp"
#include "constants.hpp"

void two_sphere_scene()
{

    std::shared_ptr<Material> red_diffuse_material = std::make_shared<DiffuseMaterial>(glm::vec3(0.8f, 0.3f, 0.4f));
    std::shared_ptr<Material> green_diffuse_material = std::make_shared<DiffuseMaterial>(glm::vec3(0.3f, 0.8f, 0.4f));
    Window win(WIN_WIDTH, WIN_HEIGHT, "Raytracer");
    Camera camera(60, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 2.0f, -4.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    Scene s(WIN_WIDTH, WIN_HEIGHT, camera, RAY_TRACING_DEPTH);
    LightSource l1(glm::vec3(1.f, 1.f, 1.f), glm::vec3(1.f, 1.f, 1.f), 6.f);

    LightSource l2(glm::vec3(-50.f, 50.f, 50.f), glm::vec3(0.f, 0.f, 1.f), 1000.f);
    Sphere s1(glm::vec3(1.f, 2.f, -4.f), 1.f, green_diffuse_material);
    Plane p1(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), red_diffuse_material);
    AxisAlignedBox b1({glm::vec3(1.f, 2.f, -4.f), glm::vec3(5.f, 3.f, -2.f)}, red_diffuse_material);
    AxisAlignedBox b2({glm::vec3(-1.f, -2.f, -4.f), glm::vec3(1.f, 1.f, -2.f)}, green_diffuse_material);
    s.objects.push_back(s1);
    s.objects.push_back(p1);
    s.objects.push_back(b1);
    s.objects.push_back(b2);
    s.lights.push_back(l1);
    s.lights.push_back(l2);
    Renderer renderer(win, s);

    renderer.view();
}

int main()
{

    two_sphere_scene();

    return 0;
}
