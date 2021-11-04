#include "pch.h"
#include "Utility.h"

static void resizeFrameBuffer(GLFWwindow* window, int frameBufferWidth, int frameBufferHeight)
{
	glViewport(0, 0, frameBufferWidth, frameBufferHeight);
}

static void GLAPIENTRY debugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	std::cout << "[OpenGL error: see call stack](" << type << ") " << message << std::endl;
}

void Utility::setupGLEW()
{
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
	{
		glfwTerminate();
		std::cerr << "[Error: main.cpp] GLEW failed to initialise." << std::endl;
	}

	glDebugMessageCallback(debugMessage, nullptr);
}

GLFWwindow* Utility::setupGLFW()
{
	return setupGLFW(1);
}

GLFWwindow* Utility::setupGLFW(unsigned int samples)
{
	// Initialise GLFW.
	glfwInit();

	const int WINDOW_WIDTH = 800;
	const int WINDOW_HEIGHT = 600;

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	if (samples >= 2)
		glfwWindowHint(GLFW_SAMPLES, samples);

	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Watch this space!", nullptr, nullptr);

	glfwSetFramebufferSizeCallback(window, resizeFrameBuffer);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwMakeContextCurrent(window);

	return window;
}

void Utility::setupImgui(GLFWwindow* window)
{
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 440");
}
