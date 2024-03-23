#pragma once

#include <glm/glm.hpp>

class Camera {
public:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up; 
    glm::vec3 lookAt;
    glm::mat4 viewMatrix;

    float cameraSpeed, yaw, pitch, lastX, lastY, fov, aspect;
    bool firstMouse;
    void initialize(float aspect);
    glm::mat4 getViewMatrix();
    glm::mat4 getProjectionMatrix();
    glm::vec3 getViewDir();
    glm::vec3 getRightVector();

    void setCameraView(glm::vec3 position_vector, glm::vec3 lookat_vector, glm::vec3 up_vector);
    void updateViewMatrix();
};
