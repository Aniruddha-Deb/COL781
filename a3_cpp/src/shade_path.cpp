#include "material.hpp"
#include "scene.hpp"
#include "debug.hpp"
#include "constants.hpp"
#include <cmath>
#include <iostream>
#include <random>
#include <cmath>

// Constants
static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<float> uniform(0.0, 1.0);

glm::vec3 sample_hemisphere_cosine_weighted() {
    float u = uniform(gen);
    float v = uniform(gen);
    float theta = 2 * PI * u;
    float phi = acos(sqrt(1 - v));

    float x = cos(theta) * sin(phi);
    float y = sin(theta) * sin(phi);
    float z = cos(phi);
    return glm::vec3(x, y, z);
}

glm::vec3 trace_path_russian_roulette(Ray& r, Scene& s, float p) {
    if (uniform(gen) <= p)
        return s.trace_path(r) / p;
    return SKY;
}

glm::vec3 EmissiveMaterial::shade(HitRecord& rec, Scene& scene) {
    return emissivity;
}

glm::vec3 NormalMaterial::shade(HitRecord& rec, Scene& scene)
{
    glm::vec3 hit_color = glm::normalize((rec.normal + glm::vec3(1.f, 1.f, 1.f)) * 0.5f);
    return hit_color;
}

glm::vec3 DiffuseMaterial::shade(HitRecord& rec, Scene& scene)
{
    glm::vec3 hit_point = rec.pos;
    Ray ray(rec.pos, sample_hemisphere_cosine_weighted());
    // Hmm, how will we get irradiance at a point? Will require knowing the distance!
    // can get from rec.ray's length, but hmm
    return gamma_restore(gamma_correct(albedo) * gamma_correct(trace_path_russian_roulette(ray, scene, 0.9f)));
}

glm::vec3 BlinnPhongMaterial::shade(HitRecord& rec, Scene& scene)
{
    return SKY;
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
    return glm::min(glm::vec3(1.f, 1.f, 1.f),
                    gamma_restore(R * gamma_correct(scene.trace_ray_rec(reflected_ray, rec.n_bounces_left)) +
                                  refract_color));
}

glm::vec3 MetallicMaterial::shade(HitRecord& rec, Scene& scene)
{

    glm::vec3 n = rec.normal;
    glm::vec3 i = -rec.ray.d;

    Ray reflected_ray(rec.pos, glm::normalize(-i - 2 * glm::dot(-i, n) * n));
    float ndoti = glm::dot(n, i);
    // fresnel formula
    glm::vec3 F = F_0 + (1.f - F_0) * float(glm::pow(1 - ndoti, 5));
    return glm::min(glm::vec3(1.f, 1.f, 1.f),
                    gamma_restore(F * gamma_correct(scene.trace_ray_rec(reflected_ray, rec.n_bounces_left)))
                    );
}
