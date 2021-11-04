#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace Utility
{
	GLFWwindow* setupGLFW();
	GLFWwindow* setupGLFW(unsigned int samples);
	void setupGLEW();
	void setupImgui(GLFWwindow* window);
}