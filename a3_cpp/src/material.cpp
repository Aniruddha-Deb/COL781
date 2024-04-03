#include "material.hpp"
#include "scene.hpp"
#include "debug.hpp"

#define GAMMA 2.2f

glm::vec3 gamma_correct(glm::vec3 color) {
    return glm::pow(color, glm::vec3(GAMMA, GAMMA, GAMMA));
}

glm::vec3 gamma_restore(glm::vec3 color) {
    return glm::pow(color, glm::vec3(1.f/GAMMA, 1.f/GAMMA, 1.f/GAMMA));
}

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
            hit_color += gamma_correct(albedo) * gamma_correct(light.rgb) * light.intensity * glm::abs(glm::dot(glm::normalize(r.d), glm::normalize(rec.normal))) / r_sq;
        }
    }
    hit_color = glm::min(gamma_restore(hit_color), glm::vec3(1.f, 1.f, 1.f));

    return hit_color;
}

glm::vec3 BlinnPhongMaterial::shade(HitRecord& rec, Scene& scene) {

    glm::vec3 hit_color = gamma_correct(k_a) * 0.05f;
    for (const LightSource& light : scene.lights) {
        glm::vec3 v = glm::normalize(-rec.ray.d);
        glm::vec3 l = glm::normalize(light.pos - rec.pos);
        glm::vec3 h = glm::normalize(v + l);
        glm::vec3 n = rec.normal;
        float r_sq = glm::pow(glm::length(light.pos - rec.pos), 2);
        glm::vec3 light_irradiance = gamma_correct(light.rgb) * light.intensity / r_sq;

        hit_color += gamma_correct(k_d) * light_irradiance * glm::max(0.f, glm::dot(rec.normal, l))
                   + gamma_correct(k_s) * light_irradiance * glm::pow(glm::max(0.f, glm::dot(rec.normal, h)), p);
    }
    glm::vec3 tangent_vector = glm::normalize(glm::cross(glm::cross(rec.ray.d, rec.normal), -rec.normal));
    Ray reflected_ray{rec.pos, tangent_vector * glm::dot(rec.ray.d, tangent_vector) - rec.normal * glm::dot(rec.ray.d, rec.normal)};
    hit_color += gamma_correct(k_r) * gamma_correct(scene.trace_ray_rec(reflected_ray, rec.n_bounces_left));

    hit_color = glm::min(glm::vec3(1.f, 1.f, 1.f), gamma_restore(hit_color));

    return hit_color;
}

glm::vec3 TransparentMaterial::shade(HitRecord& rec, Scene& scene) {

}
