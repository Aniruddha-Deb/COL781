#pragma once

#include <glm/glm.hpp>

struct Ray
{
    glm::vec3 o, d, color;
};

struct Box
{
    glm::vec3 tl, br;
};

struct HitRecord
{
    glm::vec3 pos;
    glm::vec3 normal;
    int n_bounces_left;
    float t;
};
