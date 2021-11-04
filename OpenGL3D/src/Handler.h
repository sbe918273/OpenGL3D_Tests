#pragma once
#include <algorithm>
#include "Camera.h"
#include "LightData.h"

struct WindowState
{
	WindowState(Camera& camera, bool& spotlightOn);
	Camera& camera;

	bool firstMouse = true;
	float xLast = 0.0f;
	float yLast = 0.0f;

	float lastFrame = 0.0f;
	unsigned int lightIndex = 0;

	bool& spotlightOn;
	bool cursorOn = false;
};

class Handler
{
public:
	Handler(GLFWwindow* window, Camera& camera, bool& spotlightOn);
	~Handler();

	static void MaintainKeyboard(GLFWwindow* window);
	static void HandleKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void HandleCursorPos(GLFWwindow* window, double xPos, double yPos);
	static void HandleScroll(GLFWwindow* window, double xOffset, double yOffset);

private:
	WindowState* m_State;
	static WindowState* GetWindowState(GLFWwindow* window);
};

