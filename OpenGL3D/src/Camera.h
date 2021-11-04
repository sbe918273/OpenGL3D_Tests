#pragma once
#include <glm.hpp>
#include <vec2.hpp>
#include <vec3.hpp>
#include <vec4.hpp>
#include <mat4x4.hpp>
#include <gtc\matrix_transform.hpp>
#include <gtc\type_ptr.hpp>

enum CameraMovement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

class Camera
{
public:
	glm::vec3 pos;
	glm::vec3 front;
	glm::vec3 right;
	glm::vec3 up;
	glm::vec3 worldUp;

	float yaw;
	float pitch;

	float movementSpeed= 2.5f;
	const float mouseSensitivity = 0.1f;
	float fov = 45.0f;

	Camera(glm::vec3 pos, glm::vec3 worldUp, float yaw, float pitch);

	glm::mat4 GetViewMatrix();
	
	void MoveDirection(CameraMovement direction, float deltaTime);
	void PanOffset(float xOffset, float yOffset, bool constrainPitch);
	void ZoomScroll(float yOffset);

	void UpdateVectors();
	
};

