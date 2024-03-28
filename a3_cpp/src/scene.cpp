#include <glm/glm.hpp>
#include <iostream>

#include "scene.hpp"

Scene::Scene(int _w, int _h, Camera& _camera) : w{_w}, h{_h}, camera{_camera}, objects()
{
}

// auto-generated with Claude

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

glm::vec4 Scene::trace_ray(Ray& r, int n_bounces)
{
    HitRecord rec;
    float closest_hit = std::numeric_limits<float>::max();
    bool hit_found = false;
    glm::vec3 hit_color;

    if (n_bounces == 0)
    {
        return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // Return black color if no bounces to do
    }

    for (const auto& obj_rw : objects)
    {
        Object& obj = obj_rw.get();
        if (obj.hit(r, 0.001f, closest_hit, rec))
        {
            hit_color = (rec.normal + glm::vec3(1.f, 1.f, 1.f)) * 0.5f;
            hit_found = true;
            closest_hit = rec.t;

            // // Calculate Blinn-Phong shading
            // glm::vec3 light_dir = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f)); // Assume a single light source at (1,
            // 1, 1) glm::vec3 view_dir = glm::normalize(-r.d); glm::vec3 halfway_vec = glm::normalize(view_dir +
            // light_dir);

            // float diffuse_factor = std::max(0.0f, glm::dot(rec.normal, light_dir));
            // float specular_factor = std::pow(std::max(0.0f, glm::dot(rec.normal, halfway_vec)), 32.0f); // Shininess
            // = 32

            // hit_color = diffuse_factor * glm::vec3(0.8f, 0.8f, 0.8f) + // Diffuse component
            //             specular_factor * glm::vec3(0.5f, 0.5f, 0.5f); // Specular component

            // // Calculate reflection ray
            // glm::vec3 reflectionDir = glm::reflect(-r.d, rec.normal);
            // Ray reflectionRay = { rec.pos, reflectionDir };

            // // Trace reflection ray recursively
            // glm::vec4 reflectionColor = trace_ray(reflectionRay, n_bounces - 1);
            // hit_color += glm::vec3(reflectionColor); // Accumulate reflection color
        }
    }

    if (hit_found)
    {
        return glm::vec4(hit_color, 1.0f);
    }
    else
    {
        return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // Background color
    }
}

// for now
glm::vec4 Scene::trace_path(Ray& ray, int n_bounces)
{
    return trace_ray(ray, n_bounces);
}
