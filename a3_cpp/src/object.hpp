#pragma once

#include "ray.hpp"
#include "material.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <vector>

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
    void transform(const glm::mat4x4& M);

    ~Object() {}
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
    virtual ~Sphere() {}
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
    virtual ~Plane() {}
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
    virtual ~AxisAlignedBox() {}
};

class Triangle : public Object
{
  public:
    glm::vec3 p0, p1, p2;
    Triangle(glm::vec3 _p0, glm::vec3 _p1, glm::vec3 _p2, std::shared_ptr<Material>& _mat) :
        Object{_mat},
        p0{_p0},
        p1{_p1},
        p2{_p2} {}

    virtual bool hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const;
    virtual Box bounding_box();
    virtual ~Triangle() {}
};

class Mesh : public Object
{
  public:
    std::vector<glm::vec3> verts;
    std::vector<glm::ivec3> idxs;
    Box bbox;
    Mesh(std::shared_ptr<Material>& _mat) :
        Object{_mat} {}

    void load_from_file(std::string objfile_path);
    virtual bool hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const;
    virtual Box bounding_box();
    virtual ~Mesh() {}
};
