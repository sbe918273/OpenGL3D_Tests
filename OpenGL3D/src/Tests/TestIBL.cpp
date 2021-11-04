#include "pch.h"

#include <math.h>
#include <iostream>
#include "Utility.h"

#include "Camera.h"
#include "Handler.h"
#include "Texture.h"
#include "Shader.h"
#include "CubeData.h"

static struct Vertex
{
	glm::vec3 pos;
	glm::vec2 texCoords;
	glm::vec3 tangent;
	glm::vec3 bitangent;
	glm::vec3 normal;
};


static struct Light
{
	glm::vec3 pos;
	glm::vec3 color;
};

static const float pi = 3.1415926f;
static const unsigned int uSample = 64;
static const unsigned int vSample = 64;

static void createCubeMap(unsigned int dim, bool mipmap, unsigned int texUnit, GLuint& outFBO, GLuint& outCubeMap)
{
	glGenFramebuffers(1, &outFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, outFBO);

	// create and configure cubemap texture
	glGenTextures(1, &outCubeMap);
	glActiveTexture(GL_TEXTURE0 + texUnit);
	glBindTexture(GL_TEXTURE_CUBE_MAP, outCubeMap);
	for (unsigned int i = 0; i < 6; i++)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
			dim, dim, 0, GL_RGB, GL_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (mipmap)
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static GLuint convoluteCubeMap(unsigned int dim, unsigned int mipMax, bool depth, unsigned int inTexUnit, unsigned int outTexUnit,
	GLuint cubeVAO, GLuint shader)
{
	GLuint cubeMap, fbo;
	createCubeMap(dim, mipMax >= 2, outTexUnit, fbo, cubeMap);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
	glCullFace(GL_FRONT);

	GLuint rbo;
	if (depth)
	{
		glGenRenderbuffers(1, &rbo);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	}

	// uniform matrices
	glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 views[] =
	{
	   glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
	   glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
	   glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	glUseProgram(shader);
	glUniform1i(glGetUniformLocation(shader, "envMap"), inTexUnit);
	glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	// renders each face to its texture color attachment.
	glBindVertexArray(cubeVAO);

	for (unsigned int mip = 0; mip < mipMax; mip++)
	{
		unsigned int mipDim = dim * std::pow(0.5, mip);
		glViewport(0, 0, mipDim, mipDim);

		if (depth)
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipDim, mipDim);

		float roughness = (float)mip / (float)(mipMax - 1);
		glUniform1f(glGetUniformLocation(shader, "roughness"), roughness);

		for (unsigned int i = 0; i < 6; ++i)
		{
			glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(views[i]));
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubeMap, mip);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glDrawElements(GL_TRIANGLES, CubeData::indexCount, GL_UNSIGNED_INT, 0);
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, 800, 600);
	glCullFace(GL_BACK);

	return cubeMap;
}

static GLuint createLUT(unsigned int texUnit, GLuint quadVAO, GLuint shader)
{
	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glCullFace(GL_BACK);

	unsigned int texture;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0 + texUnit);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

	glViewport(0, 0, 512, 512);
	glUseProgram(shader);
	
	glBindVertexArray(quadVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, 800, 600);
	return texture;
}

static void setPermSphereUniforms(unsigned int lightCount, GLuint shader)
{
	glUseProgram(shader);

	glUniform1i(glGetUniformLocation(shader, "material.albedoMap"), 0);
	glUniform1i(glGetUniformLocation(shader, "material.metallicMap"), 1);
	glUniform1i(glGetUniformLocation(shader, "material.normalMap"), 2);
	glUniform1i(glGetUniformLocation(shader, "material.roughnessMap"), 3);
	glUniform1f(glGetUniformLocation(shader, "material.ao"), 1.0);

	for (unsigned int i = 0; i < lightCount; i++)
	{
		glUniform3fv(glGetUniformLocation(shader, ("lights[" + std::to_string(i) + "].color").c_str()),
			1, glm::value_ptr(2.0f * LightData::pointLights[i].specular));
		//glUniform3fv(glGetUniformLocation(shader, ("lights[" + std::to_string(i) + "].color").c_str()),
			//1, glm::value_ptr(glm::vec3(2.0f)));
	}
}

