#include <fstream>

#include "object.hpp"
#include "debug.hpp"
#include "iostream"
#include "constants.hpp"

struct HitInfo {
    glm::vec3 os_pos;
    glm::vec3 os_normal;
    float t;
};

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
    bool ray_originated_in_object = (glm::length(o - center) <= radius + 1e-3);
    glm::vec3 oc = o - center;
    float b = 2.0f * glm::dot(oc, d);
    float c = glm::dot(oc, oc) - radius * radius;
    float discr = b * b - 4 * c;
    if (discr < 0)
        return false;
    float sqrt_discr = sqrt(discr);
    float root = (-b - sqrt_discr) / 2.f;
    if (ray_originated_in_object) root = (-b + sqrt_discr) / 2.f;
    if (root < 0) return false;

    glm::vec3 os_pos = o + root * d;
    glm::vec3 os_normal = (os_pos - center) / radius;

    populate_hitrecord(ws_ray, os_ray, os_pos, os_normal, ray_originated_in_object, *this, rec);
    if (rec.t < t_min || rec.t > t_max) return false;
    return true;
}

Box Sphere::bounding_box()
{
    Box box;
    box.min_vert = center - glm::vec3(radius, radius, radius);
    box.max_vert = center + glm::vec3(radius, radius, radius);
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
    if (t < 0) return false;

    glm::vec3 os_pos = o + t * d;
    populate_hitrecord(ws_ray, os_ray, os_pos, n, false, *this, rec);
    if (rec.t - EPS < t_min || rec.t + EPS > t_max) return false;
    return true;
}

Box Plane::bounding_box()
{
    Box box;
    // Assume the plane is infinite, so return a large bounding box
    box.min_vert = glm::vec3(-1e9, -1e9, -1e9);
    box.max_vert = glm::vec3(1e9, 1e9, 1e9);
    return box;
}

bool aabb_hit_test(const Ray& os_ray, bool ray_originated_in_object, const Box& box, HitInfo& info) {

    glm::vec3 o = os_ray.o, d = os_ray.d;
    glm::vec3 inv_direction = 1.0f / d;

    glm::vec3 t0 = (box.min_vert - o) * inv_direction;
    glm::vec3 t1 = (box.max_vert - o) * inv_direction;

    glm::vec3 tmin = glm::min(t0, t1);
    glm::vec3 tmax = glm::max(t0, t1);

    float t_enter = glm::max(glm::max(tmin.x, tmin.y), tmin.z);
    float t_exit = glm::min(glm::min(tmax.x, tmax.y), tmax.z);

    if (t_exit < 0 || t_enter < 0 || t_enter > t_exit) return false;

    float os_t;

    if (ray_originated_in_object) os_t = t_exit;
    else os_t = t_enter;

    glm::vec3 os_pos = o + os_t * d;
    glm::vec3 os_normal;

    if (glm::abs(os_pos.x - box.min_vert.x) < EPS)
        os_normal = glm::vec3(-1, 0, 0);
    else if (glm::abs(os_pos.x - box.max_vert.x) < EPS)
        os_normal = glm::vec3(1, 0, 0);
    else if (glm::abs(os_pos.y - box.min_vert.y) < EPS)
        os_normal = glm::vec3(0, -1, 0);
    else if (glm::abs(os_pos.y - box.max_vert.y) < EPS)
        os_normal = glm::vec3(0, 1, 0);
    else if (glm::abs(os_pos.z - box.min_vert.z) < EPS)
        os_normal = glm::vec3(0, 0, -1);
    else if (glm::abs(os_pos.z - box.max_vert.z) < EPS)
        os_normal = glm::vec3(0, 0, 1);

    info.os_pos = os_pos;
    info.os_normal = os_normal;
    info.t = os_t;
    return true;
}

