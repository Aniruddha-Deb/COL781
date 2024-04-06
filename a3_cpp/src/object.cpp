#include <fstream>

#include "object.hpp"
#include "debug.hpp"
#include "iostream"
#include "constants.hpp"

glm::vec3 point_os_to_ws(const glm::vec3& os_pt, const Object& obj)
{
    glm::vec4 ws_pt_hom = glm::inverse(obj.inv_transform_mat) * glm::vec4(os_pt, 1.f);
    return glm::vec3(ws_pt_hom) / ws_pt_hom[3];
}

glm::vec3 point_ws_to_os(const glm::vec3& ws_pt, const Object& obj)
{
    glm::vec4 os_pt_hom = obj.inv_transform_mat * glm::vec4(ws_pt, 1.f);
    return glm::vec3(os_pt_hom) / os_pt_hom[3];
}

glm::vec3 direction_os_to_ws(const glm::vec3& os_dir, const Object& obj)
{
    return glm::vec3(glm::inverse(obj.inv_transform_mat) * glm::vec4(os_dir, 0.f));
}

glm::vec3 direction_ws_to_os(const glm::vec3& ws_dir, const Object& obj)
{
    return glm::vec3(obj.inv_transform_mat * glm::vec4(ws_dir, 0.f));
}

glm::vec3 normal_os_to_ws(const glm::vec3& os_normal, const Object& obj)
{
    return glm::normalize(glm::transpose(obj.inv_transform_normal_mat) * os_normal);
}

Ray ray_os_to_ws(const Ray& os_ray, const Object& obj)
{
    Ray ws_ray = os_ray;
    ws_ray.o = point_os_to_ws(os_ray.o, obj);
    ws_ray.d = glm::normalize(direction_ws_to_os(os_ray.d, obj));
    return ws_ray;
}

Ray ray_ws_to_os(const Ray& ws_ray, const Object& obj)
{
    Ray os_ray = ws_ray;
    os_ray.o = point_ws_to_os(ws_ray.o, obj);
    os_ray.d = glm::normalize(direction_ws_to_os(ws_ray.d, obj));
    return os_ray;
}

void populate_hitrecord(const Ray& ws_ray, const Ray& os_ray, glm::vec3 os_pos, glm::vec3 os_normal,
                        bool ray_originated_in_object, const Object& obj, HitRecord& rec)
{
    // the HitRecord is in world space.
    glm::vec3 ws_pos = point_os_to_ws(os_pos, obj);
    float ws_t = glm::length(ws_pos - ws_ray.o);
    glm::vec3 ws_normal = normal_os_to_ws(os_normal, obj);

    rec.ray = ws_ray;
    rec.pos = ws_pos;
    rec.normal = ws_normal;
    rec.t = ws_t;

    // all transparent objects would be subclasses of TransparentMaterial
    if (TransparentMaterial* transp_mat = dynamic_cast<TransparentMaterial*>(obj.mat.get()))
    {
        if (ray_originated_in_object)
        {
            rec.mu_1 = transp_mat->mu;
            rec.mu_2 = 1.f;
            rec.normal *= -1;
        }
        else
        {
            rec.mu_1 = 1.f;
            rec.mu_2 = transp_mat->mu;
        }
    }
    else
    {
        rec.mu_1 = rec.mu_2 = 1.f;
    }
}

void Object::transform(const glm::mat4x4& M)
{
    inv_transform_mat = glm::inverse(M) * inv_transform_mat;
    inv_transform_normal_mat = glm::inverse(glm::mat3x3(M)) * inv_transform_normal_mat;
}

bool Sphere::hit(const Ray& ws_ray, float t_min, float t_max, HitRecord& rec) const
{
    // std::cout << glm::length(ray.d) << "\n";
    Ray os_ray = ray_ws_to_os(ws_ray, *this);
    glm::vec3 o = os_ray.o, d = os_ray.d;
    glm::vec3 oc = o - center;
    float b = 2.0f * glm::dot(oc, d);
    float c = glm::dot(oc, oc) - radius * radius;
    float discr = b * b - 4 * c;
    if (discr < 0)
        return false;
    float sqrt_discr = sqrt(discr);
    float root = (-b - sqrt_discr) / 2.f;
    if (root < t_min || root > t_max)
    {
        root = (-b + sqrt_discr) / 2.f;
        if (root < t_min || root > t_max)
            return false;
    }

    glm::vec3 os_pos = o + root * d;
    glm::vec3 os_normal = (os_pos - center) / radius;
    bool ray_originated_in_object = (glm::length(o - center) <= radius + 1e-3);

    populate_hitrecord(ws_ray, os_ray, os_pos, os_normal, ray_originated_in_object, *this, rec);
    return true;
}

Box Sphere::bounding_box()
{
    Box box;
    box.tl = center - glm::vec3(radius, radius, radius);
    box.br = center + glm::vec3(radius, radius, radius);
    return box;
}

