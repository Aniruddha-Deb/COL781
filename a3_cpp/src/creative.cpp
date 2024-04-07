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
#include "constants.hpp"

const glm::vec3 ORIGIN = glm::vec3(-1.f, 0.8f, -3.f);
const glm::vec3 NEG_Z = glm::vec3(4.f, -1.f, -5.f);
// const glm::vec3 ORIGIN = glm::vec3(0.f, 0.f, 0.f);
// const glm::vec3 NEG_Z = glm::vec3(0.f, -0.f, -1.f);
const glm::vec3 POS_Y = glm::vec3(0.f, 1.f, 0.f);

class CornellBoxCamera : public Camera
{
  public:
    CornellBoxCamera() : Camera(60.f, ORIGIN, NEG_Z, POS_Y)
    {
    }
};

class CornellBoxScene : public Scene
{

    std::vector<Triangle> walls;
    std::vector<Sphere> spheres;
    std::vector<AxisAlignedBox> boxes;
    std::vector<LightSource> point_lights;

  public:
    CornellBoxScene(Camera& cam) : Scene(WIN_WIDTH, WIN_HEIGHT, cam, 1)
    {
        std::shared_ptr<Material> light_material = std::make_shared<EmissiveMaterial>(glm::vec3(100.f, 100.f, 100.f));
        std::shared_ptr<Material> white_wall_material = std::make_shared<DiffuseMaterial>(glm::vec3(.5f, .5f, .5f));
        std::shared_ptr<Material> mirror_1 = std::make_shared<MetallicMaterial>(glm::vec3(0.5f, 0.5f, 0.5f));
        std::shared_ptr<Material> mirror_2 = std::make_shared<MetallicMaterial>(glm::vec3(0.2f, 0.1f, 0.1f));
        std::shared_ptr<Material> green_wall_material = std::make_shared<DiffuseMaterial>(glm::vec3(.15f, .4f, .05f));
        std::shared_ptr<Material> red_wall_material = std::make_shared<DiffuseMaterial>(glm::vec3(.3f, .15f, .05f));

        std::shared_ptr<Material> blue_wall_material = std::make_shared<DiffuseMaterial>(glm::vec3(.1f, .2f, .5f));

        std::shared_ptr<Material> mirror_material = std::make_shared<BlinnPhongMaterial>(
            glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 1.f, 1.f), 0);
        std::shared_ptr<Material> glass_material = std::make_shared<TransparentMaterial>(1.5);

        std::shared_ptr<Material> diamond = std::make_shared<TransparentMaterial>(2.33);
        std::shared_ptr<Material> copper_material = std::make_shared<MetallicMaterial>(glm::vec3(0.8f, 0.3f, 0.f));
        std::shared_ptr<Material> mirror_3 = std::make_shared<MetallicMaterial>(glm::vec3(0.4f, 0.8f, 0.4f));

        glm::vec3 verts[12] = {glm::vec3(-2.f, 2.f, -2.f),   glm::vec3(-2.f, -2.f, -2.f), glm::vec3(2.f, -2.f, -2.f),
                               glm::vec3(2.f, 2.f, -2.f),    glm::vec3(-2.f, 2.f, -6.f),  glm::vec3(-2.f, -2.f, -6.f),
                               glm::vec3(2.f, -2.f, -6.f),   glm::vec3(2.f, 2.f, -6.f),   glm::vec3(-.5f, 1.99f, -4.f),
                               glm::vec3(-.5f, 1.99f, -5.f), glm::vec3(.5f, 1.99f, -5.f), glm::vec3(.5f, 1.99f, -4.f)};

        int idxs[14][3] = {{0, 1, 4},               // left wall (red)
                           {1, 5, 4},   {1, 2, 5},  // bottom wall
                           {2, 6, 5},   {2, 7, 6},  // right wall (green)
                           {3, 7, 2},   {3, 4, 7},  // top wall
                           {0, 4, 3},   {7, 4, 5},  // back wall
                           {5, 6, 7},   {8, 9, 10}, // light and front(blue)
                           {8, 10, 11}, {0, 2, 1},  {2, 0, 3}};

        for (int i = 0; i < 14; i++)
        {
            if (i == 0 || i == 1)
            {
                Triangle t(verts[idxs[i][0]], verts[idxs[i][1]], verts[idxs[i][2]], mirror_1);
                walls.push_back(t);
            }
            else if (i == 2 || i == 3)
            {
                Triangle t(verts[idxs[i][0]], verts[idxs[i][1]], verts[idxs[i][2]], green_wall_material);
                walls.push_back(t);
            }
            else if (i == 8 || i == 9)
            {
                Triangle t(verts[idxs[i][0]], verts[idxs[i][1]], verts[idxs[i][2]], white_wall_material);
                walls.push_back(t);
            }
            else if (i == 4 || i == 5)
            {
                Triangle t(verts[idxs[i][0]], verts[idxs[i][1]], verts[idxs[i][2]], mirror_2);
                walls.push_back(t);
            }
            else if (i == 10 || i == 11)
            {
                Triangle t(verts[idxs[i][0]], verts[idxs[i][1]], verts[idxs[i][2]], light_material);
                walls.push_back(t);
            }
            else if (i == 12 || i == 13)
            {
                Triangle t(verts[idxs[i][0]], verts[idxs[i][1]], verts[idxs[i][2]], blue_wall_material);
                walls.push_back(t);
            }
            else
            {
                Triangle t(verts[idxs[i][0]], verts[idxs[i][1]], verts[idxs[i][2]], white_wall_material);
                walls.push_back(t);
            }
        }

        Sphere reflective_sphere = Sphere(glm::vec3(-.75f, -1.25f, -4.7f), .75f, copper_material);
        glm::mat4x4 scale_y_by_2 = glm::scale(glm::mat4x4(1.f), glm::vec3(1.f, 2.f, 1.f));
        reflective_sphere.transform(scale_y_by_2);
        reflective_sphere.transform(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
        Sphere refractive_sphere = Sphere(glm::vec3(.75f, -1.25f, -3.5f), .75f, glass_material);
        spheres.push_back(reflective_sphere);
        spheres.push_back(refractive_sphere);

        Sphere refractive_sphere2 = Sphere(glm::vec3(1.0f, 0.5, -2.5f), 0.5f, diamond);
        AxisAlignedBox reflective_box = AxisAlignedBox(
            {.min_vert = glm::vec3(1.0f, 0.8f, -2.9f), .max_vert = glm::vec3(1.3f, 1.5f, -2.5f)}, red_wall_material);
        boxes.push_back(reflective_box);
        spheres.push_back(refractive_sphere2);

        reflective_box.transform(glm::rotate(glm::mat4(1.0f), glm::radians(10.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
        reflective_box.transform(glm::rotate(glm::mat4(1.0f), glm::radians(-10.0f), glm::vec3(0.0f, 1.0f, 0.0f)));

        objects.insert(objects.end(), walls.begin(), walls.end());
        objects.insert(objects.end(), spheres.begin(), spheres.end());
        objects.insert(objects.end(), boxes.begin(), boxes.end());
        // lights.insert(lights.end(), point_lights.begin(), point_lights.end());
    }
};

void view_cornell_box_scene()
{

    Window win(WIN_WIDTH, WIN_HEIGHT, "Raytracer");
    CornellBoxCamera c;
    CornellBoxScene s(c);
    Renderer renderer(win, s, 5, true);
    renderer.view();
}

int main()
{

    view_cornell_box_scene();

    return 0;
}
