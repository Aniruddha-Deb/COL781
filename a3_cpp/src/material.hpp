#pragma once

#include "ray.hpp"

class Scene;

class Material {
    public:
    virtual glm::vec3 shade(HitRecord& rec, Scene& scene) = 0;
};

class DiffuseMaterial : public Material {
    public:
    glm::vec3 albedo;
    DiffuseMaterial(glm::vec3 _albedo) : albedo{_albedo} {}
    glm::vec3 shade(HitRecord& rec, Scene& scene);
};

class NormalMaterial : public Material {
    glm::vec3 shade(HitRecord& rec, Scene& scene);
};

class BlinnPhongMaterial : public Material {
    glm::vec3 shade(HitRecord& rec, Scene& scene);
};

class TransparentMaterial : public Material {
    glm::vec3 shade(HitRecord& rec, Scene& scene);
};
