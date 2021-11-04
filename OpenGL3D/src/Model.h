#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

#include "Mesh.h"

class Model
{
public:
	Model(std::string&& path);
	void Draw(GLuint shader);
	void Draw(GLuint shader, Texture& diffMap, Texture& specMap);

	std::string directory;
	std::vector<Mesh> meshes;

private:
	
	std::vector<std::string> loadedTexNames;

	void loadModel(std::string&& path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	void emplaceMaterialTextures(aiMaterial* mat, aiTextureType type, TextureType myType, std::vector<Texture>& outTextures);
};