static void setVarSphereUniforms(unsigned int lightCount, //Light* lights, 
	const glm::vec3& camPos, const glm::mat4& view, const glm::mat4& projection, GLuint shader)
{

	glUseProgram(shader);
	glUniform3fv(glGetUniformLocation(shader, "camPos"), 1, glm::value_ptr(camPos));

	glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	for (unsigned int i = 0; i < lightCount; i++)
		glUniform3fv(glGetUniformLocation(shader, ("lights[" + std::to_string(i) + "].pos").c_str()),
			1, glm::value_ptr(LightData::pointLights[i].position));
}

static void drawSpheres(unsigned int sphereCount, GLuint shader, GLuint vao)
{
	const unsigned int uIntCount = 6 * vSample * (uSample - 2);

	glUseProgram(shader);
	glBindVertexArray(vao);
	glDrawElementsInstanced(GL_TRIANGLES, uIntCount, GL_UNSIGNED_INT, 0, sphereCount);
}

static float getDirOffset(unsigned int pos, unsigned int dim)
{
	// [0, a - 1] => [-a / 2, a / 2] (except a = 1 => 0)
	if (dim == 1)
		return 0.0f;

	else
	{
		float dimf = (float)dim;
		float offset = (dimf / (dimf - 1)) * (float)pos - dimf / 2.0f;
		return offset;
	}
}

