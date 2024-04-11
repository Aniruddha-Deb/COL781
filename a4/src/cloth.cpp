#include <glm/glm.hpp>
#include <iostream>

#include "cloth.hpp"

constexpr float GRAVITY = 0.1;

Cloth::Cloth(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4, int _res_w, int _res_h, float _k_struct,
             float _k_shear, float _k_bend, float _mass, float _time)
    : res_w{_res_w}, res_h{_res_h}, k_struct{_k_struct}, k_shear{_k_shear}, k_bend{_k_bend}, mass{_mass}, time{_time}
{
    assert(glm::distance(p1, p2) == glm::distance(p3, p4));
    assert(glm::distance(p2, p3) == glm::distance(p4, p1));
    w = glm::distance(p1, p2);
    h = glm::distance(p2, p3);
    glm::vec3 w_vert = p2 - p1;
    glm::vec3 h_vert = p3 - p2;
    vert_pos.resize(res_w * res_h);
    for (int i = 0; i < res_h; i++)
    {
        for (int j = 0; j < res_w; j++)
        {
            vert_pos[i * res_w + j] =
                p1 + (float)i * h_vert / (float)(res_h - 1) + (float)j * w_vert / (float)(res_w - 1);
        }
    }
    vert_velocity.resize(res_w * res_h, glm::vec3(0.0f, 0.0f, 0.0f));
    vert_normals.resize(res_w * res_h, glm::vec3(0.0f, 0.0f, 1.0f));
    faces.resize((res_w - 1) * (res_h - 1));
    for (int i = 0; i < res_h - 1; i++)
    {
        for (int j = 0; j < res_w - 1; j++)
        {
            faces.push_back(glm::ivec3(i * res_w + j, i * res_w + j + 1, (i + 1) * res_w + j + 1));
            faces.push_back(glm::ivec3(i * res_w + j, (i + 1) * res_w + j + 1, (i + 1) * res_w + j));
        }
    }
}

void Cloth::fix_vertex(int row, int col)
{
    fixed.insert(row * res_w + col);
}

glm::vec3 Cloth::structural_force(int row, int col)
{
    float spacing_w = w / (res_w - 1);
    float spacing_h = h / (res_h - 1);
    // std::cout << spacing_w << " " << glm::distance(vert_pos[0], vert_pos[1]) << "\n";
    int idx = row * res_w + col;
    glm::vec3 force(0.0f, 0.0f, 0.0f);
    if (row > 0)
    {
        force += k_struct * (glm::distance(vert_pos[idx], vert_pos[idx - res_w]) - spacing_h) *
                 glm::normalize(vert_pos[idx - res_w] - vert_pos[idx]);
    }
    if (row < res_h - 1)
    {
        force += k_struct * (glm::distance(vert_pos[idx], vert_pos[idx + res_w]) - spacing_h) *
                 glm::normalize(vert_pos[idx + res_w] - vert_pos[idx]);
    }
    if (col > 0)
    {
        force += k_struct * (glm::distance(vert_pos[idx], vert_pos[idx - 1]) - spacing_w) *
                 glm::normalize(vert_pos[idx - 1] - vert_pos[idx]);
    }
    if (col < res_w - 1)
    {
        force += k_struct * (glm::distance(vert_pos[idx], vert_pos[idx + 1]) - spacing_w) *
                 glm::normalize(vert_pos[idx + 1] - vert_pos[idx]);
    }
    return force;
}

glm::vec3 Cloth::shear_force(int row, int col)
{
    float spacing = sqrt((w / (res_w - 1)) * (w / (res_w - 1)) + (h / (res_h - 1)) * (h / (res_h - 1)));
    int idx = row * res_w + col;
    glm::vec3 force(0.0f, 0.0f, 0.0f);
    if (row > 0 && col > 0)
    {
        force += k_shear * (glm::distance(vert_pos[idx], vert_pos[idx - res_w - 1]) - spacing) *
                 glm::normalize(vert_pos[idx - res_w - 1] - vert_pos[idx]);
    }
    if (row > 0 && col < res_w - 1)
    {
        force += k_shear * (glm::distance(vert_pos[idx], vert_pos[idx - res_w + 1]) - spacing) *
                 glm::normalize(vert_pos[idx - res_w + 1] - vert_pos[idx]);
    }
    if (row < res_h - 1 && col > 0)
    {
        force += k_shear * (glm::distance(vert_pos[idx], vert_pos[idx + res_w - 1]) - spacing) *
                 glm::normalize(vert_pos[idx + res_w - 1] - vert_pos[idx]);
    }
    if (row < res_h - 1 && col < res_w - 1)
    {
        force += k_shear * (glm::distance(vert_pos[idx], vert_pos[idx + res_w + 1]) - spacing) *
                 glm::normalize(vert_pos[idx + res_w + 1] - vert_pos[idx]);
    }
    return force;
}

glm::vec3 Cloth::bending_force(int row, int col)
{
    float spacing_w = 2 * w / (res_w - 1);
    float spacing_h = 2 * h / (res_h - 1);
    int idx = row * res_w + col;
    glm::vec3 force(0.0f, 0.0f, 0.0f);
    if (row > 1)
    {
        force += k_bend * (glm::distance(vert_pos[idx], vert_pos[idx - 2 * res_w]) - spacing_h) *
                 glm::normalize(vert_pos[idx - 2 * res_w] - vert_pos[idx]);
    }
    if (row < res_h - 2)
    {
        force += k_bend * (glm::distance(vert_pos[idx], vert_pos[idx + 2 * res_w]) - spacing_h) *
                 glm::normalize(vert_pos[idx + 2 * res_w] - vert_pos[idx]);
    }
    if (col > 1)
    {
        force += k_bend * (glm::distance(vert_pos[idx], vert_pos[idx - 2]) - spacing_w) *
                 glm::normalize(vert_pos[idx - 2] - vert_pos[idx]);
    }
    if (col < res_w - 2)
    {
        force += k_bend * (glm::distance(vert_pos[idx], vert_pos[idx + 2]) - spacing_w) *
                 glm::normalize(vert_pos[idx + 2] - vert_pos[idx]);
    }
    return force;
}

void Cloth::update(float t)
{
    // TODO
    for (int i = 0; i < res_h; i++)
    {
        for (int j = 0; j < res_w; j++)
        {
            if (fixed.find(i * res_w + j) != fixed.end())
            {
                continue;
            }
            glm::vec3 force = structural_force(i, j) + shear_force(i, j) + bending_force(i, j) +
                              GRAVITY * mass * glm::vec3(0.0f, -1.0f, 0.0f);
            vert_velocity[i * res_w + j] += (t - time) * force / mass;
        }
    }
    for (int i = 0; i < res_h; i++)
    {
        for (int j = 0; j < res_w; j++)
        {
            if (fixed.find(i * res_w + j) != fixed.end())
            {
                continue;
            }
            vert_pos[i * res_w + j] += (t - time) * vert_velocity[i * res_w + j];
        }
    }
    time = t;
}