#include "object.hpp"
#include "debug.hpp"
#include "iostream"

Ray transform_ray(const Ray& ray, const glm::mat4x4& transform_mat)
{
    Ray transformed_ray = ray;
    glm::vec4 o_t = transform_mat * glm::vec4(transformed_ray.o, 1.f);
    transformed_ray.o = glm::vec3(o_t) / o_t.w;
    glm::vec4 d_t = transform_mat * glm::vec4(transformed_ray.d, 0.f);
    transformed_ray.d = glm::normalize(glm::vec3(d_t)); /// d_t.w;
    return transformed_ray;
}

glm::mat3x3 mat4tomat3(const glm::mat4x4& mat4)
{
    glm::mat3x3 mat3;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            mat3[i][j] = mat4[i][j];
        }
    }
    return mat3;
}

void Object::transform(const glm::mat4x4& M)
{
    inv_transform_mat = glm::inverse(M) * inv_transform_mat;
    inv_transform_normal_mat = glm::inverse(glm::mat3x3(M)) * inv_transform_normal_mat;
}

bool Sphere::hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const
{
    Ray transformed_ray = transform_ray(ray, inv_transform_mat);
    glm::vec3 o = transformed_ray.o, d = transformed_ray.d;
    glm::vec3 oc = o - center;
    float a = glm::dot(d, d);
    float b = 2.0f * glm::dot(oc, d);
    float c = glm::dot(oc, oc) - radius * radius;
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0)
        return false;
    float sqrt_discriminant = sqrt(discriminant);
    float root = (-b - sqrt_discriminant) / (2.0f * a);
    if (root < t_min || root > t_max)
    {
        root = (-b + sqrt_discriminant) / (2.0f * a);
        if (root < t_min || root > t_max)
            return false;
    }
    if (TransparentMaterial* transp_mat = dynamic_cast<TransparentMaterial*>(mat.get())) {
        if (glm::length(ray.o - center) + 1e-3 <= radius) {
            // ray originated in the sphere
            rec.mu_1 = transp_mat->mu;
            rec.mu_2 = 1.f;
        }
        else {
            rec.mu_1 = 1.f;
            rec.mu_2 = transp_mat->mu;
        }
    }
    rec.pos = ray.o + root * ray.d;
    rec.normal = (rec.pos - center) / radius;
    rec.normal = glm::transpose(inv_transform_normal_mat) * rec.normal;
    rec.t = root;
    return true;
}

Box Sphere::bounding_box()
{
    Box box;
    box.tl = center - glm::vec3(radius, radius, radius);
    box.br = center + glm::vec3(radius, radius, radius);
    return box;
}

void Sphere::transform(const glm::mat4x4& M)
{
    Object::transform(M);

    // Apply the transformation matrix to the center and radius
    glm::vec4 center_homo = M * glm::vec4(center, 1.0f);
    center = glm::vec3(center_homo) / center_homo.w;
    // Calculate the new radius by transforming a point on the sphere
    glm::vec4 point = M * glm::vec4(center + glm::vec3(radius, 0, 0), 1.0f);
    radius = glm::length(glm::vec3(point) / point.w - center);
}

bool Plane::hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const
{
    Ray transformed_ray = transform_ray(ray, inv_transform_mat);
    glm::vec3 o = transformed_ray.o, d = transformed_ray.d;
    float denom = glm::dot(n, d);
    // std::cout << denom << " " << glm::abs(denom) << " denom \n";
    if (glm::abs(denom) < 1e-8)
        return false;
    float t = glm::dot(pt - o, n) / denom;
    if (t < t_min || t > t_max)
        return false;
    rec.pos = o + t * d;
    rec.normal = glm::transpose(glm::mat3x3(inv_transform_normal_mat)) * n;
    rec.t = t;
    return true;
}

Box Plane::bounding_box()
{
    Box box;
    // Assume the plane is infinite, so return a large bounding box
    box.tl = glm::vec3(-1e9, -1e9, -1e9);
    box.br = glm::vec3(1e9, 1e9, 1e9);
    return box;
}

void Plane::transform(const glm::mat4x4& M)
{
    Object::transform(M);
    // Apply the transformation matrix to the normal and point
    glm::vec4 normal = M * glm::vec4(n, 0.0f);
    n = glm::vec3(normal);
    glm::vec4 point = M * glm::vec4(pt, 1.0f);
    pt = glm::vec3(point) / point.w;
}

