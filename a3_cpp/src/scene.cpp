#include <glm/glm.hpp>
#include <iostream>

#include "scene.hpp"
#include "debug.hpp"

constexpr glm::vec3 SKY(0.0f, 0.0f, 0.0f);

Scene::Scene(int _w, int _h, Camera& _camera, int _max_bounces)
    : w{_w}, h{_h}, camera{_camera}, objects(), lights(), max_bounces{_max_bounces}
{
}

Ray Scene::generate_ray(int px, int py)
{
    // Normalize pixel coordinates to the range [-1, 1]
    float screen_x = (2.0f * (px + 0.5)) / w - 1.0f;
    float screen_y = 1.0f - (2.0f * (py + 0.5)) / h;

    float camera_x = (screen_x * w * tan(camera.fov / 2.0f)) / h;
    float camera_y = screen_y * tan(camera.fov / 2.0f);

    glm::vec4 camera_point(camera_x, camera_y, -1.0f, 1.0f);

    // Calculate the ray direction using the camera view and projection matrices

    glm::vec4 world_point_homo = glm::inverse(camera.getViewMatrix()) * camera_point;
    glm::vec3 world_point(world_point_homo[0] / world_point_homo[3], world_point_homo[1] / world_point_homo[3],
                          world_point_homo[2] / world_point_homo[3]);

    // std::cout << camera.position[0] << " " << camera.position[1] << " " << camera.position[2] << " camera \n";
    // std::cout << world_point[0] << " " << world_point[1] << " " << world_point[2] << " camera \n";
    // Create the ray with the camera position and calculated direction
    Ray ray = {camera.position, glm::normalize(world_point - camera.position)};
    return ray;
}

glm::vec3 Scene::trace_ray(Ray& r)
{
    return trace_ray_rec(r, max_bounces);
}

glm::vec3 Scene::trace_ray_rec(Ray& r, int n_bounces_left)
{
    if (n_bounces_left == 0)
        return SKY;

    HitRecord rec, closest_hit_rec;
    float closest_hit = std::numeric_limits<float>::max();
    Object* hit_obj = nullptr;
    bool hit_found = false;

    for (const auto& obj_rw : objects)
    {
        Object& obj = obj_rw.get();
        if (obj.hit(r, 0.001f, closest_hit, rec))
        {
            hit_found = true;
            closest_hit_rec = rec;
            closest_hit = rec.t;
            hit_obj = &obj;
        }
    }

    if (hit_found)
    {
        // TODO recurse and trace rays!
        // std::cout << "hit\n";
        closest_hit_rec.ray = r;
        closest_hit_rec.n_bounces_left = n_bounces_left - 1;
        return hit_obj->mat->shade(closest_hit_rec, *this);
    }
    else
    {
        return SKY; // Background color
    }
}

// for now
glm::vec3 Scene::trace_path(Ray& ray)
{
    return trace_ray_rec(ray, max_bounces);
}

glm::vec3 Scene::trace_path_rec(Ray& r, int n_bounces_left)
{
    return trace_ray_rec(r, n_bounces_left);
}
