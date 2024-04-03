#pragma once

#include <glm/glm.hpp>
#include <string>
#include <sstream>

struct Ray
{
    glm::vec3 o, d;
};

struct Box
{
    glm::vec3 tl, br;
};

struct HitRecord
{
    Ray ray;
    glm::vec3 pos;
    glm::vec3 normal;
    int n_bounces_left;
    float t;
};
