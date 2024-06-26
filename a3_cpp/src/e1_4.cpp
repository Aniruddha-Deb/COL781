#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <glm/gtc/matrix_transform.hpp>
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
    Camera camera(60, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    Scene s(WIN_WIDTH, WIN_HEIGHT, camera, RAY_TRACING_DEPTH);
    Sphere s1(glm::vec3(1.f, 2.f, -10.f), 1.f, red_diffuse_material);
    glm::mat4x4 s1_t = glm::scale(glm::mat4x4(1.f), glm::vec3(1.5f, 1.f, 1.f));
    AxisAlignedBox b1({glm::vec3(1.f, 1.f, -6.f), glm::vec3(4.f, 2.f, -4.f)}, green_diffuse_material);
    glm::mat4x4 b1_r = glm::rotate(glm::mat4x4(1.f), glm::radians(60.f), glm::vec3(0.f, 0.f, 1.f));
    s.objects.push_back(s1);
    s.objects.push_back(b1);
    s1.transform(s1_t);
    b1.transform(b1_r);
    LightSource l1(glm::vec3(0.f, 0.f, -1.f), glm::vec3(1.f, 1.f, 1.f), 1.f);
    s.lights.push_back(l1);
    Renderer renderer(win, s);

    renderer.view();
}

int main()
{

    two_sphere_scene();

    return 0;
}
