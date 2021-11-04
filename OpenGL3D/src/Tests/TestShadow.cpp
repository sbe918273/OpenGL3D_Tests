#include "pch.h"

#include "Utility.h"
#include "Camera.h"
#include "Handler.h"
#include "Texture.h"
#include "Shader.h"

#include "CubeData.h"

static const unsigned int SHADOW_WIDTH = 4096;
static const unsigned int SHADOW_HEIGHT = 4096;

static void drawPlane(GLuint shader, GLuint vao)
{
	glm::mat4 model(1.0f);
	//model = glm::scale(model, glm::vec3(3.5f, 1.0f, 3.5f));

	glUseProgram(shader);
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniform1i(glGetUniformLocation(shader, "material.diffuse"), 0);
	glUniform1i(glGetUniformLocation(shader, "material.specular"), 1);
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

static void drawCube(GLuint shader, GLuint vao)
{
	glm::mat4 model(1.0f);

	glUseProgram(shader);
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniform1i(glGetUniformLocation(shader, "material.diffuse"), 2);
	glUniform1i(glGetUniformLocation(shader, "material.specular"), 3);
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

static void createDepthMap(GLuint& outFrameBuffer, GLuint& outDepthMap, unsigned int textureUnit)
{
	glGenTextures(1, &outDepthMap);
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_2D, outDepthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glGenFramebuffers(1, &outFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, outDepthMap);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, outDepthMap, 0);
	glDrawBuffer(GL_NONE);
	glDrawBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "[Error: TestShadow.cpp] Depth framebuffer is not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static GLuint createVertexArray()
{
	GLuint vao;
	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	return vao;
}

static GLuint createCubeVertexArray()
{
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CubeData::vertices), CubeData::vertices, GL_STATIC_DRAW);

	GLuint vao = createVertexArray();

	GLuint ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(CubeData::indices), CubeData::indices, GL_STATIC_DRAW);

	return vao;
}

static GLuint createPlaneVertexArray()
{
	static const float vertices[] = {
		// positions        normals		       texCoords
		 5.0f, -0.5f,  5.0f, 0.0f, 1.0f, 0.0f, 2.0f, 0.0f,
		-5.0f, -0.5f,  5.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		-5.0f, -0.5f, -5.0f, 0.0f, 1.0f, 0.0f, 0.0f, 2.0f,
		 5.0f, -0.5f, -5.0f, 0.0f, 1.0f, 0.0f, 2.0f, 2.0f
	};

	static const unsigned int indices[] =
	{
		2, 1, 0,
		3, 2, 0
	};

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	GLuint vao = createVertexArray();

	GLuint ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	return vao;
}

static GLuint createQuadVertexArray()
{
	static const float quadVertices[] =
	{
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
	};

	static const unsigned int quadIndices[] =
	{
		0, 1, 2,
		2, 3, 0
	};

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	GLuint vao;
	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	GLuint ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

	glBindVertexArray(0);

	return vao;
}


void TestShadow()
{
	GLFWwindow* window = Utility::setupGLFW();
	Utility::setupGLEW();

	glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// create depth map
	GLuint depthMapFBO, depthMap;
	createDepthMap(depthMapFBO, depthMap, 4);

	// load shaders and VAOs
	GLuint coreProgram;
	Shader::loadProgram(coreProgram, "res/shaders/Shadow/VertexCore.glsl", "res/shaders/Shadow/FragmentCore.glsl");

	GLuint depthProgram;
	Shader::loadProgram(depthProgram, "res/shaders/Shadow/VertexDepth.glsl", "res/shaders/Shadow/FragmentDepth.glsl");

	GLuint planeVAO = createPlaneVertexArray();
	GLuint cubeVAO = createCubeVertexArray();

	// initialise window state
	Camera camera(
		glm::vec3(0.0f, 0.0f, 3.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		-90.0f,
		0.0f
	);

	bool spotlightOn = false;

	Handler handler(window, camera, spotlightOn);

	Texture planeDiff(TextureType::DIFFUSE, 0, "res/textures/wood.png", true, true);
	Texture planeSpec(TextureType::SPECULAR, 1, "res/textures/Solid_grey.png", false, true);

	Texture cubeDiff(TextureType::DIFFUSE, 2, "res/textures/container2.png", true, true);
	Texture cubeSpec(TextureType::SPECULAR, 3, "res/textures/specular.png", true, true);

	// directional light
	glm::vec3 lightPos(2.0f, 2.5f, -1.0f); // negative of light direction (relative to origin)
	//lightPos *= 1.5f;
	glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 7.5f);
	glm::mat4 lightSpace = lightProjection * lightView;

	glUseProgram(depthProgram);
	glUniformMatrix4fv(glGetUniformLocation(depthProgram, "lightSpace"), 1, GL_FALSE, glm::value_ptr(lightSpace));

	glUseProgram(coreProgram);
	glUniformMatrix4fv(glGetUniformLocation(coreProgram, "lightSpace"), 1, GL_FALSE, glm::value_ptr(lightSpace));
	glUniform3fv(glGetUniformLocation(coreProgram, ("dirLight.direction")), 1, glm::value_ptr(-lightPos));
	glUniform3f(glGetUniformLocation(coreProgram, ("dirLight.ambient")), 0.1f, 0.1f, 0.1f);
	glUniform3f(glGetUniformLocation(coreProgram, ("dirLight.diffuse")), 0.5f, 0.5f, 0.5f);
	glUniform3f(glGetUniformLocation(coreProgram, ("dirLight.specular")), 0.7f, 0.7f, 0.7f);
	glUniform1f(glGetUniformLocation(coreProgram, "material.shininess"), 4.0f);
	glUniform1i(glGetUniformLocation(coreProgram, "shadowMap"), 4);


	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		Handler::MaintainKeyboard(window);

		int viewportParams[4];
		glGetIntegerv(GL_VIEWPORT, viewportParams);

		// depth map
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glCullFace(GL_FRONT);
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glClear(GL_DEPTH_BUFFER_BIT);

		drawCube(depthProgram, cubeVAO);
		drawPlane(depthProgram, planeVAO);

		// scene
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glCullFace(GL_BACK);
		glViewport(0, 0, viewportParams[2], viewportParams[3]);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 view = std::move(camera.GetViewMatrix());
		glm::mat4 projection = glm::perspective(glm::radians(camera.fov), 800.0f / 600.0f, 0.1f, 100.0f);

		glUseProgram(coreProgram);
		glUniformMatrix4fv(glGetUniformLocation(coreProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(coreProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		drawPlane(coreProgram, planeVAO);
		drawCube(coreProgram, cubeVAO);

		glfwSwapBuffers(window);
		glFlush();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	glDeleteProgram(coreProgram);
}