static void calcTangents(const Vertex* vertices, glm::vec3& outTangent, glm::vec3& outBitangent)
{
	const glm::vec3 edge1 = vertices[1].pos - vertices[0].pos;
	const glm::vec3 edge2 = vertices[2].pos - vertices[0].pos;
	const glm::vec2 deltaUV1 = vertices[1].texCoords - vertices[0].texCoords;
	const glm::vec2 deltaUV2 = vertices[2].texCoords - vertices[0].texCoords;

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

static void emplaceSpherePoints(unsigned int uSample, unsigned int vSample, Vertex* outVertices)
{
	const unsigned int vertexCount = vSample * (uSample - 2) + 2;

	outVertices[0].pos = glm::vec3(0.0f, 1.0f, 0.0f);
	outVertices[0].texCoords = glm::vec2(0.0f, 0.0f);
	outVertices[0].tangent = glm::vec3(0.0f, 0.0f, 1.0f);
	outVertices[0].bitangent = glm::vec3(-1.0f, 0.0, 0.0f);
	outVertices[0].normal = outVertices[0].pos;

	outVertices[vertexCount - 1].pos = glm::vec3(0.0f, -1.0f, 0.0f);
	outVertices[vertexCount - 1].texCoords = glm::vec2(0.0f, 1.0f);
	outVertices[vertexCount - 1].tangent = glm::vec3(0.0f, 0.0f, -1.0f);
	outVertices[vertexCount - 1].bitangent = glm::vec3(-1.0f, 0.0f, 0.0f);
	outVertices[vertexCount - 1].normal = outVertices[vertexCount - 1].pos;

	const float uInc = pi / (uSample - 1);
	const float vInc = 2 * pi / vSample;

	float u = uInc;
	float v = 0.0f;

	unsigned int idx = 1;

	for (unsigned int uIdx = 0; uIdx < uSample - 2; uIdx++)
	{
		for (unsigned int vIdx = 0; vIdx < vSample; vIdx++)
		{
			outVertices[idx].pos = glm::vec3(sin(u) * sin(v), cos(u), sin(u) * cos(v));
			outVertices[idx].texCoords = glm::vec2((float)uIdx / (float)uSample,
				fmodf((float)vIdx / (float)vSample + 0.5f, 1.0f));

			Vertex vertices[3] =
			{
				outVertices[idx],
				idx >= 1 ? outVertices[idx - 1] : outVertices[idx],
				idx >= vSample ? outVertices[idx - vSample] : outVertices[0]
			};

			calcTangents(vertices, outVertices[idx].tangent, outVertices[idx].bitangent);
			outVertices[idx].normal = outVertices[idx].pos;

			v += vInc;
			idx += 1;
		}
		u += uInc;
		v = 0.0f;
	}
}

static void emplaceSphereIndices(unsigned int uSample, unsigned int vSample, unsigned int* outIndices)
{
	unsigned int arrIdx = 0;

	for (unsigned int i = 1; i <= vSample; i++)
	{
		outIndices[arrIdx++] = 0;
		outIndices[arrIdx++] = i;
		outIndices[arrIdx++] = (i == vSample) ? 1 : (i + 1);
	}

	for (unsigned int uIdx = 0; uIdx < uSample - 3; uIdx++)
	{
		const unsigned int lowerStart = uIdx * vSample + 1;
		const unsigned int upperStart = lowerStart + vSample;

		for (unsigned int vIdx = 0; vIdx < vSample - 1; vIdx++)
		{
			outIndices[arrIdx++] = lowerStart + vIdx;
			outIndices[arrIdx++] = upperStart + vIdx;
			outIndices[arrIdx++] = lowerStart + vIdx + 1;

			outIndices[arrIdx++] = upperStart + vIdx;
			outIndices[arrIdx++] = upperStart + vIdx + 1;
			outIndices[arrIdx++] = lowerStart + vIdx + 1;
		}

		outIndices[arrIdx++] = upperStart - 1;
		outIndices[arrIdx++] = upperStart + vSample - 1;
		outIndices[arrIdx++] = lowerStart;

		outIndices[arrIdx++] = upperStart + vSample - 1;
		outIndices[arrIdx++] = upperStart;
		outIndices[arrIdx++] = lowerStart;
	}

	const unsigned int bottomIdx = vSample * (uSample - 2) + 1;
	const unsigned int penStart = bottomIdx - vSample;

	for (unsigned int i = 0; i < vSample; i++)
	{
		outIndices[arrIdx++] = bottomIdx;
		outIndices[arrIdx++] = (i == vSample - 1) ? penStart : penStart + i + 1;
		outIndices[arrIdx++] = penStart + i;
	}
}


static GLuint createSphereVAO(unsigned int xDim, unsigned int yDim)
{
	// sphere vertices and indices

	const unsigned int vertexCount = vSample * (uSample - 2) + 2;
	const unsigned int uIntCount = 6 * vSample * (uSample - 2);

	Vertex* sphereVertices = new Vertex[vertexCount];
	emplaceSpherePoints(uSample, vSample, sphereVertices);

	unsigned int* sphereIndices = new unsigned int[uIntCount];
	emplaceSphereIndices(uSample, vSample, sphereIndices);

	GLuint sphereVBO;
	glGenBuffers(1, &sphereVBO);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(Vertex), sphereVertices, GL_STATIC_DRAW);

	GLuint sphereVAO;
	glCreateVertexArrays(1, &sphereVAO);
	glBindVertexArray(sphereVAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

	GLuint sphereEBO;
	glGenBuffers(1, &sphereEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, uIntCount * sizeof(unsigned int), sphereIndices, GL_STATIC_DRAW);

	// model matrices 
	const unsigned int sphereCount = xDim * yDim;
	glm::mat4* modelMatrices = new glm::mat4[sphereCount];

	const float spacing = 2.0f;
	unsigned int idx = 0;
	for (unsigned int i = 0; i < xDim; i++)
	{
		float xOffset = getDirOffset(i, xDim);
		for (unsigned int j = 0; j < yDim; j++)
		{
			float yOffset = getDirOffset(j, yDim);
			modelMatrices[idx] = glm::translate(glm::mat4(1.0f),
				spacing * glm::vec3(xOffset, yOffset, 0.0f) + glm::vec3(0.0f, 0.0f, -1.0f));
			//modelMatrices[idx] = glm::rotate(modelMatrices[idx], (float)rand() / (float)RAND_MAX, glm::vec3(0.2f, 0.5f, 0.8f));

			idx += 1;
		}
	}

	GLuint modelVBO;
	glGenBuffers(1, &modelVBO);
	glBindBuffer(GL_ARRAY_BUFFER, modelVBO);
	glBufferData(GL_ARRAY_BUFFER, sphereCount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

	glBindVertexArray(sphereVAO);

	const unsigned int startAttrib = 5;
	const unsigned int vecSize = sizeof(glm::vec4);
	for (unsigned int i = 0; i < 4; i++)
	{
		glEnableVertexAttribArray(i + startAttrib);
		glVertexAttribPointer(i + startAttrib, 4, GL_FLOAT, GL_FALSE, 4 * vecSize, (void*)(i * vecSize));
		glVertexAttribDivisor(i + startAttrib, 1);
	}

	return sphereVAO;
}

static GLuint createCubeVAO()
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

	GLuint ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(CubeData::indices), CubeData::indices, GL_STATIC_DRAW);

	return vao;
}

