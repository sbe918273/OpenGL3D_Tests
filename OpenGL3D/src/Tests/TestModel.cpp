#include "pch.h"
#define STB_IMAGE_IMPLEMENTATION

#include "Utility.h"
#include "Camera.h"
#include "Handler.h"
#include "Texture.h"
#include "Model.h"
#include "Shader.h"

#include "CubeData.h"
#include "LightData.h"

void TestModel()

{
	GLFWwindow* window = Utility::setupGLFW();
	Utility::setupGLEW();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Initialise vertex buffer.
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CubeData::vertices), CubeData::vertices, GL_STATIC_DRAW);

	// Initialise shaders and load programs.

	const bool drawNormals = false;
	const bool explode = false;

	GLuint coreProgram;
	if (explode)
		Shader::loadProgram(coreProgram,
			"res/shaders/Model/VertexExplode.glsl",
			"res/shaders/Model/GeometryExplode.glsl",
			"res/shaders/FragmentCore.glsl");
	else
		Shader::loadProgram(coreProgram, "res/shaders/VertexCore.glsl", "res/shaders/FragmentCore.glsl");

	GLuint normalProgram;
	if (drawNormals)
	{
		Shader::loadProgram(normalProgram,
			"res/shaders/Model/VertexNormal.glsl",
			"res/shaders/Model/GeometryNormal.glsl",
			"res/shaders/Model/FragmentSolid.glsl");
	}

	GLuint lightProgram;
	Shader::loadProgram(lightProgram, "res/shaders/VertexCore.glsl", "res/shaders/FragmentLight.glsl");

	// Initialise vertex array.
	GLuint vao;
	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Set position and normals to locations 0 and 1, respectively.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// Initialise element buffer.
	GLuint ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(CubeData::indices), CubeData::indices, GL_STATIC_DRAW);

	// Initialise textures and materials.
	stbi_set_flip_vertically_on_load(true);

	glUseProgram(coreProgram);

	// Initialise window state.
	Camera camera(
		glm::vec3(0.0f, 0.0f, 3.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		-90.0f,
		0.0f
	);

	bool spotlightOn = true;

	Handler handler(window, camera, spotlightOn);

	// Initialise model.
	//Model modelObj("C:/Users/binma/Downloads/Ocean_Liner_V2/Ocean_Liner_V2/21398_Ocean_Liner_V2.obj");
	Model modelObj("C:/Users/binma/Downloads/uploads_files_158717_3d-model.obj/3d-model.obj");
	//Model modelObj("C:/Users/binma/Downloads/backpack/backpack.obj");
	//Model modelObj("C:/Users/binma/Downloads/bgvwoubymcqo-ThrowingCube/WAVEFRONT.obj");


	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		Handler::MaintainKeyboard(window);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindVertexArray(vao);

		glm::mat4 view = std::move(camera.GetViewMatrix());
		glm::mat4 projection = glm::perspective(glm::radians(camera.fov), 800.0f / 600.0f, 0.1f, 100.0f);

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

		glUseProgram(coreProgram);

		glUniform3fv(glGetUniformLocation(coreProgram, "viewPos"), 1, glm::value_ptr(camera.pos));

		glUniform3f(glGetUniformLocation(coreProgram, ("dirLight.direction")), -1.0f, -1.0f, -1.0f);
		glUniform3f(glGetUniformLocation(coreProgram, ("dirLight.ambient")), 0.06f, 0.02f, 0.2f);
		glUniform3f(glGetUniformLocation(coreProgram, ("dirLight.diffuse")), 0.15f, 0.05f, 0.5f);
		glUniform3f(glGetUniformLocation(coreProgram, ("dirLight.specular")), 0.21f, 0.07f, 0.7f);

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
		//model = glm::rotate(model, glm::radians((float)glfwGetTime() * 5.0f), glm::vec3(1.0f, 0.3f, 0.5f));

		glUseProgram(coreProgram);
		glUniformMatrix4fv(glGetUniformLocation(coreProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(coreProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(coreProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		if (explode)
			glUniform1f(glGetUniformLocation(coreProgram, "time"), glfwGetTime());

		modelObj.Draw(coreProgram);

		if (drawNormals)
		{
			glUseProgram(normalProgram);
			glUniformMatrix4fv(glGetUniformLocation(normalProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
			glUniformMatrix4fv(glGetUniformLocation(normalProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(normalProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

			modelObj.Draw(normalProgram);
		}

		glfwSwapBuffers(window);
		glFlush();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	glDeleteProgram(coreProgram);
}