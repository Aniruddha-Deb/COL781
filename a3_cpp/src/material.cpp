#include "material.hpp"
#include "scene.hpp"

glm::vec3 NormalMaterial::shade(HitRecord& rec, Scene& scene) {
    glm::vec3 hit_color = glm::normalize((rec.normal + glm::vec3(1.f, 1.f, 1.f)) * 0.5f);
    return hit_color;
}

glm::vec3 DiffuseMaterial::shade(HitRecord& rec, Scene& scene) {
    glm::vec3 hit_point = rec.pos;
    glm::vec3 hit_color(0.f, 0.f, 0.f);
    HitRecord dummy;
    for (const LightSource& light : scene.lights) {
        Ray r = {hit_point, light.pos - hit_point};
        bool hit_object = false;
        for (const Object& obj : scene.objects) {
            if (obj.hit(r, 0.001, 1, dummy)) {
                hit_object = true;
            }
        }
        if (hit_object) continue;
        else hit_color += albedo * light.rgb * glm::dot(glm::normalize(r.d), rec.normal);
    }

    return hit_color;
}