bool AxisAlignedBox::hit(const Ray& ws_ray, float t_min, float t_max, HitRecord& rec) const
{
    Ray os_ray = ray_ws_to_os(ws_ray, *this);
    glm::vec3 o = os_ray.o, d = os_ray.d;
    HitInfo info;
    bool ray_originated_in_object = (box.min_vert.x - EPS < o.x && box.max_vert.x + EPS > o.x && box.min_vert.y - EPS < o.y &&
                                     box.max_vert.y + EPS > o.y && box.min_vert.z - EPS < o.z && box.max_vert.z + EPS > o.z);

    if (!aabb_hit_test(os_ray, ray_originated_in_object, box, info)) return false;

    populate_hitrecord(ws_ray, os_ray, info.os_pos, info.os_normal, ray_originated_in_object, *this, rec);
    if (rec.t < t_min || rec.t > t_max) return false;
    return true;
}

Box AxisAlignedBox::bounding_box()
{
    return box;
}

bool triangle_hit_test(const Ray& os_ray, glm::vec3 tri[3], HitInfo& info) {

    glm::vec3 p0 = tri[0], p1 = tri[1], p2 = tri[2];
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
    if (t < 0) return false;
    // if (t < t_min || t > t_max)
    //     return false;

    info.os_pos = o + t * d;
    info.os_normal = glm::normalize(glm::cross(e1, e2));
    info.t = t;
    return true;
}

// Triangle implementation
bool Triangle::hit(const Ray& ws_ray, float t_min, float t_max, HitRecord& rec) const
{
    Ray os_ray = ray_ws_to_os(ws_ray, *this);
    HitInfo info;
    glm::vec3 o = os_ray.o, d = os_ray.d;
    glm::vec3 tri[3] = {p0, p1, p2};

    if(!triangle_hit_test(os_ray, tri, info)) return false;

    populate_hitrecord(ws_ray, os_ray, info.os_pos, info.os_normal, false, *this, rec);
    if (rec.t - EPS < t_min || rec.t + EPS > t_max) return false;
    return true;
}

Box Triangle::bounding_box()
{
    Box box;
    box.min_vert = glm::min(glm::min(p0, p1), p2);
    box.max_vert = glm::max(glm::max(p0, p1), p2);
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
            bbox.min_vert.x = glm::min(bbox.min_vert.x, v.x);
            bbox.min_vert.y = glm::min(bbox.min_vert.y, v.y);
            bbox.min_vert.z = glm::min(bbox.min_vert.z, v.z);
            bbox.max_vert.x = glm::max(bbox.max_vert.x, v.x);
            bbox.max_vert.y = glm::max(bbox.max_vert.y, v.y);
            bbox.max_vert.z = glm::max(bbox.max_vert.z, v.z);
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
    return bbox;
}

bool Mesh::hit(const Ray& ws_ray, float t_min, float t_max, HitRecord& rec) const
{
    Ray os_ray = ray_ws_to_os(ws_ray, *this);
    glm::vec3 o = os_ray.o, d = os_ray.d;
    HitInfo info, min_hit_info;
    HitRecord temp_rec;

    if (!aabb_hit_test(os_ray, false, bbox, info)) return false;

    min_hit_info.t = std::numeric_limits<float>::max();
    bool hit = false;

    for (const glm::ivec3& tri : idxs)
    {

        // glm::vec3 p0 = verts[tri[0]], p1 = verts[tri[1]], p2 = verts[tri[2]];
        glm::vec3 tri_verts[3] = {verts[tri[0]], verts[tri[1]], verts[tri[2]]};
        if (!triangle_hit_test(os_ray, tri_verts, info)) continue;
        glm::vec3 ws_pos = point_os_to_ws(info.os_pos, *this);
        float ws_t = glm::length(ws_pos - ws_ray.o);
        if (ws_t < t_min || ws_t > t_max) continue;
        if (ws_t < min_hit_info.t) {
            hit = true;
            min_hit_info = info;
        }
    }

    if (hit)
    {
        populate_hitrecord(ws_ray, os_ray, info.os_pos, info.os_normal, false, *this, rec);
        return true;
    }
    return false;
}
