#pragma once

#include "ray.hpp"

class Scene;

glm::vec3 gamma_correct(glm::vec3 color);
glm::vec3 gamma_restore(glm::vec3 color);

class Material {
    public:
    virtual glm::vec3 shade(HitRecord& rec, Scene& scene) = 0;
    virtual ~Material() {}
};

class EmissiveMaterial : public Material {
    public:
    glm::vec3 emissivity;
    EmissiveMaterial(glm::vec3 _emissivity): emissivity{_emissivity} {}
    glm::vec3 shade(HitRecord& rec, Scene& scene);
};

class DiffuseMaterial : public Material {
    public:
    glm::vec3 albedo;
    DiffuseMaterial(glm::vec3 _albedo) : albedo(_albedo) {}
    glm::vec3 shade(HitRecord& rec, Scene& scene);
};

class NormalMaterial : public Material {
    public:
    glm::vec3 shade(HitRecord& rec, Scene& scene);
};

class BlinnPhongMaterial : public Material {
    public:
    glm::vec3 k_a, k_d, k_s, k_r;
    float p;
    BlinnPhongMaterial(glm::vec3 _k_a, glm::vec3 _k_d, glm::vec3 _k_s, glm::vec3 _k_r, float _p) : 
        k_a{_k_a}, k_d{_k_d}, k_s{_k_s}, k_r{_k_r}, p{_p} {}
    glm::vec3 shade(HitRecord& rec, Scene& scene);
};

class TransparentMaterial : public Material {
    public:
    float mu;
    TransparentMaterial(float _mu) : mu{_mu} {}
    glm::vec3 shade(HitRecord& rec, Scene& scene);
};

class MetallicMaterial : public Material {
    public:
    glm::vec3 F_0;

    MetallicMaterial(glm::vec3 _F_0) : F_0{_F_0} {}
    glm::vec3 shade(HitRecord& rec, Scene& scene);
};
