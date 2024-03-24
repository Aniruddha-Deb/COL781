#include <glm/glm.hpp>

#include "scene.hpp"

Scene::Scene(int _w, int _h): w{_w}, h{_h}, camera(float(_w)/float(_h)), objects() {}

// auto-generated with Claude

Ray Scene::generate_ray(int px, int py) {
    // Normalize pixel coordinates to the range [-1, 1]
    float u = (2.0f * px) / w - 1.0f;
    float v = 1.0f - (2.0f * py) / h;

    // Calculate the ray direction using the camera view and projection matrices
    glm::vec4 ray_dir_ndc = glm::vec4(u, v, -1.0f, 1.0f);
    glm::vec4 ray_dir = glm::inverse(camera.getProjectionMatrix()) * ray_dir_ndc;
    ray_dir = glm::vec4(ray_dir.x, ray_dir.y, -1.0f, 0.0f);
    ray_dir = glm::inverse(camera.getViewMatrix()) * ray_dir;
    glm::vec3 final_ray_dir = glm::normalize(glm::vec3(ray_dir));

    // Create the ray with the camera position and calculated direction
    Ray ray = { camera.position, final_ray_dir };
    return ray;
}

glm::vec4 Scene::trace_ray(Ray& r, int n_bounces) {
    HitRecord rec;
    float closest_hit = std::numeric_limits<float>::max();
    bool hit_found = false;
    glm::vec3 hit_color;

    if (n_bounces == 0) {
        return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // Return black color if no bounces to do
    }

    for (const auto& obj : objects) {
        if (obj.hit(r, 0.001f, closest_hit, rec)) {
            hit_found = true;
            closest_hit = rec.pos.length();

            // Calculate Blinn-Phong shading
            glm::vec3 light_dir = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f)); // Assume a single light source at (1, 1, 1)
            glm::vec3 view_dir = glm::normalize(-r.d);
            glm::vec3 halfway_vec = glm::normalize(view_dir + light_dir);

            float diffuse_factor = std::max(0.0f, glm::dot(rec.normal, light_dir));
            float specular_factor = std::pow(std::max(0.0f, glm::dot(rec.normal, halfway_vec)), 32.0f); // Shininess = 32

            hit_color = diffuse_factor * glm::vec3(0.8f, 0.8f, 0.8f) + // Diffuse component
                        specular_factor * glm::vec3(0.5f, 0.5f, 0.5f); // Specular component

            // Calculate reflection ray
            glm::vec3 reflectionDir = glm::reflect(-r.d, rec.normal);
            Ray reflectionRay = { rec.pos, reflectionDir };

            // Trace reflection ray recursively
            glm::vec4 reflectionColor = trace_ray(reflectionRay, n_bounces - 1);
            hit_color += glm::vec3(reflectionColor); // Accumulate reflection color
        }
    }

    if (hit_found) {
        return glm::vec4(hit_color, 1.0f);
    } else {
        return glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // Background color
    }
}

// for now
glm::vec4 Scene::trace_path(Ray& ray, int n_bounces) {
    return trace_ray(ray, n_bounces);
}
