#include <SDL2/SDL.h>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "window.hpp"
#include "debug.hpp"
#include "renderer.hpp"

const int WIDTH = 640;
const int HEIGHT = 480;

const glm::vec3 ORIGIN = glm::vec3(0.f, 0.f, 0.f);
const glm::vec3 NEG_Z = glm::vec3(0.f, 0.f, -1.f);
const glm::vec3 POS_Y = glm::vec3(0.f, 1.f, 0.f);

class CornellBoxCamera : public Camera {
    public:
    CornellBoxCamera() : Camera(60.f, ORIGIN, NEG_Z, POS_Y) {}
};

class CornellBoxScene : public Scene {

    std::vector<Triangle> walls;
    std::vector<LightSource> point_lights;

    public:
    CornellBoxScene(Camera& cam): Scene(WIDTH, HEIGHT, cam, 4) { 
        LightSource l1(glm::vec3(0.f, 0.f, -4.f), glm::vec3(10.f, 10.f, 10.f));
        point_lights.push_back(l1);
        
        std::shared_ptr<Material> white_wall_material = std::make_shared<DiffuseMaterial>(glm::vec3(.9f, .9f, .9f));

        glm::vec3 verts[8] = {
            glm::vec3(2.f, 2.f, -2.f),
            glm::vec3(2.f, -2.f, -2.f),
            glm::vec3(-2.f, -2.f, -2.f),
            glm::vec3(-2.f, 2.f, -2.f),
            glm::vec3(2.f, 2.f, -6.f),
            glm::vec3(2.f, -2.f, -6.f),
            glm::vec3(-2.f, -2.f, -6.f),
            glm::vec3(-2.f, 2.f, -6.f)
        };

        int idxs[10][3] = {
            {0, 1, 4},
            {1, 5, 4},
            {1, 2, 5},
            {2, 6, 5},
            {2, 7, 6},
            {3, 7, 2},
            {3, 4, 7},
            {0, 4, 3},
            {7, 4, 5},
            {5, 6, 7}
        };

        for (int i=0; i<10; i++) {
            Triangle t(verts[idxs[i][0]], verts[idxs[i][1]], verts[idxs[i][2]], white_wall_material);
            walls.push_back(t);
        }

        objects.insert(objects.end(), walls.begin(), walls.end());
        lights.insert(lights.end(), point_lights.begin(), point_lights.end());
    }
};

void view_cornell_box_scene()
{

    Window win(WIDTH, HEIGHT, "Raytracer");
    CornellBoxCamera c;
    CornellBoxScene s(c);
    Renderer renderer(win, s);
    renderer.view();
}

int main()
{

    view_cornell_box_scene();

    return 0;
}
