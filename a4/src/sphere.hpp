#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <set>

class Sphere
{
  public:
    glm::vec3 center;
    float radius;
    float time;
    float eps;
    float mu;
    glm::vec3 velocity;
    glm::vec3 ang_velocity;
    Sphere(glm::vec3 _center, float _radius, glm::vec3 _velocity, glm::vec3 _ang_velocity, float _eps, float _mu,
           float _time);
    std::vector<glm::vec3> vert_pos;
    std::vector<glm::vec3> vert_normals;
    std::vector<glm::ivec3> faces;
    void update(float t);
};