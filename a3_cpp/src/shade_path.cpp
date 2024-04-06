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

glm::vec3 sample_hemisphere_cosine_weighted()
{
    float u = uniform(gen);
    float v = uniform(gen);
    float phi = 2 * PI * v;
    float z = sqrt(u);
    float theta = acos(z);
    float x = sin(theta) * cos(phi);
    float y = sin(theta) * sin(phi);
    return glm::vec3(x, y, z);
}

glm::vec3 trace_path_russian_roulette(Ray& r, Scene& s, float p)
{
    if (uniform(gen) <= p)
        return s.trace_path(r) / p;
    return SKY;
}

glm::vec3 EmissiveMaterial::shade(HitRecord& rec, Scene& scene)
{
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
    glm::vec3 n = rec.normal;
    glm::vec3 perp1(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 3; i++)
    {
        if (abs(n[i]) > EPS)
        {
            perp1[i] = -n[(i + 1) % 3];
            perp1[(i + 1) % 3] = n[i];
            break;
        }
    }
    perp1 = glm::normalize(perp1);
    glm::vec3 perp2 = glm::cross(n, perp1);
    glm::vec3 sample = sample_hemisphere_cosine_weighted();
    Ray ray(rec.pos, perp2 * sample[0] + perp1 * sample[1] + n * sample[2]);
    return gamma_restore(gamma_correct(albedo) * gamma_correct(trace_path_russian_roulette(ray, scene, 0.90f)));
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
            return trace_path_russian_roulette(reflected_ray, scene, 0.90f);
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

    bool reflect = (uniform(gen) < R);
    if (reflect)
    {
        glm::vec3 reflect_color = gamma_correct(trace_path_russian_roulette(reflected_ray, scene, 0.90f));
        return gamma_restore(reflect_color);
    }
    else
    {
        glm::vec3 refract_color = gamma_correct(trace_path_russian_roulette(refracted_ray, scene, 0.90f));
        return gamma_restore(refract_color);
    }
}

glm::vec3 MetallicMaterial::shade(HitRecord& rec, Scene& scene)
{

    glm::vec3 n = rec.normal;
    glm::vec3 i = -rec.ray.d;

    Ray reflected_ray(rec.pos, glm::normalize(-i - 2 * glm::dot(-i, n) * n));
    float ndoti = glm::dot(n, i);
    // fresnel formula
    glm::vec3 F = F_0 + (1.f - F_0) * float(glm::pow(1 - ndoti, 5));
    return gamma_restore(F * gamma_correct(trace_path_russian_roulette(reflected_ray, scene, 0.90)));
}
