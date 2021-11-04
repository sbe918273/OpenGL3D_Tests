#include "pch.h"

#include <math.h>
#include <iostream>
#include "Utility.h"

#include "Camera.h"
#include "Handler.h"
#include "Texture.h"
#include "Shader.h"


#include "CubeData.h"

static const float pi = 3.1415926f;

static void drawLights(GLuint lightShader, GLuint cubeVAO, glm::mat4& view, glm::mat4& projection)
{
	for (int i = 0; i < COUNT_POINT_LIGHT; i++)
	{
		const PointLight& pointLight = LightData::pointLights[i];

		glUseProgram(lightShader);

		glm::mat4 model = glm::translate(glm::mat4(1.0f), pointLight.position);
		model = glm::scale(model, glm::vec3(0.2f));
		glUniformMatrix4fv(glGetUniformLocation(lightShader, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(lightShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(lightShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glUniform3fv(glGetUniformLocation(lightShader, "lightColor"), 1, glm::value_ptr(pointLight.specular));

		glBindVertexArray(cubeVAO);
		glDrawElements(GL_TRIANGLES, CubeData::indexCount, GL_UNSIGNED_INT, 0);
	}
}

static void setUniforms(GLuint shader, Camera& camera, bool spotlightOn, glm::mat4& model, glm::mat4& view, glm::mat4& projection)
{
	glUseProgram(shader);

	for (int i = 0; i < COUNT_POINT_LIGHT; i++)
	{
		const PointLight& pointLight = LightData::pointLights[i];
		std::string number = std::to_string(i);
		glUniform3fv(glGetUniformLocation(shader, ("pointLights[" + number + "].position").c_str()), 1, glm::value_ptr(pointLight.position));
		glUniform1f(glGetUniformLocation(shader, ("pointLights[" + number + "].constant").c_str()), pointLight.constant);
		glUniform1f(glGetUniformLocation(shader, ("pointLights[" + number + "].linear").c_str()), pointLight.linear);
		glUniform1f(glGetUniformLocation(shader, ("pointLights[" + number + "].quadratic").c_str()), pointLight.quadratic);
		glUniform3fv(glGetUniformLocation(shader, ("pointLights[" + number + "].ambient").c_str()), 1, glm::value_ptr(pointLight.ambient));
		glUniform3fv(glGetUniformLocation(shader, ("pointLights[" + number + "].diffuse").c_str()), 1, glm::value_ptr(pointLight.diffuse));
		glUniform3fv(glGetUniformLocation(shader, ("pointLights[" + number + "].specular").c_str()), 1, glm::value_ptr(pointLight.specular));
	}
	
	glUseProgram(shader);
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glUniform3fv(glGetUniformLocation(shader, "viewPos"), 1, glm::value_ptr(camera.pos));


	glUniform3f(glGetUniformLocation(shader, ("diffuseColor")), 0.2f, 0.3f, 0.4f);
	glUniform3f(glGetUniformLocation(shader, ("specularColor")), 0.8f, 0.8f, 0.8f);
	glUniform1f(glGetUniformLocation(shader, ("shininess")), 16.0f);

	glUniform3f(glGetUniformLocation(shader, ("dirLight.direction")), -1.0f, -1.0f, -1.0f);
	glUniform3f(glGetUniformLocation(shader, ("dirLight.ambient")), 0.06f, 0.02f, 0.2f);
	glUniform3f(glGetUniformLocation(shader, ("dirLight.diffuse")), 0.15f, 0.05f, 0.5f);
	glUniform3f(glGetUniformLocation(shader, ("dirLight.specular")), 0.21f, 0.07f, 0.7f);

	glUniform3fv(glGetUniformLocation(shader, ("spotlight.position")), 1, glm::value_ptr(camera.pos));
	glUniform3fv(glGetUniformLocation(shader, ("spotlight.direction")), 1, glm::value_ptr(camera.front));

	glUniform3f(glGetUniformLocation(shader, ("spotlight.ambient")), 0.05f, 0.05f, 0.05f);
	glUniform3f(glGetUniformLocation(shader, ("spotlight.diffuse")), 0.5f, 0.5f, 0.5f);
	glUniform3f(glGetUniformLocation(shader, ("spotlight.specular")), 0.9f, 0.9f, 0.9f);

	glUniform1f(glGetUniformLocation(shader, ("spotlight.constant")), 1.0f);
	glUniform1f(glGetUniformLocation(shader, ("spotlight.linear")), 0.09f);
	glUniform1f(glGetUniformLocation(shader, ("spotlight.quadratic")), 0.032f);

	glUniform1f(glGetUniformLocation(shader, ("spotlight.cutOff")), glm::cos(glm::radians(12.5f)));
	glUniform1f(glGetUniformLocation(shader, ("spotlight.outerCutOff")), glm::cos(glm::radians(17.5f)));

	glUniform1i(glGetUniformLocation(shader, ("spotlight.on")), spotlightOn);
}

static void emplaceCirclePoints(unsigned int uSample, unsigned int vSample, float* outVertices)
{
	const unsigned int vertexCount = vSample * (uSample - 2) + 2;
	const unsigned int floatCount = 3 * vertexCount;

	// top centre
	outVertices[0] = 0.0f;
	outVertices[1] = 0.0f;
	outVertices[2] = 1.0f;

	// bottom centre
	outVertices[floatCount - 3] = 0.0f;
	outVertices[floatCount - 2] = 0.0f;
	outVertices[floatCount - 1] = -1.0f;

	const float uInc = pi / (uSample - 1);
	const float vInc = 2 * pi / vSample;

	// Start at second u layer and stop at second to last.
	float u = uInc;
	float v = 0.0f;

	unsigned int idx = 3;

	for (unsigned int uIdx = 0; uIdx < uSample - 2; uIdx++)
	{
		for (unsigned int vIdx = 0; vIdx < vSample; vIdx++)
		{
			outVertices[idx++] = sin(u) * cos(v);
			outVertices[idx++] = sin(u) * sin(v);
			outVertices[idx++] = cos(u);
			v += vInc;
		}
		u += uInc;
		v = 0.0f;
	}
}

static void emplaceCircleIndices(unsigned int uSample, unsigned int vSample, unsigned int* outIndices)
{
	/* 
		uIntCount explanation: 
		3 indices per trianles
		{2 * vSample} triangles in {uSample - 3} rows (as not including top and bottom) + {2 * vSample} triangles at top and bottom.
		6 * vSample * (uSample - 3) + 6 * vSample;
	 */

	unsigned int arrIdx = 0;

	for (unsigned int i = 1; i <= vSample; i++)
	{
		// 0 is index of top centre, 1 is first vertex of first layer
		outIndices[arrIdx++] = 0;
		outIndices[arrIdx++] = i;
		//accounts for joining
		outIndices[arrIdx++] = (i == vSample) ? 1 : (i + 1);
	}

	// swap 2nd and 3rd for originals
	for (unsigned int uIdx = 0; uIdx < uSample - 3; uIdx++)
	{
		// add 1 due to top centre vertex
		const unsigned int lowerStart = uIdx * vSample + 1;
		const unsigned int upperStart = lowerStart + vSample;

		for (unsigned int vIdx = 0; vIdx < vSample - 1; vIdx++)
		{

			/* 
			   up triangle	 |  down triangle 
					1		 |	  0		  1
							 |
				0		2    |		  2
			*/
			outIndices[arrIdx++] = lowerStart + vIdx;  
			outIndices[arrIdx++] = upperStart + vIdx;     
			outIndices[arrIdx++] = lowerStart + vIdx + 1;

			outIndices[arrIdx++] = upperStart + vIdx;
			outIndices[arrIdx++] = upperStart + vIdx + 1;
			outIndices[arrIdx++] = lowerStart + vIdx + 1;
		}

		// edge case triangles
		outIndices[arrIdx++] = upperStart - 1;
		outIndices[arrIdx++] = upperStart + vSample - 1;
		outIndices[arrIdx++] = lowerStart;

		outIndices[arrIdx++] = upperStart + vSample - 1;
		outIndices[arrIdx++] = upperStart;
		outIndices[arrIdx++] = lowerStart;
	}

	// index of bottom centre is vertex count - 1
	const unsigned int bottomIdx = vSample * (uSample - 2) + 1;
	// start of penultimate layer (act as if bottomIdx is first vertex of next layer)
	const unsigned int penStart  = bottomIdx - vSample;

	for (unsigned int i = 0; i < vSample; i++)
	{
		outIndices[arrIdx++] = bottomIdx;
		outIndices[arrIdx++] = (i == vSample - 1) ? penStart : penStart + i + 1;
		outIndices[arrIdx++] = penStart + i;
	}
}

void TestCircle()
{
	GLFWwindow* window = Utility::setupGLFW();
	Utility::setupGLEW();
	
	// Configure OpenGL.
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Initialise shaders and load programs.
	GLuint coreProgram;
	Shader::loadProgram(coreProgram, "res/shaders/circle/VertexCore.glsl", "res/shaders/circle/FragmentCore.glsl");

	GLuint lightProgram;
	Shader::loadProgram(lightProgram, "res/shaders/circle/VertexCore.glsl", "res/shaders/circle/FragmentLight.glsl");

	//circle
	const unsigned int uSample = 256;
	const unsigned int vSample = 256;

	const unsigned int vertexCount = vSample * (uSample - 2) + 2;
	const unsigned int floatCount = 3 * vertexCount;

	const unsigned int uIntCount = 6 * vSample * (uSample - 2);

	float* circleVertices = new float[floatCount];
	emplaceCirclePoints(uSample, vSample, circleVertices);

	unsigned int* circleIndices = new unsigned int[uIntCount];
	emplaceCircleIndices(uSample, vSample, circleIndices);

	GLuint circleVBO;
	glGenBuffers(1, &circleVBO);
	glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
	glBufferData(GL_ARRAY_BUFFER, floatCount * sizeof(float), circleVertices, GL_STATIC_DRAW);

	GLuint circleVAO;
	glCreateVertexArrays(1, &circleVAO);
	glBindVertexArray(circleVAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	
	GLuint circleEBO;
	glGenBuffers(1, &circleEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, circleEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, uIntCount * sizeof(unsigned int), circleIndices, GL_STATIC_DRAW);

	glBindVertexArray(0);

	// cube
	GLuint cubeVBO;
	glGenBuffers(1, &cubeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CubeData::vertices), CubeData::vertices, GL_STATIC_DRAW);

	GLuint cubeVAO;
	glCreateVertexArrays(1, &cubeVAO);
	glBindVertexArray(cubeVAO);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	
	GLuint cubeEBO;
	glGenBuffers(1, &cubeEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(CubeData::indices), CubeData::indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

	// Initialise window state.
	Camera camera(
		glm::vec3(0.0f, 0.0f, 2.0f),
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

		glm::mat4 model(1.0f);
		glm::mat4 view = std::move(camera.GetViewMatrix());
		glm::mat4 projection = glm::perspective(glm::radians(camera.fov), 800.0f / 600.0f, 0.1f, 100.0f);

		drawLights(lightProgram, cubeVAO, view, projection);
		setUniforms(coreProgram, camera, spotlightOn, model, view, projection);

		glBindVertexArray(circleVAO);
		glDrawElements(GL_TRIANGLES, uIntCount, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glFlush();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	glDeleteProgram(coreProgram);
	glDeleteProgram(lightProgram);
}

