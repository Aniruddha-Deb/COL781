#include "material.hpp"
#include "scene.hpp"
#include "debug.hpp"
#include <cmath>

glm::vec3 NormalMaterial::shade(HitRecord& rec, Scene& scene)
{
    glm::vec3 hit_color = glm::normalize((rec.normal + glm::vec3(1.f, 1.f, 1.f)) * 0.5f);
    return hit_color;
}

glm::vec3 EmissiveMaterial::shade(HitRecord& rec, Scene& scene) {
    return emissivity;
}

glm::vec3 DiffuseMaterial::shade(HitRecord& rec, Scene& scene)
{
    glm::vec3 hit_point = rec.pos;
    glm::vec3 hit_color(0.f, 0.f, 0.f);
    HitRecord dummy;
    for (const LightSource& light : scene.lights)
    {
        Ray r(hit_point, light.pos - hit_point);
        bool hit_object = false;
        for (const Object& obj : scene.objects)
        {
            if (obj.hit(r, 0.001, 1, dummy))
            {
                hit_object = true;
            }
        }
        if (hit_object)
            continue;
        else
        {
            float r_sq = glm::pow(glm::length(light.pos - hit_point), 2);
            hit_color += gamma_correct(albedo) * gamma_correct(light.rgb) * light.intensity *
                         glm::abs(glm::dot(glm::normalize(r.d), glm::normalize(rec.normal))) / r_sq;
        }
    }

    return gamma_restore(hit_color);
}

glm::vec3 BlinnPhongMaterial::shade(HitRecord& rec, Scene& scene)
{

    glm::vec3 hit_color = gamma_correct(k_a) * 0.05f;
    for (const LightSource& light : scene.lights)
    {
        glm::vec3 v = glm::normalize(-rec.ray.d);
        glm::vec3 l = glm::normalize(light.pos - rec.pos);
        glm::vec3 h = glm::normalize(v + l);
        glm::vec3 n = rec.normal;
        float r_sq = glm::pow(glm::length(light.pos - rec.pos), 2);
        glm::vec3 light_irradiance = gamma_correct(light.rgb) * light.intensity / r_sq;

        hit_color += gamma_correct(k_d) * light_irradiance * glm::max(0.f, glm::dot(rec.normal, l)) +
                     gamma_correct(k_s) * light_irradiance * glm::pow(glm::max(0.f, glm::dot(rec.normal, h)), p);
    }
    Ray reflected_ray(rec.pos, rec.ray.d - 2 * glm::dot(rec.ray.d, rec.normal) * rec.normal);
    hit_color += gamma_correct(k_r) * gamma_correct(scene.trace_ray_rec(reflected_ray, rec.n_bounces_left));

    return gamma_restore(hit_color);
}

glm::vec3 TransparentMaterial::shade(HitRecord& rec, Scene& scene)
{

    glm::vec3 n = rec.normal;
    glm::vec3 i = -rec.ray.d;

    Ray reflected_ray(rec.pos, glm::normalize(-i - 2 * glm::dot(-i, n) * n));
    // compute critical angle
    if (rec.mu_2 < rec.mu_1)
    {
        float critical_angle = asinf(rec.mu_2 / rec.mu_1);
        if (acosf(glm::dot(i, rec.normal)) > critical_angle)
        {
            // total internal reflection.
            return scene.trace_ray_rec(reflected_ray, rec.n_bounces_left);
        }
    }
    float mu_r = rec.mu_1 / rec.mu_2;
    float ndoti = glm::dot(n, i);
    Ray refracted_ray(rec.pos,
                      glm::normalize((mu_r * ndoti - glm::sqrt(1 - mu_r * mu_r * (1 - ndoti * ndoti))) * n - mu_r * i));

    // fresnel formula
    float R_0 = glm::pow((rec.mu_1 - rec.mu_2) / (rec.mu_1 + rec.mu_2), 2);
    float cos_theta_max = glm::max(ndoti, glm::dot(refracted_ray.d, -n));


    float R = R_0 + (1 - R_0) * glm::pow(1 - cos_theta_max, 5);
    glm::vec3 refract_color = (1 - R) * gamma_correct(scene.trace_ray_rec(refracted_ray, rec.n_bounces_left));
    return gamma_restore(R * gamma_correct(scene.trace_ray_rec(reflected_ray, rec.n_bounces_left)) + refract_color);
}

glm::vec3 MetallicMaterial::shade(HitRecord& rec, Scene& scene)
{

    glm::vec3 n = rec.normal;
    glm::vec3 i = -rec.ray.d;

    Ray reflected_ray(rec.pos, glm::normalize(-i - 2 * glm::dot(-i, n) * n));
    float ndoti = glm::dot(n, i);
    // fresnel formula
    glm::vec3 F = F_0 + (1.f - F_0) * float(glm::pow(1 - ndoti, 5));
    return gamma_restore(F * gamma_correct(scene.trace_ray_rec(reflected_ray, rec.n_bounces_left)));
}
