#include "pch.h"

#include "Utility.h"
#include "Camera.h"
#include "Handler.h"
#include "Texture.h"
#include "Shader.h"

#include "CubeData.h"
#include "LightData.h"

struct pointLight
{
	glm::vec3 position;

	float constant;
	float linear;
	float quadratic;

	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
};

static const unsigned int xDim = 10;
static const unsigned int yDim = 10;
static const unsigned int count = 100;

static float randFloat(float a, float b) {
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

static void setupModelMatrices(unsigned int xDim, unsigned int zDim, GLuint vao)
{
	const unsigned int count = xDim * yDim;
	glm::mat4* modelMatrices = new glm::mat4[count];

	unsigned int idx = 0;
	for (unsigned int i = 0; i < xDim; i++)
	{
		float xOffset = (float)i - (float)(xDim) / 2.0f;
		for (unsigned int j = 0; j < zDim; j++)
		{
			float zOffset = (float)j - (float)(zDim) / 2.0f;
			glm::vec3 offset = xOffset * glm::vec3(2.0f, 0.0f, 0.0f) 
				+ (float)zOffset * glm::vec3(0.0f, 0.0f, -2.0f);
			modelMatrices[idx++] = glm::translate(glm::mat4(1.0f), offset);
		}
	}

	GLuint modelVBO;
	glGenBuffers(1, &modelVBO);
	glBindBuffer(GL_ARRAY_BUFFER, modelVBO);
	glBufferData(GL_ARRAY_BUFFER, count * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

	glBindVertexArray(vao);

	const unsigned int vecSize = sizeof(glm::vec4);
	for (unsigned int i = 0; i < 4; i++)
	{
		glEnableVertexAttribArray(i + 3);
		glVertexAttribPointer(i + 3, 4, GL_FLOAT, GL_FALSE, 4 * vecSize, (void*)(i * vecSize));
		glVertexAttribDivisor(i + 3, 1);
	}

	glBindVertexArray(0);
}

static void emplacePointLights(unsigned int count, unsigned int xDim, unsigned int zDim, PointLight* outLights)
{
	srand(glfwGetTime());

	float xLim = (float)xDim;
	float yLim = 2.0f;
	float zLim = (float)zDim;

	for (unsigned int i = 0; i < count; i++)
	{
		glm::vec3 color = glm::vec3(randFloat(0.0f, 1.0f), randFloat(0.0f, 1.0f), randFloat(0.0f, 1.0f));
		color = glm::normalize(color) * 1.5f;

		outLights[i] = {
			glm::vec3(randFloat(-xLim, xLim), randFloat(-yLim, yLim), randFloat(-zLim, zLim)),
			1.0f, 0.09f, 0.032f,
			randFloat(0.0f, 0.0f) * color, randFloat(0.6f, 0.7f) * color, randFloat(0.6f, 0.9f) * color
		};
	}
}

static void setupQuadLightUniforms(unsigned int count, GLuint shader, PointLight* lights)
{
	const float limit = 5.0f / 265.0f;

	glUseProgram(shader);
	for (unsigned int i = 0; i < count; i++)
	{
		std::string number = std::to_string(i);
		glUniform3fv(glGetUniformLocation(shader, ("pointLights[" + number + "].position").c_str()), 1, glm::value_ptr(lights[i].position));
		glUniform1f(glGetUniformLocation(shader, ("pointLights[" + number + "].constant").c_str()), lights[i].constant);
		glUniform1f(glGetUniformLocation(shader, ("pointLights[" + number + "].linear").c_str()), lights[i].linear);
		glUniform1f(glGetUniformLocation(shader, ("pointLights[" + number + "].quadratic").c_str()), lights[i].quadratic);
		glUniform3fv(glGetUniformLocation(shader, ("pointLights[" + number + "].ambient").c_str()), 1, glm::value_ptr(lights[i].ambient));
		glUniform3fv(glGetUniformLocation(shader, ("pointLights[" + number + "].diffuse").c_str()), 1, glm::value_ptr(lights[i].diffuse));
		glUniform3fv(glGetUniformLocation(shader, ("pointLights[" + number + "].specular").c_str()), 1, glm::value_ptr(lights[i].specular));

		float lightMax = std::fmaxf(std::fmaxf(lights[i].diffuse.r, lights[i].diffuse.g), lights[i].diffuse.b);
		float radius = (-lights[i].linear + std::sqrtf(lights[i].linear * lights[i].linear - 4 * lights[i].quadratic * (lights[i].constant - lightMax / limit)))
			/ (2 * lights[i].quadratic);
		glUniform1f(glGetUniformLocation(shader, ("pointLights[" + number + "].radius").c_str()), radius);
	}
}

static void setupLightAttributes(unsigned int count, GLuint vao, PointLight* lights)
{
	glm::mat4* modelMatrices = new glm::mat4[count];
	for (unsigned int i = 0; i < count; i++)
	{
		modelMatrices[i] = glm::translate(glm::mat4(1.0f), lights[i].position);
		modelMatrices[i] = glm::scale(modelMatrices[i], glm::vec3(0.2f));
	}

	glm::vec3* colors = new glm::vec3[count];
	for (unsigned int i = 0; i < count; i++)
		colors[i] = lights[i].specular;

	GLuint modelVBO;
	glGenBuffers(1, &modelVBO);
	glBindBuffer(GL_ARRAY_BUFFER, modelVBO);
	glBufferData(GL_ARRAY_BUFFER, count * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

	GLuint colorVBO;
	glGenBuffers(1, &colorVBO);
	glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
	glBufferData(GL_ARRAY_BUFFER, count * sizeof(glm::vec3), &colors[0], GL_STATIC_DRAW);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, modelVBO);
	const unsigned int vecSize = sizeof(glm::vec4);
	for (unsigned int i = 0; i < 4; i++)
	{
		glEnableVertexAttribArray(i + 3);
		glVertexAttribPointer(i + 3, 4, GL_FLOAT, GL_FALSE, 4 * vecSize, (void*)(i * vecSize));
		glVertexAttribDivisor(i + 3, 1);
	}

	glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
	glEnableVertexAttribArray(7);
	glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
	glVertexAttribDivisor(7, 1);

	glBindVertexArray(0);
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

void TestDeferred()

{
	GLFWwindow* window = Utility::setupGLFW();
	Utility::setupGLEW();

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDepthFunc(GL_LEQUAL);
	//glDisable(GL_BLEND);

	GLuint deferFBO, deferPos, deferNorm, deferDiff, deferSpec;
	createGBuffer(&deferFBO, &deferPos, &deferNorm, &deferDiff, &deferSpec);

	// Initialise shaders and load programs.
	GLuint lightProgram;
	Shader::loadProgram(lightProgram, "res/shaders/Deferred/VertexLight.glsl", "res/shaders/Deferred/FragmentLight.glsl");

	GLuint deferProgram;
	Shader::loadProgram(deferProgram, "res/shaders/VertexCore.glsl", "res/shaders/Deferred/FragmentDeferred.glsl");

	GLuint quadProgram;
	Shader::loadProgram(quadProgram, "res/shaders/Frame/VertexQuad.glsl", "res/shaders/Deferred/FragmentQuad.glsl");

	GLuint cubeVAO = createCubeVertexArray();
	GLuint lightVAO = createCubeVertexArray();
	GLuint quadVAO = createQuadVertexArray();

	Camera camera(
		glm::vec3(0.0f, 3.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		-90.0f,
		0.0f
	);

	bool spotlightOn = true;

	Handler handler(window, camera, spotlightOn);

	stbi_set_flip_vertically_on_load(true);
	Texture sphereDiff(TextureType::DIFFUSE, 0, "res/textures/container2.png", true, true);
	Texture sphereSpec(TextureType::SPECULAR, 1, "res/textures/specular.png", true, true);

	glUseProgram(quadProgram);
	glm::mat4 model(1.0f);
	glUniformMatrix4fv(glGetUniformLocation(quadProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniform1f(glGetUniformLocation(quadProgram, "shininess"), 32.0f);

	GLuint buffers[] = { deferPos, deferNorm, deferDiff, deferSpec };
	const char* uniformNames[] = { "gPos", "gNormal", "gDiff", "gSpec" };
	for (unsigned int i = 0; i < 4; i++)
	{
		glActiveTexture(GL_TEXTURE2 + i);
		glBindTexture(GL_TEXTURE_2D, buffers[i]);
		glUniform1i(glGetUniformLocation(quadProgram, uniformNames[i]), i + 2);
	}

	glUseProgram(deferProgram);
	glUniform1i(glGetUniformLocation(deferProgram, "material.diffuse"), 0);
	glUniform1i(glGetUniformLocation(deferProgram, "material.specular"), 1);
	glUniform1i(glGetUniformLocation(deferProgram, "instanced"), 1);

	PointLight* pointLights = new PointLight[count];
	emplacePointLights(count, xDim, yDim, pointLights);
	setupQuadLightUniforms(count, quadProgram, pointLights);
	setupLightAttributes(count, lightVAO, pointLights);

	setupModelMatrices(xDim, yDim, cubeVAO);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		Handler::MaintainKeyboard(window);

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

		glBindVertexArray(cubeVAO);
		glDrawElementsInstanced(GL_TRIANGLES, CubeData::indexCount, GL_UNSIGNED_INT, 0, xDim * yDim);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_FRAMEBUFFER_SRGB);

		glBindVertexArray(cubeVAO);
		glDrawElementsInstanced(GL_TRIANGLES, CubeData::indexCount, GL_UNSIGNED_INT, 0, xDim * yDim);

		glDisable(GL_DEPTH_TEST);
		glUseProgram(quadProgram);
		glUniform3fv(glGetUniformLocation(quadProgram, "viewPos"), 1, glm::value_ptr(camera.pos));
		drawQuad(quadProgram, quadVAO);

		glEnable(GL_DEPTH_TEST);
		glUseProgram(lightProgram);
		glUniformMatrix4fv(glGetUniformLocation(lightProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(lightProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(lightVAO);
		glDrawElementsInstanced(GL_TRIANGLES, CubeData::indexCount, GL_UNSIGNED_INT, 0, count);

		glfwSwapBuffers(window);
		glFlush();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	glDeleteProgram(deferProgram);
}