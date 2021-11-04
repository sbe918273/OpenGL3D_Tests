#include "pch.h"

#include "Utility.h"
#include "Camera.h"
#include "Handler.h"
#include "Texture.h"
#include "Shader.h"

#include "CubeData.h"

static float gamma = 2.2f;
static bool correctGamma = true;
static float blinn = true;
static Texture sDiffMap;
static Texture diffMap;
static Texture sSpecMap;
static Texture specMap;

static void handleKeyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	stbi_set_flip_vertically_on_load(true);

	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_M)
			gamma += 0.1f;
		else if (key == GLFW_KEY_N)
			gamma = std::max(0.1f, gamma - 0.1f);
		else if (key == GLFW_KEY_V)
		{
			diffMap.changeUnit(0);
			specMap.changeUnit(1);
			correctGamma = false;
		}
		else if (key == GLFW_KEY_C)
			blinn = !blinn;
	}

	else if (action == GLFW_RELEASE)
	{
		if (key == GLFW_KEY_V)
		{
			sDiffMap.changeUnit(0);
			sSpecMap.changeUnit(1);
			correctGamma = true;
		}
		else if (key == GLFW_KEY_C)
			blinn = !blinn;
	}

	Handler::HandleKeyboard(window, key, scancode, action, mods);
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

void TestBlinn()
{
	GLFWwindow* window = Utility::setupGLFW();
	Utility::setupGLEW();

	//glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Initialise shaders and load programs.
	GLuint coreProgram;
	Shader::loadProgram(coreProgram, "res/shaders/Blinn/VertexCore.glsl", "res/shaders/Blinn/FragmentCore.glsl");
	//Shader::loadProgram(coreProgram, "res/shaders/VertexCore.glsl", "res/shaders/FragmentCore.glsl");

	GLuint lightProgram;
	Shader::loadProgram(lightProgram, "res/shaders/VertexCore.glsl", "res/shaders/FragmentLight.glsl");

	GLuint planeVAO = createPlaneVertexArray();
	GLuint cubeVAO = createCubeVertexArray();

	// Initialise window state.
	Camera camera(
		glm::vec3(0.0f, 0.0f, 3.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		-90.0f,
		0.0f
	);

	bool spotlightOn = false;

	Handler handler(window, camera, spotlightOn);

	diffMap = Texture(TextureType::DIFFUSE, 0, "res/textures/wood.png", true, false);
	specMap = Texture(TextureType::SPECULAR, 1, "res/textures/white.png", false, false);
	sDiffMap = Texture(TextureType::DIFFUSE, 0, "res/textures/wood.png", true, true);
	sSpecMap = Texture(TextureType::SPECULAR, 1, "res/textures/white.png", false, true);

	glfwSetKeyCallback(window, handleKeyboard);

	for (int i = 0; i < COUNT_POINT_LIGHT; i++)
		LightData::pointLights[i].position += glm::vec3(0.0f, 1.0f, 0.0f);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		Handler::MaintainKeyboard(window);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 model(1.0f);
		glm::mat4 view = std::move(camera.GetViewMatrix());
		glm::mat4 projection = glm::perspective(glm::radians(camera.fov), 800.0f / 600.0f, 0.1f, 100.0f);

		glBindVertexArray(cubeVAO);

		for (int i = 0; i < COUNT_POINT_LIGHT; i++)
		{
			const PointLight& pointLight = LightData::pointLights[i];

			// ************ LIGHT ************ //
			glUseProgram(lightProgram);

			glm::mat4 model(1.0f);
			model = glm::translate(model, pointLight.position);
			
			model = glm::scale(model, glm::vec3(0.2f));
			glUniformMatrix4fv(glGetUniformLocation(lightProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
			glUniformMatrix4fv(glGetUniformLocation(lightProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(lightProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

			glUniform3fv(glGetUniformLocation(lightProgram, "lightColor"), 1, glm::value_ptr(pointLight.specular));

			glDrawElements(GL_TRIANGLES, CubeData::indexCount, GL_UNSIGNED_INT, 0);

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

		glUseProgram(coreProgram);

		// setup uniforms
		//glUniform3f(glGetUniformLocation(coreProgram, ("dirLight.direction")), -1.0f, -1.0f, -1.0f);
		//glUniform3f(glGetUniformLocation(coreProgram, ("dirLight.ambient")), 0.06f, 0.02f, 0.2f);
		//glUniform3f(glGetUniformLocation(coreProgram, ("dirLight.diffuse")), 0.15f, 0.05f, 0.5f);
		//glUniform3f(glGetUniformLocation(coreProgram, ("dirLight.specular")), 0.21f, 0.07f, 0.7f);

		glUniform3fv(glGetUniformLocation(coreProgram, ("spotlight.position")), 1, glm::value_ptr(camera.pos));
		glUniform3fv(glGetUniformLocation(coreProgram, ("spotlight.direction")), 1, glm::value_ptr(camera.front));

		glUniform3f(glGetUniformLocation(coreProgram, ("spotlight.ambient")), 0.05f, 0.05f, 0.05f);
		glUniform3f(glGetUniformLocation(coreProgram, ("spotlight.diffuse")), 0.5f, 0.5f, 0.5f);
		glUniform3f(glGetUniformLocation(coreProgram, ("spotlight.specular")), 0.9f, 0.9f, 0.9f);

		glUniform1f(glGetUniformLocation(coreProgram, ("spotlight.constant")), 1.0f);
		glUniform1f(glGetUniformLocation(coreProgram, ("spotlight.linear")), 0.002f);
		glUniform1f(glGetUniformLocation(coreProgram, ("spotlight.quadratic")), 0.0032f);

		glUniform1f(glGetUniformLocation(coreProgram, ("spotlight.cutOff")), glm::cos(glm::radians(12.5f)));
		glUniform1f(glGetUniformLocation(coreProgram, ("spotlight.outerCutOff")), glm::cos(glm::radians(17.5f)));

		glUniform1i(glGetUniformLocation(coreProgram, ("spotlight.on")), spotlightOn);

		glUniformMatrix4fv(glGetUniformLocation(coreProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(coreProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(coreProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glUniform1i(glGetUniformLocation(coreProgram, "material.diffuse"), 0);
		glUniform1i(glGetUniformLocation(coreProgram, "material.specular"), 1);
		glUniform1f(glGetUniformLocation(coreProgram, "material.shininess"), 16.0f);


		glUniform1i(glGetUniformLocation(coreProgram, "blinn"), blinn);
		glUniform1f(glGetUniformLocation(coreProgram, "gamma"), gamma);
		glUniform1i(glGetUniformLocation(coreProgram, "correctGamma"), correctGamma);


		// *********** //

		glBindVertexArray(planeVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glfwSwapBuffers(window);
		glFlush();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	glDeleteProgram(coreProgram);
}