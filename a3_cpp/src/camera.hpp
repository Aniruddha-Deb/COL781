#pragma once

#include <glm/glm.hpp>

class Camera
{
  public:
    glm::vec3 position;
    glm::vec3 up;
    glm::vec3 lookAt;
    glm::mat4 viewMatrix;

    float fov;
    Camera(float fov_degrees, glm::vec3 _position, glm::vec3 _lookat, glm::vec3 _up);
    glm::mat4 getViewMatrix();
    glm::vec3 getViewDir();
    glm::vec3 getRightVector();
    void setCameraView(glm::vec3 position_vector, glm::vec3 lookat_vector, glm::vec3 up_vector);
    void updateViewMatrix();
};
