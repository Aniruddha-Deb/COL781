#include <glm/glm.hpp>
#include <iostream>

#include "cloth.hpp"

constexpr float GRAVITY = -10;

Cloth::Cloth(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4, int _res_w, int _res_h, float _k_struct,
             float _k_shear, float _k_bend, float _mass)
    : res_w{_res_w}, res_h{_res_h}, k_struct{_k_struct}, k_shear{_k_shear}, k_bend{_k_bend}, mass{_mass}
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
}

void Cloth::update(float t)
{
    // TODO
}