static GLuint createQuadVAO()
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


static void drawLights(GLuint lightShader, GLuint cubeVAO, glm::mat4& view, glm::mat4& projection)
{
	glUseProgram(lightShader);
	glBindVertexArray(cubeVAO);

	for (int i = 0; i < COUNT_POINT_LIGHT; i++)
	{
		const PointLight& pointLight = LightData::pointLights[i];

		glm::mat4 model = glm::translate(glm::mat4(1.0f), pointLight.position);
		model = glm::scale(model, glm::vec3(0.15f));
		glUniformMatrix4fv(glGetUniformLocation(lightShader, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(lightShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(lightShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glUniform3fv(glGetUniformLocation(lightShader, "lightColor"), 1, glm::value_ptr(pointLight.specular));
		//glUniform3f(glGetUniformLocation(lightShader, "lightColor"), 0.9f, 0.9f, 0.9f);

		glDrawElements(GL_TRIANGLES, CubeData::indexCount, GL_UNSIGNED_INT, 0);
	}

}

void TestIBL()
{
	GLFWwindow* window = Utility::setupGLFW();
	Utility::setupGLEW();
	Utility::setupImgui(window);

	bool show_demo_window = true;

	// openGL
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_BLEND);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// shaders
	GLuint coreProgram;
	Shader::loadProgram(coreProgram, "res/shaders/PBR/VertexCore.glsl", "res/shaders/IBL/FragmentCore.glsl");

	GLuint lightProgram;
	Shader::loadProgram(lightProgram, "res/shaders/PBR/VertexLight.glsl", "res/shaders/PBR/FragmentLight.glsl");

	GLuint equirecProgram;
	Shader::loadProgram(equirecProgram, "res/shaders/IBL/VertexEquirec.glsl", "res/shaders/IBL/FragmentEquirec.glsl");

	GLuint cubeMapProgram;
	Shader::loadProgram(cubeMapProgram, "res/shaders/IBL/VertexCubeMap.glsl", "res/shaders/IBL/FragmentCubeMap.glsl");

	GLuint diffProgram;
	Shader::loadProgram(diffProgram, "res/shaders/IBL/VertexCubeMap.glsl", "res/shaders/IBL/FragmentDiff.glsl");

	GLuint prefilterProgram;
	Shader::loadProgram(prefilterProgram, "res/shaders/IBL/VertexCubeMap.glsl", "res/shaders/IBL/FragmentPrefilter.glsl");

	GLuint bdrfProgram;
	Shader::loadProgram(bdrfProgram, "res/shaders/IBL/VertexQuad.glsl", "res/shaders/IBL/FragmentBDRF.glsl");

	// material textures
	//Texture rustAlbedo(TextureType::OTHER, 0, "res/textures/rusty_ball/rustediron2_basecolor.png", true, true);
	//Texture rustMetallic(TextureType::FLOAT, 1, "res/textures/rusty_ball/rustediron2_metallic.png", false, false);
	//Texture rustNormal(TextureType::OTHER, 2, "res/textures/rusty_ball/rustediron2_normal.png", false, false);
	//Texture rustRoughness(TextureType::FLOAT, 3, "res/textures/rusty_ball/rustediron2_roughness.png", false, false);
	
	Texture albedo(TextureType::OTHER, 0, "res/textures/titanium_ball/Titanium-Scuffed_basecolor.png", true, true);
	Texture metallic(TextureType::FLOAT, 1, "res/textures/titanium_ball/Titanium-Scuffed_metallic.png", false, false);
	Texture normal(TextureType::OTHER, 2, "res/textures/titanium_ball/Titanium-Scuffed_normal.png", false, false);
	Texture roughness(TextureType::FLOAT, 3, "res/textures/titanium_ball/Titanium-Scuffed_roughness.png", false, false);

	//Texture albedo(TextureType::OTHER, 0,    "res/textures/bamboo_ball/bamboo-wood-semigloss-albedo.png", false, true);
	//Texture metallic(TextureType::FLOAT, 1,  "res/textures/bamboo_ball/bamboo-wood-semigloss-metal.png", false, false);
	//Texture normal(TextureType::OTHER, 2,    "res/textures/bamboo_ball/bamboo-wood-semigloss-normal.png", false, false);
	//Texture roughness(TextureType::FLOAT, 3, "res/textures/bamboo_ball/bamboo-wood-semigloss-roughness.png", false, false);

	//Texture equirecMap(TextureType::OTHER, 4, "res/textures/Milkyway_small.hdr");
	Texture equirecMap(TextureType::OTHER, 4, "res/textures/bridge.hdr");
	equirecMap.loadTextureHDR();

	// VAOs
	const unsigned int xDim = 3;
	const unsigned int yDim = 3;
	const unsigned int sphereCount = xDim * yDim;
	GLuint sphereVAO = createSphereVAO(xDim, yDim);
	GLuint cubeVAO = createCubeVAO();
	GLuint quadVAO = createQuadVAO();

	// camera and handler
	Camera camera(
		glm::vec3(0.0f, 0.0f, 2.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		-90.0f,
		0.0f
	);

	bool spotlightOn = true;

	Handler handler(window, camera, spotlightOn);

	setPermSphereUniforms(COUNT_POINT_LIGHT, coreProgram);

	GLuint hdrMap = convoluteCubeMap(512, 1, false, 4, 5, cubeVAO, equirecProgram);
	glActiveTexture(GL_TEXTURE5);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);


	GLuint diffMap = convoluteCubeMap(32, 1, false, 5, 6, cubeVAO, diffProgram);
	GLuint prefilterMap = convoluteCubeMap(128, 5, false, 5, 7, cubeVAO, prefilterProgram);
	GLuint bdrfMap = createLUT(8, quadVAO, bdrfProgram);

	float mipLevel = 0.0f;

	while (!glfwWindowShouldClose(window))
	{

		glfwPollEvents();
		Handler::MaintainKeyboard(window);

		// imgui
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Choose mipmap level.");
		ImGui::SliderFloat("mipLevel", &mipLevel, 0.0f, 4.99f);
		ImGui::End();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_BACK);

		glUseProgram(coreProgram);
		glUniform1i(glGetUniformLocation(coreProgram, "irradianceMap"), 6);
		glUniform1i(glGetUniformLocation(coreProgram, "prefilterMap"), 7);
		glUniform1i(glGetUniformLocation(coreProgram, "bdrfMap"), 8);
		glUniform1i(glGetUniformLocation(coreProgram, "showNormal"), glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS);
		glUniform1i(glGetUniformLocation(coreProgram, "normalMapping"), !(glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS));

		glm::mat4 view = std::move(camera.GetViewMatrix());
		glm::mat4 projection = glm::perspective(glm::radians(camera.fov), 800.0f / 600.0f, 0.1f, 50.0f);

		setVarSphereUniforms(COUNT_POINT_LIGHT, camera.pos, view, projection, coreProgram);
		drawSpheres(sphereCount, coreProgram, sphereVAO);

		drawLights(lightProgram, cubeVAO, view, projection);

		//*
		glCullFace(GL_FRONT);
		glUseProgram(cubeMapProgram);
		glUniform1i(glGetUniformLocation(cubeMapProgram, "envMap"), 5);
		glUniformMatrix4fv(glGetUniformLocation(cubeMapProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(cubeMapProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(cubeVAO);
		glDrawElements(GL_TRIANGLES, CubeData::indexCount, GL_UNSIGNED_INT, 0);
		// */

		/*
		glCullFace(GL_BACK);
		glUseProgram(showQuadProgram);
		glUniform1i(glGetUniformLocation(showQuadProgram, "aTex"), 8);
		glBindVertexArray(quadVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		//*/

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glFlush();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	glDeleteProgram(coreProgram);
	glDeleteProgram(lightProgram);
}

