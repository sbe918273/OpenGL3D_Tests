#pragma once
#include <string>
#include <vector>
#include <vec2.hpp>
#include <vec3.hpp>

#include "Texture.h"

struct Vertex
{
	Vertex(glm::vec3 pos, glm::vec3 normal, glm::vec2 texCoords);

	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 texCoords;
};

class Mesh
{
public:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	GLuint vao = 0;

	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
	void Draw(GLuint shader);
	void Draw(GLuint shader, Texture& diffMap, Texture& specMap);

private:
	
	GLuint vbo = 0;
	GLuint ebo = 0;
	
	void setupMesh();
};