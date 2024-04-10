#include "camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace COL781 {

	void Camera::initialize(float aspect) {
		firstMouse = true;
		yaw   = -90.0f;	
		pitch =  0.0f;
		lastX =  800.0f / 2.0;
		lastY =  600.0 / 2.0;
		fov   =  60.0f;

		this->aspect = aspect;

		position = glm::vec3(0.0f, 0.0f,  1.5f);
		lookAt = glm::vec3(0.0f, 0.0f, 0.0f);
		up = glm::vec3(0.0f, 1.0f,  0.0f);

		updateViewMatrix();
	}

	glm::mat4 Camera::getViewMatrix() {
		return viewMatrix;
	}

	void Camera::updateViewMatrix() {
		viewMatrix = glm::lookAt(position, lookAt, up);
	}

	glm::mat4 Camera::getProjectionMatrix() {
		return glm::perspective(glm::radians(fov), aspect, 0.1f, 100.0f);
	}
	glm::vec3 getRightVector();

	glm::vec3 Camera:: getViewDir() {
		return -glm::transpose(viewMatrix)[2];
	}

	glm::vec3 Camera::getRightVector() {
		return glm::transpose(viewMatrix)[0];
	}

	void Camera::setCameraView(glm::vec3 position_vector, glm::vec3 lookat_vector, glm::vec3 up_vector) {
		position = std::move(position_vector);
		lookAt = std::move(lookat_vector);
		up = std::move(up_vector);

		viewMatrix = glm::lookAt(position, lookAt, up);
	}

	void CameraControl::initialize(int width, int height) {
		camera.initialize((float)width/(float)height);

		deltaAngleX = 2.0 * 3.14 / width;
		deltaAngleY = 3.14 / height;

		SDL_GetMouseState(&lastxPos, &lastyPos);
	}

	void CameraControl::update() {
		camera.updateViewMatrix();

		Uint32 buttonState = SDL_GetMouseState(&xPos, &yPos);
		if( buttonState & SDL_BUTTON(SDL_BUTTON_LEFT) ) {
			glm::vec4 pivot = glm::vec4(camera.lookAt.x, camera.lookAt.y, camera.lookAt.z, 1.0f);
			glm::vec4 position = glm::vec4(camera.position.x, camera.position.y, camera.position.z, 1.0f);

			float xAngle = (float)(lastxPos - xPos) * deltaAngleX;
			float yAngle = (float)(lastyPos - yPos) * deltaAngleY;

			float cosAngle = dot(camera.getViewDir(), camera.up);

			if(cosAngle * signbit(deltaAngleY) > 0.99f)
				deltaAngleY = 0.0f;

			glm::mat4 rotationMatX(1.0f);
			rotationMatX = glm::rotate(rotationMatX, xAngle, camera.up);
			position = (rotationMatX * (position - pivot)) + pivot;

			glm::mat4 rotationMatY(1.0f);
			rotationMatY = glm::rotate(rotationMatY, yAngle, camera.getRightVector());
			glm::vec3 finalPosition = (rotationMatY * (position - pivot)) + pivot;
			camera.position = finalPosition;
			camera.updateViewMatrix();
		}

		buttonState = SDL_GetMouseState(&xPos, &yPos);
		if( buttonState & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
			// Update camera parameters

			float deltaY =  (float)(lastyPos - yPos) * 0.01f;
			glm::mat4 dollyTransform = glm::mat4(1.0f);
			dollyTransform = glm::translate(dollyTransform, normalize(camera.lookAt - camera.position) * deltaY);
			glm::vec3 newCameraPosition = dollyTransform * glm::vec4(camera.position, 1.0f);
			float newCameraFov = 2 * glm::atan(600.0f / (2 * deltaY)); // TODO Ask
					
			if(signbit(newCameraPosition.z) == signbit(camera.position.z)) {
				camera.position = newCameraPosition;
				camera.fov = newCameraFov; // TODO Ask
			}
		}

		lastxPos = xPos;
		lastyPos = yPos;
	}

}
