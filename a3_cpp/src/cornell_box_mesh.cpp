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

const glm::vec3 ORIGIN = glm::vec3(0.f, 0.f, 0.f);
const glm::vec3 NEG_Z = glm::vec3(0.f, 0.f, -1.f);
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
    std::vector<Mesh> meshes;
    std::vector<AxisAlignedBox> boxes;
    std::vector<LightSource> point_lights;

  public:
    CornellBoxScene(Camera& cam) : Scene(WIN_WIDTH, WIN_HEIGHT, cam, RAY_TRACING_DEPTH)
    {
        LightSource l1(glm::vec3(0.f, 1.f, -3.f), glm::vec3(1.f, 1.f, 1.f), 20.f);
        LightSource l2(glm::vec3(0.f, 0.f, -3.f), glm::vec3(1.f, 1.f, 1.f), 20.f);
        // point_lights.push_back(l1);
        point_lights.push_back(l2);

        std::shared_ptr<Material> white_wall_material = std::make_shared<DiffuseMaterial>(glm::vec3(.5f, .5f, .5f));
        std::shared_ptr<Material> green_wall_material = std::make_shared<DiffuseMaterial>(glm::vec3(.15f, .4f, .05f));
        std::shared_ptr<Material> red_wall_material = std::make_shared<DiffuseMaterial>(glm::vec3(.4f, .15f, .05f));
        std::shared_ptr<Material> mirror_material = std::make_shared<BlinnPhongMaterial>(
            glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(1.f, 1.f, 1.f), 0);
        std::shared_ptr<Material> glass_material = std::make_shared<TransparentMaterial>(1.5);
        std::shared_ptr<Material> copper_material = std::make_shared<MetallicMaterial>(glm::vec3(0.8f, 0.3f, 0.f));

        glm::vec3 verts[8] = {
            glm::vec3(-2.f, 2.f, -2.f), glm::vec3(-2.f, -2.f, -2.f), glm::vec3(2.f, -2.f, -2.f),
            glm::vec3(2.f, 2.f, -2.f),  glm::vec3(-2.f, 2.f, -6.f),  glm::vec3(-2.f, -2.f, -6.f),
            glm::vec3(2.f, -2.f, -6.f), glm::vec3(2.f, 2.f, -6.f),
        };

        int idxs[10][3] = {{0, 1, 4},            // left wall (red)
                           {1, 5, 4}, {1, 2, 5}, // bottom wall
                           {2, 6, 5}, {2, 7, 6}, // right wall (green)
                           {3, 7, 2}, {3, 4, 7}, // top wall
                           {0, 4, 3}, {7, 4, 5}, // back wall
                           {5, 6, 7}};

        for (int i = 0; i < 10; i++)
        {
            if (i == 0 || i == 1)
            {
                Triangle t(verts[idxs[i][0]], verts[idxs[i][1]], verts[idxs[i][2]], red_wall_material);
                walls.push_back(t);
            }
            else if (i == 4 || i == 5)
            {
                Triangle t(verts[idxs[i][0]], verts[idxs[i][1]], verts[idxs[i][2]], green_wall_material);
                walls.push_back(t);
            }
            else
            {
                Triangle t(verts[idxs[i][0]], verts[idxs[i][1]], verts[idxs[i][2]], white_wall_material);
                walls.push_back(t);
            }
        }

        Mesh m(green_wall_material);
        // glm::mat4x4 scale = glm::scale(glm::mat4x4(1.f), glm::vec3(2.f, 2.f, 2.f));
        glm::mat4x4 translate = glm::translate(glm::mat4x4(1.f), glm::vec3(0.f, -1.f, -3.5f));
        m.load_from_file("meshes/bunny-1k.obj");
        // m.transform(scale);
        m.transform(translate);
        meshes.push_back(m);
        cdebug << "Verts: " << m.verts.size() << std::endl;
        cdebug << "Faces: " << m.idxs.size() << std::endl;

        objects.insert(objects.end(), walls.begin(), walls.end());
        objects.insert(objects.end(), meshes.begin(), meshes.end());
        lights.insert(lights.end(), point_lights.begin(), point_lights.end());
    }
};

void view_cornell_box_scene()
{

    Window win(WIN_WIDTH, WIN_HEIGHT, "Raytracer");
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
