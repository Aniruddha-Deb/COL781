#pragma once

#include "ray.hpp"
#include "material.hpp"
#include <glm/glm.hpp>
#include <memory>

class Object
{

  public:
    std::shared_ptr<Material> mat;
    glm::mat4x4 inv_transform_mat;
    glm::mat3x3 inv_transform_normal_mat;

    Object(std::shared_ptr<Material>& _mat) : 
        mat{_mat}, 
        inv_transform_mat(1.f), 
        inv_transform_normal_mat(1.f) {}

    virtual bool hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const = 0;
    virtual Box bounding_box() = 0;
    virtual void transform(const glm::mat4x4& M);
};

class Sphere : public Object
{
  public:
    glm::vec3 center;
    float radius;
    Sphere(glm::vec3 _c, float _r, std::shared_ptr<Material>& _mat) :
        Object(_mat),
        center{_c},
        radius{_r} {}

    virtual bool hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const;
    virtual Box bounding_box();
    virtual void transform(const glm::mat4x4& M);
};

class Plane : public Object
{
  public:
    glm::vec3 n, pt;
    Plane(glm::vec3 _n, glm::vec3 _pt, std::shared_ptr<Material>& _mat) :
        Object(_mat),
        n{_n},
        pt{_pt} {}

    virtual bool hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const;
    virtual Box bounding_box();
    virtual void transform(const glm::mat4x4& M);
};

class AxisAlignedBox : public Object
{
  public:
    Box box;
    AxisAlignedBox(Box _b, std::shared_ptr<Material>& _mat) :
        Object(_mat),
        box{_b} {}

    virtual bool hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const;
    virtual Box bounding_box();
    virtual void transform(const glm::mat4x4& M);
};

class Triangle : public Object
{
  public:
    glm::vec3 p0, p1, p2;
    glm::vec3 albedo;
    Triangle(glm::vec3& p0, glm::vec3& p1, glm::vec3& p2, glm::vec3 _a);

    virtual bool hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const;
    virtual Box bounding_box();
    virtual void transform(const glm::mat4x4& M);
};
