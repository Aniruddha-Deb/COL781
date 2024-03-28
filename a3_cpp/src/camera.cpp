#include "camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <utility>

Camera::Camera(float fov_degrees, glm::vec3 _position, glm::vec3 _lookat, glm::vec3 _up)
{
    fov = (fov_degrees * M_PI) / 180.0f;

    setCameraView(_position, _lookat, _up);
}

glm::mat4 Camera::getViewMatrix()
{
    return viewMatrix;
}

void Camera::updateViewMatrix()
{
    viewMatrix = glm::lookAt(position, lookAt, up);
}

glm::vec3 Camera::getViewDir()
{
    return -glm::transpose(viewMatrix)[2];
}

glm::vec3 Camera::getRightVector()
{
    return glm::transpose(viewMatrix)[0];
}

void Camera::setCameraView(glm::vec3 position_vector, glm::vec3 lookat_vector, glm::vec3 up_vector)
{
    position = std::move(position_vector);
    lookAt = std::move(lookat_vector);
    up = std::move(up_vector);

    updateViewMatrix();
}
