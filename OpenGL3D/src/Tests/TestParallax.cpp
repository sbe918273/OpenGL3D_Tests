#include "pch.h"

#include "Utility.h"
#include "Camera.h"
#include "Handler.h"
#include "Texture.h"
#include "Shader.h"

#include "CubeData.h"

static void calcTangents(const glm::vec3* pos, const glm::vec2* uv, glm::vec3& outTangent, glm::vec3& outBitangent)
{
	const glm::vec3 edge1 = pos[1] - pos[0];
	const glm::vec3 edge2 = pos[2] - pos[0];
	const glm::vec2 deltaUV1 = uv[1] - uv[0];
	const glm::vec2 deltaUV2 = uv[2] - uv[0];

	const float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

	const glm::mat2 uvMatrix(
		deltaUV2.y, -deltaUV2.x,
		-deltaUV1.y, deltaUV1.x
	);

	const glm::mat3x2 edgeMatrix(
		edge1.x, edge2.x,
		edge1.y, edge2.y,
		edge1.z, edge2.z
	);

	const glm::mat3x2 tangentMatrix = f * uvMatrix * edgeMatrix;

	outTangent = glm::vec3(tangentMatrix[0][0], tangentMatrix[1][0], tangentMatrix[2][0]);
	outBitangent = glm::vec3(tangentMatrix[0][1], tangentMatrix[1][1], tangentMatrix[2][1]);
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

static GLuint createPlaneVertexArray()
{

	static const float vertices[] = {
		// positions        normals		       texCoords
		 5.0f, 0.0f,  5.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		-5.0f, 0.0f,  5.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		-5.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		 5.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f
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


static void drawPlane(GLuint shader, GLuint vao)
{
	glm::mat4 model(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -2.0f, -2.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	//model = glm::scale(model, glm::vec3(100.5f, 105.0f, 105.5f));

	glUseProgram(shader);
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniform1i(glGetUniformLocation(shader, "material.diffuse"), 0);
	glUniform1i(glGetUniformLocation(shader, "material.specular"), 1);
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

static void drawLightCube(GLuint shader, GLuint vao)
{
	glm::mat4 model(1.0f);
	model = glm::translate(model, LightData::pointLights[0].position);
	model = glm::scale(model, glm::vec3(0.2f));

	glUseProgram(shader);
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniform3fv(glGetUniformLocation(shader, "lightColor"), 1, glm::value_ptr(LightData::pointLights[0].specular));

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, CubeData::indexCount, GL_UNSIGNED_INT, 0);
}


void TestParallax()
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

	const glm::vec3 pos[] = {
		{ 5.0f, 0.0f,  5.0f},
		{-5.0f, 0.0f,  5.0f},
		{-5.0f, 0.0f, -5.0f},
	};

	const glm::vec2 texCoords[] = {
		{1.0f, 1.0f},
		{0.0f, 1.0f},
		{0.0f, 0.0f},
	};

	const glm::vec3 normal(0.0f, 1.0f, 0.0f);

	glm::vec3 tangent, bitangent;
	calcTangents(pos, texCoords, tangent, bitangent);

	// Initialise shaders and load programs.
	GLuint coreProgram;
	Shader::loadProgram(coreProgram, "res/shaders/Normal/VertexCore.glsl", "res/shaders/Parallax/FragmentCore.glsl");

	GLuint lightProgram;
	Shader::loadProgram(lightProgram, "res/shaders/VertexCore.glsl", "res/shaders/FragmentLight.glsl");

	GLuint planeVAO = createPlaneVertexArray();
	GLuint cubeVAO = createCubeVertexArray();

	// Initialise textures and materials.
	stbi_set_flip_vertically_on_load(true);

	/*Texture wallDiff(TextureType::DIFFUSE, 0, "res/textures/bricks2.jpg", false, true);
	Texture wallSpec(TextureType::SPECULAR, 1, "res/textures/bricks2.jpg", false, true);
	Texture wallNorm(TextureType::OTHER, 2, "res/textures/bricks2_normal.jpg", false, false);
	Texture wallDisp(TextureType::OTHER, 3, "res/textures/bricks2_disp.jpg", false, false);*/

	Texture toyDiff(TextureType::DIFFUSE, 0, "res/textures/wood.png", true, true);
	Texture toySpec(TextureType::SPECULAR, 1, "res/textures/wood.png", true, true);
	Texture toyNorm(TextureType::OTHER, 2, "res/textures/toy_box_normal.png", false, false);
	Texture toyDisp(TextureType::OTHER, 3, "res/textures/toy_box_disp.png", true, false);

	// set fixed core program uniforms
	glUseProgram(coreProgram);

	glUniform1f(glGetUniformLocation(coreProgram, "material.shininess"), 32.0f);
	glUniform1i(glGetUniformLocation(coreProgram, "material.diffuse"), 0);
	glUniform1i(glGetUniformLocation(coreProgram, "material.specular"), 1);
	glUniform1i(glGetUniformLocation(coreProgram, "normalMap"), 2);
	glUniform1i(glGetUniformLocation(coreProgram, "depthMap"), 3);

	glUniform1f(glGetUniformLocation(coreProgram, "pointLight.constant"), LightData::pointLights[0].constant);
	glUniform1f(glGetUniformLocation(coreProgram, "pointLight.linear"), LightData::pointLights[0].linear);
	glUniform1f(glGetUniformLocation(coreProgram, "pointLight.quadratic"), LightData::pointLights[0].quadratic);
	glUniform3fv(glGetUniformLocation(coreProgram, "pointLight.ambient"), 1, glm::value_ptr(0.5f * LightData::pointLights[0].ambient));
	glUniform3fv(glGetUniformLocation(coreProgram, "pointLight.diffuse"), 1, glm::value_ptr(0.7f * LightData::pointLights[0].diffuse));
	glUniform3fv(glGetUniformLocation(coreProgram, "pointLight.specular"), 1, glm::value_ptr(1.1f * LightData::pointLights[0].specular));

	glUniform3fv(glGetUniformLocation(coreProgram, "tangent"), 1, glm::value_ptr(tangent));
	glUniform3fv(glGetUniformLocation(coreProgram, "bitangent"), 1, glm::value_ptr(bitangent));
	glUniform3fv(glGetUniformLocation(coreProgram, "normal"), 1, glm::value_ptr(normal));

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
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 view = std::move(camera.GetViewMatrix());
		glm::mat4 projection = glm::perspective(glm::radians(camera.fov), 800.0f / 600.0f, 0.1f, 100.0f);

		// light cube
		glUseProgram(lightProgram);
		glUniformMatrix4fv(glGetUniformLocation(lightProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(lightProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		drawLightCube(lightProgram, cubeVAO);

		// wall
		glUseProgram(coreProgram);
		glUniformMatrix4fv(glGetUniformLocation(coreProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(coreProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glUniform3fv(glGetUniformLocation(coreProgram, "viewPos"), 1, glm::value_ptr(camera.pos));
		glUniform3fv(glGetUniformLocation(coreProgram, "lightPos"), 1, glm::value_ptr(LightData::pointLights[0].position));
		glUniform3fv(glGetUniformLocation(coreProgram, "pointLight.position"), 1, glm::value_ptr(LightData::pointLights[0].position));

		glUniform1i(glGetUniformLocation(coreProgram, "mapNormals"), !(glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS));

		drawPlane(coreProgram, planeVAO);

		glfwSwapBuffers(window);
		glFlush();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	glDeleteProgram(coreProgram);
}
