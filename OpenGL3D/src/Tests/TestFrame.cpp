#include "pch.h"
#include "Utility.h"

#include "Camera.h"
#include "Handler.h"
#include "Texture.h"
#include "Shader.h"

#include "StencilData.h"

static void createFrameBuffer(GLuint& framebuffer, GLuint& texColorBuffer)
{
	// frame buffer
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	// create texture and attach to frame buffer as color attachment
	glGenTextures(1, &texColorBuffer);
	glBindTexture(GL_TEXTURE_2D, texColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);

	// create renderbuffer and attach to frame buffer as depth and stencil attachement
	GLuint rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "[Error: TestFrame.cpp] Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

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

static void drawCubes(GLuint shader, GLuint vao)
{
	glUseProgram(shader);
	glUniform1i(glGetUniformLocation(shader, "aTex"), 0);

	glBindVertexArray(vao);
	for (const glm::vec3& pos : StencilData::cubePositions)
	{
		glm::mat4 model(1.0f);
		model = glm::translate(model, pos);
		model = glm::translate(model, glm::vec3(2.5f, 0.0f, -5.0f));
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

static void drawQuad(GLuint shader, GLuint vao, int mode, float scale)
{
	glUseProgram(shader);
	glUniform1i(glGetUniformLocation(shader, "aTex"), 2);
	glUniform1i(glGetUniformLocation(shader, "mode"), mode);

	glm::mat4 model(1.0f);

	model = glm::translate(model, glm::vec3(1.0f - scale));
	model = glm::scale(model, glm::vec3(scale));

	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void TestFrame()
{
	GLFWwindow* window = Utility::setupGLFW();
	Utility::setupGLEW();

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	GLuint framebuffer, colorbuffer;
	createFrameBuffer(framebuffer, colorbuffer);

	int modes[] = { 4, 3};
	float scales[] = { 1.0f, 0.4f };

	GLuint coreProgram;
	Shader::loadProgram(coreProgram, "res/shaders/stencil/VertexCore.glsl", "res/shaders/stencil/FragmentCore.glsl");

	GLuint quadProgram;
	Shader::loadProgram(quadProgram, "res/shaders/frame/VertexQuad.glsl", "res/shaders/frame/FragmentQuad.glsl");

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

	// quad
	float quadVertices[] =
	{
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
	};

	unsigned int quadIndices[] =
	{
		0, 1, 2,
		2, 3, 0
	};

	GLuint quadVBO;
	glGenBuffers(1, &quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	GLuint quadVAO = createVertexArray();
	glBindVertexArray(quadVAO);

	GLuint quadEBO;
	glGenBuffers(1, &quadEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

	glBindVertexArray(0);

	// Initialise textures and materials.
	stbi_set_flip_vertically_on_load(true);

	Texture cubeTexture(TextureType::DIFFUSE, 0, "res/textures/container.jpg", false);
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

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glm::mat4 projection = glm::perspective(glm::radians(camera.fov), 800.0f / 600.0f, 0.1f, 100.0f);

		for (unsigned int i = 0; i < 2; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);

			glm::mat4 view = std::move(camera.GetViewMatrix());
			
			glUseProgram(coreProgram);
			glUniformMatrix4fv(glGetUniformLocation(coreProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(coreProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

			drawCubes(coreProgram, cubeVAO);
			drawPlane(coreProgram, planeVAO);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDisable(GL_DEPTH_TEST);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, colorbuffer);
			drawQuad(quadProgram, quadVAO, modes[i], scales[i]);

			camera.front *= -1.0f;
		}

		glfwSwapBuffers(window);
		glFlush();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	glDeleteProgram(coreProgram);
}