bool Plane::hit(const Ray& ws_ray, float t_min, float t_max, HitRecord& rec) const
{
    Ray os_ray = ray_ws_to_os(ws_ray, *this);
    glm::vec3 o = os_ray.o, d = os_ray.d;
    float denom = glm::dot(n, d);
    if (glm::abs(denom) < EPS)
        return false;
    float t = glm::dot(pt - o, n) / denom;
    if (t < t_min || t > t_max)
        return false;

    glm::vec3 os_pos = o + t * d;
    populate_hitrecord(ws_ray, os_ray, os_pos, n, false, *this, rec);
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

bool AxisAlignedBox::hit(const Ray& ws_ray, float t_min, float t_max, HitRecord& rec) const
{
    Ray os_ray = ray_ws_to_os(ws_ray, *this);
    glm::vec3 o = os_ray.o, d = os_ray.d;

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

    float os_t;

    // Record the hit information
    if (t_min - EPS <= t_enter && t_enter <= t_max + EPS)
    {
        os_t = t_enter;
    }
    else if (t_min - EPS <= t_exit && t_exit <= t_max + EPS)
    {
        os_t = t_exit;
    }
    else
    {
        return false;
    }

    glm::vec3 os_pos = o + os_t * d;
    glm::vec3 os_normal;

    if (glm::abs(os_pos.x - box.tl.x) < EPS)
        os_normal = glm::vec3(-1, 0, 0);
    else if (glm::abs(os_pos.x - box.br.x) < EPS)
        os_normal = glm::vec3(1, 0, 0);
    else if (glm::abs(os_pos.y - box.tl.y) < EPS)
        os_normal = glm::vec3(0, -1, 0);
    else if (glm::abs(os_pos.y - box.br.y) < EPS)
        os_normal = glm::vec3(0, 1, 0);
    else if (glm::abs(os_pos.z - box.tl.z) < EPS)
        os_normal = glm::vec3(0, 0, -1);
    else if (glm::abs(os_pos.z - box.br.z) < EPS)
        os_normal = glm::vec3(0, 0, 1);

    bool ray_originated_in_object = (box.tl.x - EPS < o.x && box.br.x + EPS > o.x && box.tl.y - EPS < o.y &&
                                     box.br.y + EPS > o.y && box.tl.z - EPS < o.z && box.br.z + EPS > o.z);

    populate_hitrecord(ws_ray, os_ray, os_pos, os_normal, ray_originated_in_object, *this, rec);
    return true;
}

Box AxisAlignedBox::bounding_box()
{
    return box;
}

// Triangle implementation
bool Triangle::hit(const Ray& ws_ray, float t_min, float t_max, HitRecord& rec) const
{
    Ray os_ray = ray_ws_to_os(ws_ray, *this);
    glm::vec3 o = os_ray.o, d = os_ray.d;

    glm::vec3 e1 = p1 - p0;
    glm::vec3 e2 = p2 - p0;
    glm::vec3 pvec = glm::cross(d, e2);
    float det = glm::dot(e1, pvec);
    if (glm::abs(det) < EPS)
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

    glm::vec3 os_pos = o + t * d;
    glm::vec3 os_normal = glm::normalize(glm::cross(e1, e2));
    populate_hitrecord(ws_ray, os_ray, os_pos, os_normal, false, *this, rec);

    return true;
}

Box Triangle::bounding_box()
{
    Box box;
    box.tl = glm::min(glm::min(p0, p1), p2);
    box.br = glm::max(glm::max(p0, p1), p2);
    return box;
}

void Mesh::load_from_file(std::string objfile_path)
{

    std::ifstream objFile(objfile_path);
    if (!objFile.is_open())
    {
        std::cerr << "Unable to open file: " << objfile_path << '\n';
    }
    std::string line;
    while (getline(objFile, line))
    {
        if (line.size() == 0)
            continue;
        std::istringstream iss(line);
        std::string token;
        iss >> token;
        if (token == "#")
            continue;
        if (token == "v")
        {
            glm::vec3 v;
            iss >> v.x >> v.y >> v.z;
            verts.push_back(v);
        }
        else if (token == "f")
        {
            // vertex_index/texture_index/normal_index. Parse.
            glm::ivec3 v;
            int t1, t2;
            std::string e1, e2, e3;
            iss >> e1 >> e2 >> e3;
            sscanf(e1.data(), "%d/%d/%d", &v[0], &t1, &t2);
            sscanf(e2.data(), "%d/%d/%d", &v[1], &t1, &t2);
            sscanf(e3.data(), "%d/%d/%d", &v[2], &t1, &t2);
            v -= 1; // obj files are 1-indexed
            // assuming all the triangles are counter-clockwise
            idxs.push_back(v);
        }
    }
    objFile.close();
}

Box Mesh::bounding_box()
{
    return {glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 0.f)};
}

bool Mesh::hit(const Ray& ws_ray, float t_min, float t_max, HitRecord& rec) const
{
    Ray os_ray = ray_ws_to_os(ws_ray, *this);
    glm::vec3 o = os_ray.o, d = os_ray.d;

    float min_t = std::numeric_limits<float>::max();
    glm::vec3 os_pos, os_normal;
    bool hit = false;

    for (const glm::ivec3& tri : idxs)
    {

        glm::vec3 p0 = verts[tri[0]], p1 = verts[tri[1]], p2 = verts[tri[2]];
        glm::vec3 e1 = p1 - p0;
        glm::vec3 e2 = p2 - p0;
        // std::cout << vec3_to_str(e1) << " " << vec3_to_str(e2) << "\n";
        glm::vec3 pvec = glm::cross(d, e2);
        float det = glm::dot(e1, pvec);
        if (glm::abs(det) < EPS)
            continue;
        float inv_det = 1.0f / det;
        glm::vec3 tvec = o - p0;
        float u = glm::dot(tvec, pvec) * inv_det;
        if (u < 0.0f || u > 1.0f)
            continue;
        glm::vec3 qvec = glm::cross(tvec, e1);
        float v = glm::dot(d, qvec) * inv_det;
        if (v < 0.0f || u + v > 1.0f)
            continue;
        float t = glm::dot(e2, qvec) * inv_det;
        if (t < t_min || t > t_max)
            continue;

        if (t < min_t)
        {
            // we've hit the tri here.
            hit = true;
            os_pos = o + t * d;
            os_normal = glm::normalize(glm::cross(e2, e1));
            min_t = t;
        }
    }

    if (hit)
    {
        populate_hitrecord(ws_ray, os_ray, os_pos, os_normal, false, *this, rec);
        return true;
    }
    return false;
}
