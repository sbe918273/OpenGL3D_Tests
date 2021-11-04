#include "pch.h"
#include "Mesh.h"

Vertex::Vertex(glm::vec3 pos, glm::vec3 normal, glm::vec2 texCoords)
	: pos(pos), normal(normal), texCoords(texCoords)
{}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
	: vertices(vertices), indices(indices), textures(textures)
{
	setupMesh();
}

void Mesh::setupMesh()
{
	// vertex buffer
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	//vertex array
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
	glEnableVertexAttribArray(2);

	// element buffer
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);
}


void Mesh::Draw(GLuint shader)
{

	glUseProgram(shader);

	if (!textures.size())
	{
		Texture whiteDiffMap(TextureType::DIFFUSE, 0, "res/textures/white.png", false);
		glUniform1i(glGetUniformLocation(shader, "material.diffuse"), 0);
		Texture whiteSpecMap(TextureType::SPECULAR, 1, "res/textures/white.png", false);
		glUniform1i(glGetUniformLocation(shader, "material.specular"), 1);
	}
	else 
	{
		for (unsigned int i = 0; i < textures.size(); i++)
		{
			std::string label("");
			switch (textures[i].type)
			{
			case TextureType::DIFFUSE:
				label = "material.diffuse";
				break;
			case TextureType::SPECULAR:
				label = "material.specular";
				break;
			}

			textures[i].changeUnit(i);
			glUniform1i(glGetUniformLocation(shader, label.c_str()), i);
		}
	}
	glUniform1f(glGetUniformLocation(shader, "material.shininess"), 32.0f);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Mesh::Draw(GLuint shader, Texture& diffMap, Texture& specMap)
{

	glUseProgram(shader);

	diffMap.changeUnit(0);
	glUniform1i(glGetUniformLocation(shader, "material.diffuse"), 0);
	diffMap.changeUnit(1);
	glUniform1i(glGetUniformLocation(shader, "material.specular"), 1);

	glUniform1f(glGetUniformLocation(shader, "material.shininess"), 32.0f);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}


