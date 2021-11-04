#include "pch.h"
#include "Handler.h"

WindowState::WindowState(Camera& camera, bool& spotlightOn)
	: camera(camera), spotlightOn(spotlightOn)
{}

Handler::Handler(GLFWwindow* window, Camera& camera, bool& spotlightOn)
{
	m_State = new WindowState(camera, spotlightOn);
	glfwSetWindowUserPointer(window, m_State);

	glfwSetKeyCallback(window, HandleKeyboard);
	glfwSetCursorPosCallback(window, HandleCursorPos);
	glfwSetScrollCallback(window, HandleScroll);
}

Handler::~Handler()
{
	delete m_State;
}

WindowState* Handler::GetWindowState(GLFWwindow* window)
{
	return (WindowState*)glfwGetWindowUserPointer(window);
}

void Handler::MaintainKeyboard(GLFWwindow* window)
{

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
		return;
	}

	WindowState* state = GetWindowState(window);

	const float currentFrame = glfwGetTime();
	const float deltaTime = currentFrame - state->lastFrame;
	state->lastFrame = currentFrame;

	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		glm::vec3& lightPos = LightData::pointLights[state->lightIndex].position;
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			lightPos += glm::vec3(0.01f, 0.0f, 0.0f);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			lightPos -= glm::vec3(0.01f, 0.0f, 0.0f);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			lightPos += glm::vec3(0.0f, 0.0f, 0.01f);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			lightPos -= glm::vec3(0.0f, 0.0f, 0.01f);
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			lightPos += glm::vec3(0.0f, 0.01f, 0.0f);
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			lightPos -= glm::vec3(0.0f, 0.01f, 0.0f);

	}
	else {
		Camera& camera = state->camera;
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera.MoveDirection(FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera.MoveDirection(BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera.MoveDirection(LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera.MoveDirection(RIGHT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			camera.MoveDirection(UP, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			camera.MoveDirection(DOWN, deltaTime);
	}
}

void Handler::HandleKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	WindowState* state = GetWindowState(window);

	if (action == GLFW_PRESS)
	{
		float& speed = state->camera.movementSpeed;
		if (key == GLFW_KEY_N)
			speed += 1.0f;

		else if (key == GLFW_KEY_M)
			speed = std::max(speed - 1.0f, 0.0f);

		else if (key == GLFW_KEY_Z)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		else if (key == GLFW_KEY_F)
			state->spotlightOn = !(state->spotlightOn);

		else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		{
			unsigned int& lightIndex = state->lightIndex;
			if (key == GLFW_KEY_UP)
				lightIndex = (lightIndex + 1) % COUNT_POINT_LIGHT;
			else if (key == GLFW_KEY_DOWN)
				lightIndex = (lightIndex - 1) % COUNT_POINT_LIGHT;
		}

		else if (key == GLFW_KEY_LEFT_ALT)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			state->cursorOn = true;
		}
	}

	else if (action == GLFW_RELEASE)
	{
		if (key == GLFW_KEY_Z)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else if (key == GLFW_KEY_LEFT_ALT)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			state->cursorOn = false;
		}
	}
}

void Handler::HandleCursorPos(GLFWwindow* window, double xPos, double yPos)
{
	WindowState* state = GetWindowState(window);


	if (state->firstMouse)
	{
		state->xLast = xPos;
		state->yLast = yPos;
		state->firstMouse = false;
	}

	const float xOffset = (xPos - state->xLast);
	const float yOffset = (yPos - state->yLast);
	state->xLast = xPos;
	state->yLast = yPos;

	if (!state->cursorOn)
		state->camera.PanOffset(xOffset, yOffset, false);
}

void Handler::HandleScroll(GLFWwindow* window, double xOffset, double yOffset)
{
	WindowState* state = GetWindowState(window);
	state->camera.ZoomScroll(yOffset);
}
