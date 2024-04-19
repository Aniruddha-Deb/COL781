#pragma once

#include <glm/glm.hpp>
#include <vector>

class Plane
{
  public:
    glm::vec3 center;
    glm::vec3 normal;
    float time;
    float eps;
    float mu;
    Plane(glm::vec3 _center, glm::vec3 _normal, float _eps, float _mu, float _time);
    std::vector<glm::vec3> vert_pos;
    std::vector<glm::vec3> vert_normals;
    std::vector<glm::ivec3> faces;
    void update(float t);
};