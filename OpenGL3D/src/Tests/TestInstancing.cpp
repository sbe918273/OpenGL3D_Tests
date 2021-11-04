#include "pch.h"

#include "Utility.h"
#include "Camera.h"
#include "Handler.h"
#include "Texture.h"
#include "Model.h"
#include "Shader.h"

#include "CubeData.h"
#include "LightData.h"


static GLuint loadCubeMap(unsigned int textureUnit)
{
	static const std::string faces[] =
	{
		"bkg1_right1.png",
		"bkg1_left2.png",
		"bkg1_top3.png",
		"bkg1_bottom4.png",
		"bkg1_front5.png",
		"bkg1_back6.png"
	};

	GLuint texture;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

	int width, height, channelCount;
	for (unsigned int i = 0; i < 6; i++)
	{
		const std::string path = "C:/Users/binma/Downloads/bkg/bkg/red/" + faces[i];

		unsigned char* data = stbi_load(path.c_str(), &width, &height, &channelCount, 0);
		if (data)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

		else
			std::cerr << "[Error: loadCubeMap] Cubemap texture failed to load at path"
			<< path << std::endl;
		stbi_image_free(data);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return texture;
}

static GLuint createCubeVertexArray()
{
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CubeData::vertices), CubeData::vertices, GL_STATIC_DRAW);

	GLuint vao;
	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

	GLuint ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(CubeData::indices), CubeData::indices, GL_STATIC_DRAW);

	return vao;
}

static float getDisplacement()
{
	// [0 -> 200a] / 100 - a => [-a, a] 
	static const float offset = 30.0f;
	return (rand() % (int)(200 * offset)) / 100.0f - offset;
}

