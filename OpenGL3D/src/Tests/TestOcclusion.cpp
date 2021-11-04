#include "pch.h"

#include <random>

#include "Utility.h"
#include "Camera.h"
#include "Handler.h"
#include "Texture.h"
#include "Shader.h"

#include "CubeData.h"

static const float near = 0.5f;
static const float far = 50.0f;
static const float elevation = 0.1f;

static void createFramebuffer(GLuint& outFBO, GLuint& outCBO)
{
	glGenFramebuffers(1, &outFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, outFBO);

	glGenTextures(1, &outCBO);
	glBindTexture(GL_TEXTURE_2D, outCBO);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 800, 600, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outCBO, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void emplaceRandomVectors(unsigned int count, bool rotation, glm::vec3* outVectors)
{
	std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
	std::default_random_engine generator;
	for (unsigned int i = 0; i < count; i++)
	{
		// random vector above surface in tangent space.
		glm::vec3 sample(
			randomFloats(generator) * 2.0f - 1.0f,
			randomFloats(generator) * 2.0f - 1.0f,
			rotation ? 0.0f : randomFloats(generator)
		);

		if (!rotation)
		{
			sample = glm::normalize(sample);
			sample *= randomFloats(generator);

			float scale = (float)i / 64.0;
			scale = 0.1f + scale * scale * (1.0f - 0.1f);
			sample *= scale;
		}

		outVectors[i] = sample;
	}
}

static void createGBuffer(GLuint* outFrame, GLuint* outPos, GLuint* outNormal, GLuint* outDiff, GLuint* outSpec)
{
	glGenFramebuffers(1, outFrame);
	glBindFramebuffer(GL_FRAMEBUFFER, *outFrame);

	GLenum formats[] = { GL_RGBA16F, GL_RGBA16F, GL_RGBA, GL_RGBA };
	GLuint* buffers[] = { outPos, outNormal, outDiff, outSpec };
	GLuint attachments[4] = {};

	for (unsigned int i = 0; i < 4; i++)
	{
		glGenTextures(1, buffers[i]);
		glBindTexture(GL_TEXTURE_2D, *buffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, formats[i], 800, 600, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, *buffers[i], 0);

		attachments[i] = GL_COLOR_ATTACHMENT0 + i;
	}

	glDrawBuffers(4, attachments);

	GLuint rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 800, 600);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);


	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void drawPlane(GLuint shader, GLuint vao)
{
	glUseProgram(shader);
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

static void drawCube(GLuint shader, GLuint vao)
{
	glUseProgram(shader);
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, CubeData::indexCount, GL_UNSIGNED_INT, 0);
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

static GLuint createQuadVertexArray()
{
	static const float quadVertices[] =
	{
		-1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f, 1.0f, 1.0f, 0.0f,
		 1.0f,  1.0f, 1.0f, 1.0f, 1.0f,
		-1.0f,  1.0f, 1.0f, 0.0f, 1.0f,
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

void TestOcclusion()
{
	GLFWwindow* window = Utility::setupGLFW();
	Utility::setupGLEW();

	glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glDisable(GL_BLEND);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	GLuint deferFBO, deferPos, deferNorm, deferDiff, deferSpec;
	createGBuffer(&deferFBO, &deferPos, &deferNorm, &deferDiff, &deferSpec);


	GLuint ssaoFBO, ssaoCBO;
	createFramebuffer(ssaoFBO, ssaoCBO);

	GLuint blurFBO, blurCBO;
	createFramebuffer(blurFBO, blurCBO);

	glm::vec3* kernel = new glm::vec3[64];
	emplaceRandomVectors(64, false, kernel);

	glm::vec3* noiseVects = new glm::vec3[16];
	emplaceRandomVectors(16, true, noiseVects);

	GLuint noiseTexture;
	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &noiseVects[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// load shaders and VAOs
	GLuint lightProgram;
	Shader::loadProgram(lightProgram, "res/shaders/VertexCore.glsl", "res/shaders/Circle/FragmentLight.glsl");

	GLuint deferProgram; //view space
	Shader::loadProgram(deferProgram, "res/shaders/Occlusion/VertexCore.glsl", "res/shaders/Deferred/FragmentDeferred.glsl"); 

	GLuint quadProgram;
	Shader::loadProgram(quadProgram, "res/shaders/Frame/VertexQuad.glsl", "res/shaders/Occlusion/FragmentQuad.glsl");

	GLuint ssaoProgram;
	Shader::loadProgram(ssaoProgram, "res/shaders/Occlusion/VertexOcclusion.glsl", "res/shaders/Occlusion/FragmentOcclusion.glsl");

	GLuint blurProgram;
	Shader::loadProgram(blurProgram, "res/shaders/Occlusion/VertexOcclusion.glsl", "res/shaders/Occlusion/FragmentBlur.glsl");

	GLuint planeVAO = createPlaneVertexArray();
	GLuint cubeVAO = createCubeVertexArray();
	GLuint quadVAO = createQuadVertexArray();

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

	Texture cubeDiff(TextureType::DIFFUSE, 0, "res/textures/container2.png", true, true);
	Texture cubeSpec(TextureType::SPECULAR, 1, "res/textures/specular.png", true, true);

	glUseProgram(quadProgram);
	glm::mat4 model(1.0f);
	glUniformMatrix4fv(glGetUniformLocation(quadProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniform1f(glGetUniformLocation(quadProgram, "shininess"), 16.0f);

	GLuint buffers[] = { deferPos, deferNorm, deferDiff, deferSpec, blurCBO };
	const char* uniformNames[] = { "gPos", "gNormal", "gDiff", "gSpec", "ssaoTex" };
	for (unsigned int i = 0; i <= 4; i++)
	{
		glActiveTexture(GL_TEXTURE2 + i);
		glBindTexture(GL_TEXTURE_2D, buffers[i]);
		glUniform1i(glGetUniformLocation(quadProgram, uniformNames[i]), i + 2);
	}

	glUseProgram(deferProgram);
	glUniform1i(glGetUniformLocation(deferProgram, "material.diffuse"), 0);
	glUniform1i(glGetUniformLocation(deferProgram, "material.specular"), 1);
	glUniform1i(glGetUniformLocation(deferProgram, "instanced"), 0);

	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);

	glUseProgram(ssaoProgram);
	glUniform1i(glGetUniformLocation(ssaoProgram, "gPos"), 2);
	glUniform1i(glGetUniformLocation(ssaoProgram, "gNormal"), 3);
	glUniform1i(glGetUniformLocation(ssaoProgram, "texNoise"), 8);

	for (unsigned int i = 0; i < 64; i++)
		glUniform3fv(glGetUniformLocation(ssaoProgram, 
			("samples[" + std::to_string(i) + "]").c_str()), 1, glm::value_ptr(kernel[i]));

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, ssaoCBO);

	glUseProgram(blurProgram);
	glUniform1i(glGetUniformLocation(blurProgram, "ssaoTex"), 7);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		Handler::MaintainKeyboard(window);

		// geometry pass
		glBindFramebuffer(GL_FRAMEBUFFER, deferFBO);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_FRAMEBUFFER_SRGB);

		glm::mat4 view = std::move(camera.GetViewMatrix());
		glm::mat4 projection = glm::perspective(glm::radians(camera.fov), 800.0f / 600.0f, 0.2f, 50.0f);

		glUseProgram(deferProgram);
		glUniformMatrix4fv(glGetUniformLocation(deferProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(deferProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, elevation, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(deferProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		cubeDiff.changeUnit(0);
		cubeSpec.changeUnit(1);
		drawCube(deferProgram, cubeVAO);

		model = glm::mat4(1.0f);
		glUniformMatrix4fv(glGetUniformLocation(deferProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		planeDiff.changeUnit(0);
		planeDiff.changeUnit(1);
		drawPlane(deferProgram, planeVAO);

		// ssao pass
		glDisable(GL_DEPTH_TEST);

		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(ssaoProgram);
		glUniformMatrix4fv(glGetUniformLocation(ssaoProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		drawPlane(ssaoProgram, quadVAO);

		// blur ssao texture
		glBindFramebuffer(GL_FRAMEBUFFER, blurFBO);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(blurProgram);
		drawPlane(blurProgram, quadVAO);

		// lighting pass
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_FRAMEBUFFER_SRGB);

		// fill depth buffer
		glEnable(GL_DEPTH_TEST);
		glUseProgram(deferProgram);

		model =  glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, elevation, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(deferProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		drawCube(deferProgram, cubeVAO);
		model = glm::mat4(1.0f);
		glUniformMatrix4fv(glGetUniformLocation(deferProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		drawPlane(deferProgram, planeVAO);

		glDisable(GL_DEPTH_TEST);
		glUseProgram(quadProgram);
		glUniform3fv(glGetUniformLocation(quadProgram, "pointLight.position"), 1, glm::value_ptr(LightData::pointLights[0].position));
		glUniform1f(glGetUniformLocation(quadProgram, "pointLight.constant"), LightData::pointLights[0].constant);
		glUniform1f(glGetUniformLocation(quadProgram, "pointLight.linear"), LightData::pointLights[0].linear);
		glUniform1f(glGetUniformLocation(quadProgram, "pointLight.quadratic"), LightData::pointLights[0].quadratic);
		glUniform3fv(glGetUniformLocation(quadProgram, "pointLight.ambient"), 1, glm::value_ptr(0.5f * LightData::pointLights[0].ambient));
		glUniform3fv(glGetUniformLocation(quadProgram, "pointLight.diffuse"), 1, glm::value_ptr(LightData::pointLights[0].diffuse));
		glUniform3fv(glGetUniformLocation(quadProgram, "pointLight.specular"), 1, glm::value_ptr(LightData::pointLights[0].specular));
		glUniformMatrix4fv(glGetUniformLocation(quadProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		drawPlane(quadProgram, quadVAO);

		glEnable(GL_DEPTH_TEST);
		glUseProgram(lightProgram);
		glUniform3fv(glGetUniformLocation(lightProgram, "lightColor"), 1, glm::value_ptr(LightData::pointLights[0].specular));

		model = glm::translate(glm::mat4(1.0f), LightData::pointLights[0].position);
		model = glm::scale(model, glm::vec3(0.2f));
		glUniformMatrix4fv(glGetUniformLocation(lightProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(lightProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(lightProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		drawCube(lightProgram, cubeVAO);

		glfwSwapBuffers(window);
		glFlush();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	glDeleteProgram(deferProgram);
	glDeleteProgram(lightProgram);
	glDeleteProgram(quadProgram);
}