#include "pch.h"

#include "Utility.h"
#include "Camera.h"
#include "Handler.h"
#include "Texture.h"
#include "Shader.h"

#include "StencilData.h"

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

static void drawCubes(GLuint shader, GLuint vao, bool border)
{
	glUseProgram(shader);
	glUniform1i(glGetUniformLocation(shader, "aTex"), 0);

	glBindVertexArray(vao);
	for (const glm::vec3& pos : StencilData::cubePositions)
	{
		glm::mat4 model(1.0f);
		model = glm::translate(model, pos);
		if (border)
			model = glm::scale(model, glm::vec3(1.1f, 1.1f, 1.1f));
		glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	}
}

static void drawPlane(GLuint shader, GLuint vao)
{
	glUseProgram(shader);
	glm::mat4 model(1.0f);
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniform1i(glGetUniformLocation(shader, "aTex"), 1);

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void TestStencil()

{
	GLFWwindow* window = Utility::setupGLFW();
	Utility::setupGLEW();

	glEnable(GL_STENCIL_TEST);
	glFrontFace(GL_CCW);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Initialise shaders and load programs.
	GLuint coreProgram;
	if (!Shader::loadProgram(coreProgram, "res/shaders/stencil/VertexCore.glsl", "res/shaders/stencil/FragmentCore.glsl"))
		std::cerr << "[Error: main.cpp] Failed to load core program." << std::endl;

	GLuint solidProgram;
	if (!Shader::loadProgram(solidProgram, "res/shaders/stencil/VertexCore.glsl", "res/shaders/stencil/FragmentSolid.glsl"))
		std::cerr << "[Error: main.cpp] Failed to load border program." << std::endl;

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

	Texture cubeTexture(TextureType::DIFFUSE, 0, "res/textures/cobble.png", true);
	Texture floorTexture(TextureType::DIFFUSE, 1, "res/textures/tile.png", true);

	// Initialise window state.
	Camera camera(
		glm::vec3(0.0f, 0.0f, 3.0f),
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
		glStencilMask(0xFF);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);

		glm::mat4 view = std::move(camera.GetViewMatrix());
		glm::mat4 projection = glm::perspective(glm::radians(camera.fov), 800.0f / 600.0f, 0.1f, 100.0f);

		glUseProgram(coreProgram);
		glUniformMatrix4fv(glGetUniformLocation(coreProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(coreProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		drawCubes(coreProgram, cubeVAO, false);
		
		glStencilMask(0x00);
		drawPlane(coreProgram, planeVAO);

		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glDisable(GL_DEPTH_TEST);

		glUseProgram(solidProgram);
		glUniformMatrix4fv(glGetUniformLocation(solidProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(solidProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		drawCubes(solidProgram, cubeVAO, true);

		glfwSwapBuffers(window);
		glFlush();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	glDeleteProgram(coreProgram);
}