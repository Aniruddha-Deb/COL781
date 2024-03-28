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
    Scene s(WIDTH, HEIGHT, camera);
    Sphere s1 = Sphere(glm::vec3(0.f, 0.f, -2.f), 1.f);
    Plane p1 = Plane(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    s.objects.push_back(s1);
    s.objects.push_back(p1);
    Renderer renderer(win, s);

    renderer.view();
}

int main()
{

    two_sphere_scene();

    return 0;
}
