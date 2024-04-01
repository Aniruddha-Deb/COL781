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

    Window win(WIDTH, HEIGHT, "Raytracer");
    Camera camera(60, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    Scene s(WIDTH, HEIGHT, camera, diffuse_shader);
    Sphere s1(glm::vec3(1.f, 2.f, -4.f), 1.f, glm::vec3(0.5f, 0.6f, 0.3f));
    glm::mat4x4 s1_t(1.f);
    s1_t[0][1] = s1_t[1][0] = .5f;
    AxisAlignedBox b1({glm::vec3(3.f, 3.f, -9.f), glm::vec3(-1.f, -1.f, -7.f)}, glm::vec3(0.3f, 0.6f, 0.1f));
    s.objects.push_back(s1);
    // s.objects.push_back(b1);
    s1.transform(s1_t);
    LightSource l1(glm::vec3(0.f, 0.f, -1.f), glm::vec3(1.f, 1.f, 1.f));
    s.lights.push_back(l1);
    Renderer renderer(win, s);

    renderer.view();
}

int main()
{

    two_sphere_scene();

    return 0;
}