void TestInstancing()
{
	GLFWwindow* window = Utility::setupGLFW();
	Utility::setupGLEW();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Initialise shaders and load programs.

	GLuint coreProgram;
	Shader::loadProgram(coreProgram, "res/shaders/VertexCore.glsl", "res/shaders/FragmentCore.glsl");

	GLuint lightProgram;
	Shader::loadProgram(lightProgram, "res/shaders/VertexCore.glsl", "res/shaders/FragmentLight.glsl");

	GLuint skyProgram;
	Shader::loadProgram(skyProgram, "res/shaders/CubeMap/VertexCubeMap.glsl", "res/shaders/CubeMap/FragmentCubeMap.glsl");

	GLuint vao = createCubeVertexArray();

	// Initialise textures and materials.
	stbi_set_flip_vertically_on_load(false);

	// Initialise window state.
	Camera camera(
		glm::vec3(0.0f, 0.0f, 30.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		-90.0f,
		0.0f
	);

	bool spotlightOn = false;

	Handler handler(window, camera, spotlightOn);

	// model matrices
	const unsigned int count = 60000;
	glm::mat4* modelMatrices = new glm::mat4[count];
	srand(glfwGetTime());
	const float radius = 100.0f;
	for (unsigned int i = 0; i < count; i++)
	{
		glm::mat4 model(1.0f);

		float angle = (float)i / (float)count * 360.0f;
		glm::vec3 position(
			sin(angle) * radius + getDisplacement(),
			0.4f * getDisplacement(),
			cos(angle) * radius + getDisplacement()
		);
		model = glm::translate(model, position);

		// [0, 20] / 100 + 0.05 => [0.05, 0.25]
		float scale = (rand() % 20 / 100.0f + 0.05f);
		model = glm::scale(model, glm::vec3(scale));

		float rotAngle = (rand() % 360);
		model = glm::rotate(model, glm::radians(rotAngle), glm::vec3(0.4f, 0.6f, 0.8f));

		modelMatrices[i] = model;
	}
	
	Model planetObj("C:/Users/binma/Downloads/planet/planet.obj");


	// rock
	Model rockObj("C:/Users/binma/Downloads/rock/rock.obj");
	Texture rockDiff(TextureType::DIFFUSE, 0, "C:/Users/binma/Downloads/rock/rock.png", false);
	Texture rockSpec(TextureType::SPECULAR, 1, "C:/Users/binma/Downloads/rock/rock.png", false);

	GLuint modelVBO;
	glGenBuffers(1, &modelVBO);
	glBindBuffer(GL_ARRAY_BUFFER, modelVBO);
	glBufferData(GL_ARRAY_BUFFER, count * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

	// skybox
	unsigned int cubeMapTexture = loadCubeMap(2);

	for (unsigned int i = 0; i < rockObj.meshes.size(); i++)
	{
		GLuint modelVAO = rockObj.meshes[i].vao;
		glBindVertexArray(modelVAO);

		std::size_t vec4Size = sizeof(glm::vec4);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);

		glBindVertexArray(0);
	}

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		Handler::MaintainKeyboard(window);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glCullFace(GL_BACK);

		glBindVertexArray(vao);

		glm::mat4 view = std::move(camera.GetViewMatrix());
		glm::mat4 skyView = glm::mat4(glm::mat3(view));
		glm::mat4 projection = glm::perspective(glm::radians(camera.fov), 800.0f / 600.0f, 0.1f, 200.0f);

		for (int i = 0; i < COUNT_POINT_LIGHT; i++)
		{
			const PointLight& pointLight = LightData::pointLights[i];

			// ************ LIGHT ************ //
			glUseProgram(lightProgram);

			glm::mat4 model = glm::translate(glm::mat4(1.0f), pointLight.position);
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

		glUniform3f(glGetUniformLocation(coreProgram, "pointLights[0].ambient"), 0.2f, 0.03f, 0.03f);
		glUniform1f(glGetUniformLocation(coreProgram, "pointLights[0].constant"), 0.8f);
		glUniform1f(glGetUniformLocation(coreProgram, "pointLights[0].linear"), 0.001f);
		glUniform1f(glGetUniformLocation(coreProgram, "pointLights[0].quadratic"), 0.0f);

		glUseProgram(coreProgram);

		glUniform3fv(glGetUniformLocation(coreProgram, "viewPos"), 1, glm::value_ptr(camera.pos));

		glUniform3f(glGetUniformLocation(coreProgram, ("dirLight.direction")), -1.0f, -1.0f, -1.0f);
		//glUniform3f(glGetUniformLocation(coreProgram, ("dirLight.ambient")), 0.06f, 0.02f, 0.2f);
		//glUniform3f(glGetUniformLocation(coreProgram, ("dirLight.diffuse")), 0.15f, 0.05f, 0.5f);
		//glUniform3f(glGetUniformLocation(coreProgram, ("dirLight.specular")), 0.21f, 0.07f, 0.7f);

		glUniform3f(glGetUniformLocation(coreProgram, ("dirLight.ambient")), 0.0f, 0.0f, 0.0f);
		glUniform3f(glGetUniformLocation(coreProgram, ("dirLight.diffuse")), 0.0f, 0.0f, 0.0f);
		glUniform3f(glGetUniformLocation(coreProgram, ("dirLight.specular")), 0.0f, 0.0f, 0.0f);

		glUniform3fv(glGetUniformLocation(coreProgram, ("spotlight.position")), 1, glm::value_ptr(camera.pos));
		glUniform3fv(glGetUniformLocation(coreProgram, ("spotlight.direction")), 1, glm::value_ptr(camera.front));

		glUniform3f(glGetUniformLocation(coreProgram, ("spotlight.ambient")), 0.05f, 0.05f, 0.05f);
		glUniform3f(glGetUniformLocation(coreProgram, ("spotlight.diffuse")), 0.5f, 0.5f, 0.5f);
		glUniform3f(glGetUniformLocation(coreProgram, ("spotlight.specular")), 0.9f, 0.9f, 0.9f);

		glUniform1f(glGetUniformLocation(coreProgram, ("spotlight.constant")), 1.0f);
		glUniform1f(glGetUniformLocation(coreProgram, ("spotlight.linear")), 0.09f);
		glUniform1f(glGetUniformLocation(coreProgram, ("spotlight.quadratic")), 0.032f);

		glUniform1f(glGetUniformLocation(coreProgram, ("spotlight.cutOff")), glm::cos(glm::radians(12.5f)));
		glUniform1f(glGetUniformLocation(coreProgram, ("spotlight.outerCutOff")), glm::cos(glm::radians(17.5f)));

		glUniform1i(glGetUniformLocation(coreProgram, ("spotlight.on")), spotlightOn);

		glm::mat4 model(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -6.0f));
		model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));

		glUseProgram(coreProgram);
		glUniformMatrix4fv(glGetUniformLocation(coreProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(coreProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(coreProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniform1i(glGetUniformLocation(coreProgram, "instanced"), 0);

		planetObj.Draw(coreProgram);

		rockDiff.changeUnit(0);
		glUniform1i(glGetUniformLocation(coreProgram, "material.diffuse"), 0);
		rockSpec.changeUnit(1);
		glUniform1i(glGetUniformLocation(coreProgram, "material.specular"), 1);

		glUniform1i(glGetUniformLocation(coreProgram, "instanced"), 1);

		for (unsigned int i = 0; i < rockObj.meshes.size(); i++)
		{
			glBindVertexArray(rockObj.meshes[i].vao);
			glDrawElementsInstanced(
				GL_TRIANGLES, rockObj.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, count
			);
		}

		// skybox
		glCullFace(GL_FRONT);

		glUseProgram(skyProgram);
		glUniformMatrix4fv(glGetUniformLocation(skyProgram, "view"), 1, GL_FALSE, glm::value_ptr(skyView));
		glUniformMatrix4fv(glGetUniformLocation(skyProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glUniform1i(glGetUniformLocation(skyProgram, "skybox"), 2);

		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glFlush();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	glDeleteProgram(coreProgram);
}