#include "camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <utility>

Camera::Camera(float aspect)
{
    firstMouse = true;
    yaw = -90.0f;
    pitch = 0.0f;
    lastX = 800.0f / 2.0;
    lastY = 600.0 / 2.0;
    fov = 60.0f;

    this->aspect = aspect;

    position = glm::vec3(0.0f, 0.0f, 0.0f);
    lookAt = glm::vec3(0.0f, 0.0f, -1.0f);
    up = glm::vec3(0.0f, -1.0f, 0.0f);

    updateViewMatrix();
}

glm::mat4 Camera::getViewMatrix()
{
    return viewMatrix;
}

void Camera::updateViewMatrix()
{
    viewMatrix = glm::lookAt(position, lookAt, up);
}

glm::vec3 getRightVector();

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

    viewMatrix = glm::lookAt(position, lookAt, up);
}
