#include "pch.h"

#include "Utility.h"

#include "Camera.h"
#include "Handler.h"
#include "Texture.h"
#include "Shader.h"
#include "Model.h"

#include "CubeData.h"

static GLuint loadCubeMap(std::string* faces, unsigned int textureUnit)
{
	GLuint texture;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

	int width, height, channelCount;
	for (unsigned int i = 0; i < 6; i++)
	{
		const std::string path = "res/textures/mountain_skybox/" + faces[i];

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


static GLuint createVertexArray()
{
	GLuint vao;
	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glBindVertexArray(0);
	return vao;
}

void TestCubeMap()
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

	GLuint coreProgram;
	Shader::loadProgram(coreProgram, "res/shaders/circle/VertexCore.glsl", "res/shaders/CubeMap/FragmentCore.glsl");

	GLuint skyProgram;
	Shader::loadProgram(skyProgram, "res/shaders/CubeMap/VertexCubeMap.glsl", "res/shaders/CubeMap/FragmentCubeMap.glsl");

	// cube
	GLuint cubeVBO;
	glGenBuffers(1, &cubeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CubeData::vertices), CubeData::vertices, GL_STATIC_DRAW);

	GLuint cubeVAO = createVertexArray();
	glBindVertexArray(cubeVAO);

	GLuint cubeEBO;
	glGenBuffers(1, &cubeEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(CubeData::indices), CubeData::indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

	// skybox
	std::string faces[] =
	{
		"right.jpg",
		"left.jpg",
		"top.jpg",
		"bottom.jpg",
		"front.jpg",
		"back.jpg"
	};

	unsigned int cubeMapTexture = loadCubeMap(faces, 2);

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

	//Model modelObj("C:/Users/binma/Downloads/backpack/backpack.obj");
	Model modelObj("C:/Users/binma/Downloads/Ocean_Liner_V2/Ocean_Liner_V2/21398_Ocean_Liner_V2.obj");

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		Handler::MaintainKeyboard(window);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 model(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.0f));
		glm::mat4 view = std::move(camera.GetViewMatrix());
		glm::mat4 skyView = glm::mat4(glm::mat3(view));
		glm::mat4 projection = glm::perspective(glm::radians(camera.fov), 800.0f / 600.0f, 0.1f, 100.0f);

		// rest of scene
		glCullFace(GL_BACK);

		glUseProgram(coreProgram);

		glUniform1i(glGetUniformLocation(coreProgram, "skybox"), 2);
		glUniform1i(glGetUniformLocation(coreProgram, "mode"), 1);
		glUniform3fv(glGetUniformLocation(coreProgram, "viewPos"), 1, glm::value_ptr(camera.pos));
		glUniformMatrix4fv(glGetUniformLocation(coreProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(coreProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(coreProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	
		modelObj.Draw(coreProgram);
		//glBindVertexArray(cubeVAO);
		//glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		
		// skybox
		glCullFace(GL_FRONT);

		glUseProgram(skyProgram);
		glUniformMatrix4fv(glGetUniformLocation(skyProgram, "view"), 1, GL_FALSE, glm::value_ptr(skyView));
		glUniformMatrix4fv(glGetUniformLocation(skyProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glUniform1i(glGetUniformLocation(skyProgram, "skybox"), 2);

		glBindVertexArray(cubeVAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glFlush();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	glDeleteProgram(coreProgram);
}