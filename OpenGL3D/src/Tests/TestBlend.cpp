#include "pch.h"

#include <map>

#include "Utility.h"
#include "Camera.h"
#include "Handler.h"
#include "Texture.h"
#include "Shader.h"

#include "StencilData.h"
#include "BlendData.h"

static GLuint createVertexArray()
{
	GLuint vao;
	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glBindVertexArray(0);
	return vao;
}

static void drawWindows(GLuint shader, GLuint vao, const Camera& camera)
{

	std::map<float, glm::vec3> sorted;

	for (unsigned int i = 0; i < BlendData::windowPos.size(); i++)
	{
		float distance = glm::length(camera.pos - BlendData::windowPos[i]);
		sorted[distance] = BlendData::windowPos[i];
	}

	glUseProgram(shader);
	glUniform1i(glGetUniformLocation(shader, "aTex"), 0);
	glBindVertexArray(vao);

	for (std::map<float, glm::vec3>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it)
	{
		glm::mat4 model(1.0f);
		model = glm::translate(model, it->second);
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.1f));
		glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	}
}

static void drawCubes(GLuint shader, GLuint vao)
{
	glUseProgram(shader);
	glUniform1i(glGetUniformLocation(shader, "aTex"), 2);

	glBindVertexArray(vao);
	for (const glm::vec3& pos : StencilData::cubePositions)
	{
		glm::mat4 model(1.0f);
		model = glm::translate(model, pos);
		model = glm::translate(model, glm::vec3(2.5f, 0, -7.5f));
		glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	}

}
static void drawPlane(GLuint shader, GLuint vao)
{
	glUseProgram(shader);
	glm::mat4 model(1.0f);
	model = glm::translate(model, glm::vec3(2.5f, 0, -2.5f));
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniform1i(glGetUniformLocation(shader, "aTex"), 1);

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}


void TestBlend()

{
	GLFWwindow* window = Utility::setupGLFW();
	Utility::setupGLEW();

	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Initialise shaders and load programs.
	GLuint coreProgram;
	if (!Shader::loadProgram(coreProgram, "res/shaders/stencil/VertexCore.glsl", "res/shaders/stencil/FragmentCore.glsl"))
		std::cerr << "[Error: main.cpp] Failed to load core program." << std::endl;

	// cube
	GLuint cubeVBO;
	glGenBuffers(1, &cubeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(StencilData::cubeVertices), StencilData::cubeVertices, GL_STATIC_DRAW);

	GLuint cubeVAO = createVertexArray();
	glBindVertexArray(cubeVAO);

	GLuint cubeEBO;
	glGenBuffers(1, &cubeEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(StencilData::cubeIndices), StencilData::cubeIndices, GL_STATIC_DRAW);

	glBindVertexArray(0);

	// plane 
	GLuint planeVBO;
	glGenBuffers(1, &planeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(StencilData::planeVertices), StencilData::planeVertices, GL_STATIC_DRAW);

	GLuint planeVAO = createVertexArray();
	glBindVertexArray(planeVAO);

	GLuint planeEBO;
	glGenBuffers(1, &planeEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(StencilData::planeIndices), StencilData::planeIndices, GL_STATIC_DRAW);

	glBindVertexArray(0);

	// Initialise textures and materials.
	stbi_set_flip_vertically_on_load(true);

	Texture windowTexture(TextureType::DIFFUSE, 0, "res/textures/rose_window.png", true);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	Texture floorTexture(TextureType::DIFFUSE, 1, "res/textures/tile.png", true);
	Texture cubeTexture(TextureType::DIFFUSE, 2, "res/textures/cobble.png", true);

	// Initialise window state.
	Camera camera(
		glm::vec3(0.0f, 0.0f, 6.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		-90.0f,
		0.0f
	);

	bool spotlightOn = true;

	Handler handler(window, camera, spotlightOn);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		Handler::MaintainKeyboard(window);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		

		glm::mat4 view = std::move(camera.GetViewMatrix());
		glm::mat4 projection = glm::perspective(glm::radians(camera.fov), 800.0f / 600.0f, 0.1f, 100.0f);

		glUseProgram(coreProgram);
		glUniformMatrix4fv(glGetUniformLocation(coreProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(coreProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glEnable(GL_CULL_FACE);
		drawCubes(coreProgram, cubeVAO);
		drawPlane(coreProgram, planeVAO);

		glDisable(GL_CULL_FACE);
		drawWindows(coreProgram, planeVAO, camera);
		

		glfwSwapBuffers(window);
		glFlush();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	glDeleteProgram(coreProgram);
}
