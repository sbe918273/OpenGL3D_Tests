#include "pch.h"

#include <vector>

#include "Utility.h"
#include "Camera.h"
#include "Handler.h"
#include "Texture.h"
#include "Shader.h"

#include "CubeData.h"
#include "StencilData.h"

static const float near = 1.0f;
static const float far = 25.0f;

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
	glUseProgram(shader);
	glUniform1i(glGetUniformLocation(shader, "material.diffuse"), 2);
	glUniform1i(glGetUniformLocation(shader, "material.specular"), 3);
	glBindVertexArray(vao);

	for (const glm::vec3& pos : StencilData::cubePositions)
	{
		glm::mat4 model(1.0f);
		model = glm::translate(model, pos);
		model = glm::translate(model, glm::vec3(3.0f, 0.0f, -3.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	}
}

static void emplaceShadowTransforms(std::vector<glm::mat4>& outTransforms, const glm::vec3& lightPos)
{
	float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
	glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near, far);

	outTransforms.emplace_back(shadowProj *
		glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	outTransforms.emplace_back(shadowProj *
		glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	outTransforms.emplace_back(shadowProj *
		glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
	outTransforms.emplace_back(shadowProj *
		glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
	outTransforms.emplace_back(shadowProj *
		glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
	outTransforms.emplace_back(shadowProj *
		glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
}

static void createDepthMap(GLuint& outFrameBuffer, GLuint& outCubeMap, unsigned int textureUnit)
{
	glGenTextures(1, &outCubeMap);
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, outCubeMap);
	for (unsigned int i = 0; i < 6; i++)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
			SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glGenFramebuffers(1, &outFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, outFrameBuffer);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, outCubeMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "[Error: createDepthCubeMap] Depth framebuffer is not complete!" << std::endl;

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
		 5.0f, -0.5f,  5.0f, 0.0f, 1.0f, 0.0f, 2.0f, 2.0f,
		-5.0f, -0.5f,  5.0f, 0.0f, 1.0f, 0.0f, 0.0f, 2.0f,
		-5.0f, -0.5f, -5.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		 5.0f, -0.5f, -5.0f, 0.0f, 1.0f, 0.0f, 2.0f, 0.0f
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

void TestPointShadow()
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
	Shader::loadProgram(coreProgram, "res/shaders/PointShadow/VertexCore.glsl", "res/shaders/PointShadow/FragmentCore.glsl");

	GLuint depthProgram;
	Shader::loadProgram(depthProgram, "res/shaders/PointShadow/VertexDepth.glsl",
		"res/shaders/PointShadow/GeometryDepth.glsl",
		"res/shaders/PointShadow/FragmentDepth.glsl");

	GLuint lightProgram;
	Shader::loadProgram(lightProgram, "res/shaders/VertexCore.glsl", "res/shaders/FragmentLight.glsl");

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

	// point light

	glUseProgram(depthProgram);
	glUniform1f(glGetUniformLocation(depthProgram, "farPlane"), far);

	glUseProgram(coreProgram);
	glUniform1i(glGetUniformLocation(coreProgram, "depthMap"), 4);
	glUniform1f(glGetUniformLocation(coreProgram, "farPlane"), far);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		Handler::MaintainKeyboard(window);

		int viewportParams[4];
		glGetIntegerv(GL_VIEWPORT, viewportParams);

		// depth map
		glCullFace(GL_FRONT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glClear(GL_DEPTH_BUFFER_BIT);

		glUseProgram(depthProgram);

		glUniform3fv(glGetUniformLocation(depthProgram, "lightPos"), 1, glm::value_ptr(LightData::pointLights[0].position));

		std::vector<glm::mat4> shadowTransforms;
		emplaceShadowTransforms(shadowTransforms, LightData::pointLights[0].position);

		for (unsigned int i = 0; i < 6; i++)
		{
			const std::string number = std::to_string(i);
			glUniformMatrix4fv(glGetUniformLocation(depthProgram,
				("shadowTransforms[" + number + "]").c_str()), 1, GL_FALSE, glm::value_ptr(shadowTransforms[i]));
		}
		
		drawCube(depthProgram, cubeVAO);
		drawPlane(depthProgram, planeVAO);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glCullFace(GL_BACK);
		glViewport(0, 0, viewportParams[2], viewportParams[3]);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 view = std::move(camera.GetViewMatrix());
		glm::mat4 projection = glm::perspective(glm::radians(camera.fov), 800.0f / 600.0f, near, far);

		// light
		glm::mat4 model = glm::translate(glm::mat4(1.0f), LightData::pointLights[0].position);
		model = glm::scale(model, glm::vec3(0.2f));

		glUseProgram(lightProgram);
		glUniformMatrix4fv(glGetUniformLocation(lightProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(lightProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(lightProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniform3fv(glGetUniformLocation(lightProgram, "lightColor"), 1, glm::value_ptr(LightData::pointLights[0].specular));

		glBindVertexArray(cubeVAO);
		glDrawElements(GL_TRIANGLES, CubeData::indexCount, GL_UNSIGNED_INT, 0);

		// rest of scene
		glUseProgram(coreProgram);
		glUniformMatrix4fv(glGetUniformLocation(coreProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(coreProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniform3fv(glGetUniformLocation(coreProgram, "pointLight.position"), 1, glm::value_ptr(LightData::pointLights[0].position));
		glUniform1f(glGetUniformLocation(coreProgram, "pointLight.constant"), LightData::pointLights[0].constant);
		glUniform1f(glGetUniformLocation(coreProgram, "pointLight.linear"), LightData::pointLights[0].linear);
		glUniform1f(glGetUniformLocation(coreProgram, "pointLight.quadratic"), LightData::pointLights[0].quadratic);
		glUniform3fv(glGetUniformLocation(coreProgram, "pointLight.ambient"), 1, glm::value_ptr(0.5f * LightData::pointLights[0].ambient));
		glUniform3fv(glGetUniformLocation(coreProgram, "pointLight.diffuse"), 1, glm::value_ptr(LightData::pointLights[0].diffuse));
		glUniform3fv(glGetUniformLocation(coreProgram, "pointLight.specular"), 1, glm::value_ptr(LightData::pointLights[0].specular));

		glUniform1f(glGetUniformLocation(coreProgram, "material.shininess"), 32.0f);

		glUniform1i(glGetUniformLocation(coreProgram, "material.diffuse"), 0);
		glUniform1i(glGetUniformLocation(coreProgram, "material.specular"), 1);
		drawPlane(coreProgram, planeVAO);

		glUniform1i(glGetUniformLocation(coreProgram, "material.diffuse"), 2);
		glUniform1i(glGetUniformLocation(coreProgram, "material.specular"), 3);
		drawCube(coreProgram, cubeVAO);

		glfwSwapBuffers(window);
		glFlush();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	glDeleteProgram(coreProgram);
}