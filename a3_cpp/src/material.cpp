#include "material.hpp"
#include "scene.hpp"
#include "debug.hpp"

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
        else {
            float r_sq = glm::pow(glm::length(light.pos - hit_point), 2);
            // cdebug << r_sq << std::endl;
            hit_color += albedo * light.rgb * glm::abs(glm::dot(glm::normalize(r.d), glm::normalize(rec.normal))); // / r_sq;
            // cdebug << vec3_to_str(hit_color) << std::endl;
        }
    }

    return hit_color;
}

glm::vec3 BlinnPhongMaterial::shade(HitRecord& rec, Scene& scene) {

    glm::vec3 hit_color = k_a * 0.05f;
    for (const LightSource& light : scene.lights) {
        glm::vec3 v = glm::normalize(-rec.ray.d);
        glm::vec3 l = glm::normalize(light.pos - rec.pos);
        glm::vec3 h = glm::normalize(v + l);
        glm::vec3 n = rec.normal;

        hit_color += k_d * light.rgb * glm::max(0.f, glm::dot(rec.normal, l))
                   + k_s * light.rgb * glm::pow(glm::max(0.f, glm::dot(rec.normal, h)), p);
    }
    glm::vec3 tangent_vector = glm::normalize(glm::cross(glm::cross(rec.ray.d, rec.normal), -rec.normal));
    Ray reflected_ray{rec.pos, tangent_vector * glm::dot(rec.ray.d, tangent_vector) - rec.normal * glm::dot(rec.ray.d, rec.normal)};
    hit_color += k_r * scene.trace_ray_rec(reflected_ray, rec.n_bounces_left);

    hit_color = glm::min(glm::vec3(1.f, 1.f, 1.f), hit_color);

    return hit_color;
}

glm::vec3 TransparentMaterial::shade(HitRecord& rec, Scene& scene) {

}
