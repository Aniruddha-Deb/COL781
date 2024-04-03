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

    std::shared_ptr<Material> blue_blinn_phong_material = std::make_shared<BlinnPhongMaterial>(
            glm::vec3(0.2f, 0.3f, 0.8f), // k_a
            glm::vec3(0.2f, 0.3f, 0.8f), // k_d
            glm::vec3(0.9f, 0.9f, 0.9f), // k_s
            glm::vec3(0.9f, 0.9f, 0.9f), // k_r
            50);
    std::shared_ptr<Material> red_blinn_phong_material = std::make_shared<BlinnPhongMaterial>(
            glm::vec3(0.7f, 0.3f, 0.5f), // k_a
            glm::vec3(0.7f, 0.3f, 0.5f), // k_d
            glm::vec3(0.8f, 0.3f, 0.4f), // k_s
            glm::vec3(0.2f, 0.2f, 0.1f), // k_r
            2);
    std::shared_ptr<Material> green_blinn_phong_material = std::make_shared<BlinnPhongMaterial>(
            glm::vec3(0.2f, 0.6f, 0.4f), // k_a
            glm::vec3(0.2f, 0.6f, 0.4f), // k_d
            glm::vec3(1.f, 1.f, 1.f), // k_s
            glm::vec3(0.3f, 0.3f, 0.3f), // k_r
            90);
    Window win(WIDTH, HEIGHT, "Raytracer");
    Camera camera(60, glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(1.0f, 2.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    Scene s(WIDTH, HEIGHT, camera, 4);
    Sphere s1(glm::vec3(1.f, 2.f, -4.f), 1.f, blue_blinn_phong_material);
    Sphere s2(glm::vec3(3.f, 1.f, -3.f), 1.f, green_blinn_phong_material);
    AxisAlignedBox b1({glm::vec3(1.f, 2.f, -4.f), glm::vec3(5.f, 3.f, -2.f)}, red_blinn_phong_material);
    s.objects.push_back(s1);
    s.objects.push_back(b1);
    s.objects.push_back(s2);
    LightSource l1(glm::vec3(0.f, 0.f, -3.f), glm::vec3(1.f, 1.f, 1.f));
    LightSource l2(glm::vec3(3.f, 0.f, -0.f), glm::vec3(1.f, .5f, .2f));
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
