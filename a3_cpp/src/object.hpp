#pragma once

#include "ray.hpp"
#include <glm/glm.hpp>

class Object {

    public:
    virtual bool hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const = 0;
    virtual Box bounding_box() = 0;
    virtual void transform(const glm::mat4x4& M) = 0;
};

class Sphere: public Object {
    public:
    glm::vec3 c;
    float r;
    Sphere(glm::vec3 _c, float _r);

    virtual bool hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const;
    virtual Box bounding_box();
    virtual void transform(const glm::mat4x4& M);
};

class Plane: public Object {
    public:
    glm::vec3 n, pt;
    Plane(glm::vec3& _n, glm::vec3& _pt);

    virtual bool hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const;
    virtual Box bounding_box();
    virtual void transform(const glm::mat4x4& M);
};

class Triangle: public Object {
    public:
    glm::vec3 p0, p1, p2;
    Triangle(glm::vec3& p0, glm::vec3& p1, glm::vec3& p2);

    virtual bool hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const;
    virtual Box bounding_box();
    virtual void transform(const glm::mat4x4& M);
};
