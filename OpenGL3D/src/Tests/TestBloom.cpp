#include "pch.h"

#include "Utility.h"
#include "Camera.h"
#include "Handler.h"
#include "Texture.h"
#include "Shader.h"

#include "CubeData.h"

static float exposure = 0.5f;

static void handleKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_J)
			exposure += 0.1f;
		else if (key == GLFW_KEY_K)
			exposure -= 0.1f;
	}

	Handler::HandleKeyboard(window, key, scancode, action, mods);
}

static void createFramebuffer(unsigned int textureCount, bool depthBuffer, GLuint& outFramebuffer, GLuint* outTextures)
{
	glGenFramebuffers(1, &outFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, outFramebuffer);

	glGenTextures(textureCount, outTextures);

	for (unsigned int i = 0; i < textureCount; i++)
	{
		glBindTexture(GL_TEXTURE_2D, outTextures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 800, 600, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, outTextures[i], 0);
	}

	if (depthBuffer)
	{
		GLuint rbo;
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
	}

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "[Error: createFramebuffer] Framebuffer is not complete!" << std::endl;
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

static void drawCube(GLuint shader, GLuint vao)
{
	glUseProgram(shader);
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, CubeData::indexCount, GL_UNSIGNED_INT, 0);
}

static void drawQuad(GLuint shader, GLuint vao)
{
	glUseProgram(shader);
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void TestBloom()
{
	GLFWwindow* window = Utility::setupGLFW();
	Utility::setupGLEW();

	glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// floating point framebuffer
	GLuint framebuffer;
	GLuint colorBuffers[2];
	createFramebuffer(2, true, framebuffer, colorBuffers);

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	const unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);

	// blur ping pong framebuffers

	GLuint blurFBOs[2] = {};
	GLuint blurCBOs[2] = {};

	for (unsigned int i = 0; i < 2; i++)
		createFramebuffer(1, false, blurFBOs[i], &blurCBOs[i]);

	// Initialise shaders and load programs.
	GLuint coreProgram;
	Shader::loadProgram(coreProgram, "res/shaders/Bloom/VertexCore.glsl", "res/shaders/Bloom/FragmentCore.glsl");

	GLuint lightProgram;
	Shader::loadProgram(lightProgram, "res/shaders/VertexCore.glsl", "res/shaders/FragmentLight.glsl");

	GLuint quadProgram;
	Shader::loadProgram(quadProgram, "res/shaders/frame/VertexQuad.glsl", "res/shaders/frame/FragmentQuad.glsl");

	GLuint blurProgram;
	Shader::loadProgram(blurProgram, "res/shaders/frame/VertexQuad.glsl", "res/shaders/Bloom/FragmentBlur.glsl");

	GLuint cubeVAO = createCubeVertexArray();
	GLuint quadVAO = createQuadVertexArray();

	// Initialise textures and materials.
	stbi_set_flip_vertically_on_load(true);

	Texture wallDiff(TextureType::DIFFUSE, 0, "res/textures/container2.png", true, true);
	Texture wallSpec(TextureType::SPECULAR, 1, "res/textures/specular.png", true, true);

	// set fixed core program uniforms
	glUseProgram(coreProgram);

	glUniform1f(glGetUniformLocation(coreProgram, "material.shininess"), 32.0f);
	glUniform1i(glGetUniformLocation(coreProgram, "material.diffuse"), 0);
	glUniform1i(glGetUniformLocation(coreProgram, "material.specular"), 1);

	// set quad and blur program uniforms
	glm::mat4 model(1.0f);

	glUseProgram(quadProgram);
	glUniformMatrix4fv(glGetUniformLocation(quadProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniform1i(glGetUniformLocation(quadProgram, "mode"), 8);
	glUniform1i(glGetUniformLocation(quadProgram, "aTex"), 2);
	glUniform1i(glGetUniformLocation(quadProgram, "bloomBlur"), 3);

	glUseProgram(blurProgram);
	glUniformMatrix4fv(glGetUniformLocation(blurProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniform1i(glGetUniformLocation(blurProgram, "aTex"), 3);

	// Initialise window state.
	Camera camera(
		glm::vec3(0.0f, 0.0f, 7.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		-90.0f,
		0.0f
	);

	bool spotlightOn = true;

	Handler handler(window, camera, spotlightOn);
	glfwSetKeyCallback(window, handleKeyboard);

	for (int i = 0; i < 4; i++)
	{
		LightData::pointLights[i].ambient *= 0.1f;
		LightData::pointLights[i].linear = 1.0f;
	}

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		Handler::MaintainKeyboard(window);

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);


		glm::mat4 view = std::move(camera.GetViewMatrix());
		glm::mat4 projection = glm::perspective(glm::radians(camera.fov), 800.0f / 600.0f, 0.1f, 100.0f);

		glUseProgram(lightProgram);
		glUniformMatrix4fv(glGetUniformLocation(lightProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(lightProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glUseProgram(coreProgram);
		glUniformMatrix4fv(glGetUniformLocation(coreProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(coreProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniform3fv(glGetUniformLocation(coreProgram, "viewPos"), 1, glm::value_ptr(camera.pos));

		for (int i = 0; i < 4; i++)
		{
			const PointLight& pointLight = LightData::pointLights[i];

			// ************ LIGHT ************ //

			glm::mat4 model = glm::translate(glm::mat4(1.0f), pointLight.position);
			model = glm::scale(model, glm::vec3(0.2f));

			glUseProgram(lightProgram);
			glUniformMatrix4fv(glGetUniformLocation(lightProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
			glUniform3fv(glGetUniformLocation(lightProgram, "lightColor"), 1, glm::value_ptr(pointLight.specular));

			drawCube(lightProgram, cubeVAO);

			// ************ CORE ************ //
			glUseProgram(coreProgram);

			std::string number = std::to_string(i);
			glUniform3fv(glGetUniformLocation(coreProgram, ("pointLights[" + number + "].position").c_str()), 1, glm::value_ptr(pointLight.position));
			glUniform1f(glGetUniformLocation(coreProgram, ("pointLights[" + number + "].constant").c_str()), pointLight.constant);
			glUniform1f(glGetUniformLocation(coreProgram, ("pointLights[" + number + "].linear").c_str()), pointLight.linear);
			glUniform1f(glGetUniformLocation(coreProgram, ("pointLights[" + number + "].quadratic").c_str()), pointLight.quadratic);
			glUniform3fv(glGetUniformLocation(coreProgram, ("pointLights[" + number + "].ambient").c_str()), 1, glm::value_ptr(pointLight.ambient));
			glUniform3fv(glGetUniformLocation(coreProgram, ("pointLights[" + number + "].diffuse").c_str()), 1, glm::value_ptr(pointLight.diffuse));
			glUniform3fv(glGetUniformLocation(coreProgram, ("pointLights[" + number + "].specular").c_str()), 1, glm::value_ptr(pointLight.specular));
		}

		// containers

		glUseProgram(coreProgram);

		for (unsigned int i = 0; i < 7; i++)
		{
			glm::mat4 model = glm::translate(glm::mat4(1.0f), CubeData::cubePositions[i]);
			float angle = 20.0f * i;
			model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			glUniformMatrix4fv(glGetUniformLocation(coreProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
			drawCube(coreProgram, cubeVAO);
		}

		// blur
		glDisable(GL_DEPTH_TEST);

		glUseProgram(blurProgram);

		const int amount = 30;
		bool horizontal = true;
		int idx = 0;

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, colorBuffers[1]);

		for (unsigned int i = 0; i < amount; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, blurFBOs[idx]);
			glClear(GL_COLOR_BUFFER_BIT);

			glUniform1i(glGetUniformLocation(blurProgram, "horizontal"), horizontal);
			drawQuad(blurProgram, quadVAO);
			
			glBindTexture(GL_TEXTURE_2D, blurCBOs[idx]);
			
			idx = 1 - idx;
			horizontal = !horizontal;
		}

		// quad
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);

		glUseProgram(quadProgram);
		glUniform1f(glGetUniformLocation(quadProgram, "exposure"), exposure);

		drawQuad(quadProgram, quadVAO);

		glfwSwapBuffers(window);
		glFlush();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	glDeleteProgram(coreProgram);
}
