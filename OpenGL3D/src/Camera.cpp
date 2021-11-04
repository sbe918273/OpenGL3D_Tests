#include "pch.h"
#include "Camera.h"

Camera::Camera(glm::vec3 pos, glm::vec3 worldUp, float yaw, float pitch)
	: pos(pos), worldUp(worldUp), yaw(yaw), pitch(pitch)
{
	UpdateVectors();
}

glm::mat4 Camera::GetViewMatrix()
{
	//return glm::lookAt(pos, pos + front, up);

	return {
		 right.x,               up.x,             -front.x,              0.0f,
		 right.y,               up.y,             -front.y,              0.0f,
		 right.z,               up.z,             -front.z,              0.0f,
		-glm::dot(pos, right), -glm::dot(pos, up), glm::dot(pos, front), 1.0f
	};
}

void Camera::MoveDirection(CameraMovement direction, float deltaTime)
{
	switch (direction)
	{
	case FORWARD:
		pos += front * movementSpeed * deltaTime;
		break;

	case BACKWARD:
		pos -= front * movementSpeed * deltaTime;
		break;

	case LEFT:
		pos -= right * movementSpeed * deltaTime;
		break;
	
	case RIGHT:
		pos += right * movementSpeed * deltaTime;
		break;
	
	case UP:
		pos += up * movementSpeed * deltaTime;
		break;

	case DOWN:
		pos -= up * movementSpeed * deltaTime;
		break;
	}
}

void Camera::PanOffset(float xOffset, float yOffset, bool constrainPitch)
{
	yaw += mouseSensitivity * xOffset;
	pitch -= mouseSensitivity * yOffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	else if (pitch < -89.0f)
		pitch = -89.0f;

	UpdateVectors();
}

void Camera::UpdateVectors()
{
	front = glm::vec3(
		cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
		sin(glm::radians(pitch)),
		sin(glm::radians(yaw)) * cos(glm::radians(pitch))
	);
	front = glm::normalize(front);
	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));
}

void Camera::ZoomScroll(float yOffset)
{
	fov -= yOffset;
	if (fov < 1.0f)
		fov = 1.0f;
	if (fov > 45.0f)
		fov = 45.0f;
}
