#pragma once

#include "ray.hpp"
#include <glm/glm.hpp>

class Object {

    public:
    virtual bool hit(const Ray& r, float t_min, float t_max, HitRecord& rec);
    virtual Box bounding_box();
    virtual void transform(const glm::mat4x4& M);
};

class Sphere: virtual Object {
    float c, r;
    Sphere(float _c, float _r);
};

class Plane: virtual Object {
    glm::vec3 n, pt;
    Plane(glm::vec3& _n, glm::vec3& _pt);
};

class Triangle: virtual Object {
    glm::vec3 p0, p1, p2;
    Triangle(glm::vec3& p0, glm::vec3& p1, glm::vec3& p2);
};