bool AxisAlignedBox::hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const
{
    Ray transformed_ray = transform_ray(ray, inv_transform_mat);
    glm::vec3 o = transformed_ray.o, d = transformed_ray.d;

    // Check if the ray is parallel to any axis
    // TODO fix
    if (glm::abs(d.x) < 1e-6 || glm::abs(d.y) < 1e-6 || glm::abs(d.z) < 1e-6)
        return false;

    glm::vec3 inv_direction = 1.0f / d;

    glm::vec3 t0 = (box.tl - o) * inv_direction;
    glm::vec3 t1 = (box.br - o) * inv_direction;

    glm::vec3 tmin = glm::min(t0, t1);
    glm::vec3 tmax = glm::max(t0, t1);

    float t_enter = glm::max(glm::max(tmin.x, tmin.y), tmin.z);
    float t_exit = glm::min(glm::min(tmax.x, tmax.y), tmax.z);

    // Check for edge cases
    if (t_exit < 0 || t_enter > t_exit || t_enter > t_max || t_exit < t_min)
        return false;

    // Record the hit information
    rec.t = t_enter;
    rec.pos = o + rec.t * d;

    if (glm::abs(rec.pos.x - box.tl.x) < 1e-6)
        rec.normal = glm::vec3(-1, 0, 0);
    else if (glm::abs(rec.pos.x - box.br.x) < 1e-6)
        rec.normal = glm::vec3(1, 0, 0);
    else if (glm::abs(rec.pos.y - box.tl.y) < 1e-6)
        rec.normal = glm::vec3(0, -1, 0);
    else if (glm::abs(rec.pos.y - box.br.y) < 1e-6)
        rec.normal = glm::vec3(0, 1, 0);
    else if (glm::abs(rec.pos.z - box.tl.z) < 1e-6)
        rec.normal = glm::vec3(0, 0, -1);
    else if (glm::abs(rec.pos.z - box.br.z) < 1e-6)
        rec.normal = glm::vec3(0, 0, 1);

    // transform normal
    rec.normal = glm::transpose(glm::mat3x3(inv_transform_normal_mat)) * rec.normal;

    return true;
}

Box AxisAlignedBox::bounding_box()
{
    return box;
}

void AxisAlignedBox::transform(const glm::mat4x4& M)
{
    Object::transform(M);
    box.tl = glm::vec3(M * glm::vec4(box.tl, 1.0f));
    box.br = glm::vec3(M * glm::vec4(box.br, 1.0f));
}

// Triangle implementation
bool Triangle::hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const
{
    Ray transformed_ray = transform_ray(ray, inv_transform_mat);
    glm::vec3 o = transformed_ray.o, d = transformed_ray.d;
    glm::vec3 e1 = p1 - p0;
    glm::vec3 e2 = p2 - p0;
    glm::vec3 pvec = glm::cross(d, e2);
    float det = glm::dot(e1, pvec);
    if (glm::abs(det) < 1e-6)
        return false;
    float inv_det = 1.0f / det;
    glm::vec3 tvec = o - p0;
    float u = glm::dot(tvec, pvec) * inv_det;
    if (u < 0.0f || u > 1.0f)
        return false;
    glm::vec3 qvec = glm::cross(tvec, e1);
    float v = glm::dot(d, qvec) * inv_det;
    if (v < 0.0f || u + v > 1.0f)
        return false;
    float t = glm::dot(e2, qvec) * inv_det;
    if (t < t_min || t > t_max)
        return false;
    rec.pos = o + t * d;
    rec.normal = glm::transpose(glm::mat3x3(inv_transform_normal_mat)) * glm::normalize(glm::cross(e1, e2));
    rec.t = t;
    return true;
}

Box Triangle::bounding_box()
{
    Box box;
    box.tl = glm::min(glm::min(p0, p1), p2);
    box.br = glm::max(glm::max(p0, p1), p2);
    return box;
}

void Triangle::transform(const glm::mat4x4& M)
{
    Object::transform(M);
    // Apply the transformation matrix to the vertices
    glm::vec4 v0 = M * glm::vec4(p0, 1.0f);
    p0 = glm::vec3(v0) / v0.w;
    glm::vec4 v1 = M * glm::vec4(p1, 1.0f);
    p1 = glm::vec3(v1) / v1.w;
    glm::vec4 v2 = M * glm::vec4(p2, 1.0f);
    p2 = glm::vec3(v2) / v2.w;
}
