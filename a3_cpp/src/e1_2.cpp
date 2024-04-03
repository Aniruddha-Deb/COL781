#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "window.hpp"
#include "renderer.hpp"
#include "material.hpp"

const int WIDTH = 640;
const int HEIGHT = 480;

void two_sphere_scene()
{

    Window win(WIDTH, HEIGHT, "Raytracer");
    std::shared_ptr<Material> normal_material = std::make_shared<NormalMaterial>();
    Camera camera(60, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    Scene s(WIDTH, HEIGHT, camera, 1);
    Sphere s1(glm::vec3(1.f, 2.f, -4.f), 1.f, normal_material);
    Plane p1(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), normal_material);
    AxisAlignedBox b1({glm::vec3(1.f, 2.f, -4.f), glm::vec3(5.f, 3.f, -2.f)}, normal_material);
    s.objects.push_back(s1);
    s.objects.push_back(p1);
    s.objects.push_back(b1);
    Renderer renderer(win, s);

    renderer.view();
}

int main()
{

    two_sphere_scene();

    return 0;
}
