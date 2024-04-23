#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>
#include "sphere.hpp"

constexpr float THICKNESS = 0.03f;

Sphere::Sphere(glm::vec3 _center, float _radius, glm::vec3 _velocity, glm::vec3 _ang_velocity, float _eps, float _mu,
               float _time)
    : center{_center}, radius{_radius}, velocity{_velocity}, ang_velocity{_ang_velocity}, eps{_eps}, mu{_mu},
      time{_time}
{
    int m = 20, n = 20; // m is slices(longi) n is stacks(lati)

    float latitude_dist = M_PI / n;
    float longitude_dist = 2 * M_PI / m;

    vert_pos.push_back(center + (radius - THICKNESS) * glm::vec3(0.0, 1.0, 0.0));
    vert_normals.push_back((vert_pos.back() - center) / (radius - THICKNESS));
    for (int i = 1; i < n; i++)
    {
        float curr_latitude = i * latitude_dist;
        for (int j = 0; j < m; j++)
        {
            float curr_longitude = j * longitude_dist;
            vert_pos.push_back(center + (radius - THICKNESS) * glm::vec3(cosf(curr_longitude) * sinf(curr_latitude),
                                                                         cosf(curr_latitude),
                                                                         sinf(curr_longitude) * sinf(curr_latitude)));
            vert_normals.push_back((vert_pos.back() - center) / (radius - THICKNESS));
        }
    }
    vert_pos.push_back(center + (radius - THICKNESS) * glm::vec3(0, -1.0, 0.0));
    vert_normals.push_back((vert_pos.back() - center) / (radius - THICKNESS));

    for (int j = 0; j < m; j++)
    {
        // north pole and first layer
        faces.push_back(glm::ivec3(0, (j + 1) % m + 1, j + 1));
    }
    for (int i = 1; i < n - 1; i++)
    {
        for (int j = 0; j < m; j++)
        {
            // middle layers
            faces.push_back(glm::ivec3((i - 1) * m + j + 1, (i - 1) * m + (j + 1) % m + 1, i * m + j + 1));
            faces.push_back(glm::ivec3((i - 1) * m + (j + 1) % m + 1, i * m + (j + 1) % m + 1, i * m + j + 1));
        }
    }
    for (int j = 0; j < m; j++)
    {
        // south pole and last layer
        faces.push_back(glm::ivec3((n - 2) * m + j + 1, (n - 2) * m + (j + 1) % m + 1, n * m - m + 1));
    }
}

void Sphere::update(float t)
{
    for (int i = 0; i < vert_pos.size(); i++)
    {
        if (glm::length(ang_velocity) >= 1e-3)
            vert_pos[i] =
                glm::translate(center) *
                glm::rotate(glm::mat4(1.0f), (t - time) * glm::length(ang_velocity), glm::normalize(ang_velocity)) *
                glm::translate(-center) * glm::vec4(vert_pos[i], 1.0f);
        vert_pos[i] += velocity * (t - time);
    }
    center += velocity * (t - time);
    for (int i = 0; i < vert_normals.size(); i++)
    {
        if (glm::length(ang_velocity) >= 1e-3)
            vert_normals[i] =
                glm::rotate(glm::mat4(1.0f), (t - time) * glm::length(ang_velocity), glm::normalize(ang_velocity)) *
                glm::vec4(vert_normals[i], 0.0f);
    }
    time = t;
}