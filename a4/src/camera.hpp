#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "hw.hpp"

namespace COL781 {

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

	class CameraControl {
	public:
		Camera camera;
		void initialize(int width, int height);
		void update();
	private:
		int lastxPos, lastyPos, xPos, yPos;
		float deltaAngleX, deltaAngleY;
	};

	class Mesh {
	public:
		bool initialize(int nv, int nt);
	};

}

#endif
