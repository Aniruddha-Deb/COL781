#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "window.hpp"
#include "renderer.hpp"

const int WIDTH = 640;
const int HEIGHT = 480;

void two_sphere_scene()
{

    std::shared_ptr<Material> red_diffuse_material = std::make_shared<DiffuseMaterial>(glm::vec3(0.8f, 0.3f, 0.4f));
    std::shared_ptr<Material> green_diffuse_material = std::make_shared<DiffuseMaterial>(glm::vec3(0.3f, 0.8f, 0.4f));
    Window win(WIDTH, HEIGHT, "Raytracer");
    Camera camera(60, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    Scene s(WIDTH, HEIGHT, camera, 1);
    Sphere s1(glm::vec3(1.f, 2.f, -4.f), 1.f, red_diffuse_material);
    AxisAlignedBox b1({glm::vec3(1.f, 2.f, -4.f), glm::vec3(5.f, 3.f, -2.f)}, green_diffuse_material);
    s.objects.push_back(s1);
    s.objects.push_back(b1);
    LightSource l1(glm::vec3(0.f, 0.f, -3.f), glm::vec3(1.f, 1.f, 1.f));
    s.lights.push_back(l1);
    Renderer renderer(win, s);

    renderer.view();
}

int main()
{

    two_sphere_scene();

    return 0;
}
