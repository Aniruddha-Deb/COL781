#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>
#include "plane.hpp"

Plane::Plane(glm::vec3 _center, glm::vec3 _normal, float _eps, float _mu, float _time)
    : center{_center}, normal{_normal}, eps{_eps}, mu{_mu}, time{_time}
{
    glm::vec3 tangent(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 3; i++)
    {
        if (normal[i] != 0)
        {
            tangent[(i + 1) % 3] = normal[i];
            tangent[i] = -normal[(i + 1) % 3];
            break;
        }
    }
    tangent = glm::normalize(tangent);
    vert_pos.push_back(center + 500.0f * tangent);
    vert_pos.push_back(center + 500.0f * glm::cross(tangent, normal));
    vert_pos.push_back(center - 500.0f * tangent);
    vert_pos.push_back(center - 500.0f * glm::cross(tangent, normal));
    vert_normals.push_back(normal);
    vert_normals.push_back(normal);
    vert_normals.push_back(normal);
    vert_normals.push_back(normal);
    faces.push_back(glm::ivec3(0, 1, 2));
    faces.push_back(glm::ivec3(2, 3, 0));
}