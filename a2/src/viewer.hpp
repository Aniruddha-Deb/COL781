#ifndef VIEWER_HPP
#define VIEWER_HPP

#include "hw.hpp"

namespace COL781 {
    namespace Viewer {

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

        class Viewer {
        public:
            bool initialize(const std::string &title, int width, int height);
            void setVertices(int n, const glm::vec3* vertices);
            void setNormals(int n, const glm::vec3* normals);
            void setTriangles(int n, const glm::ivec3* triangles);
            void view();
        private:
            COL781::OpenGL::Rasterizer r;
            COL781::OpenGL::ShaderProgram program;
            COL781::OpenGL::Object object;
            Camera camera;
        };

    }
}

#endif
