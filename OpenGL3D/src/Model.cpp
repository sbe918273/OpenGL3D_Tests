#include "pch.h"
#include "Model.h"

Model::Model(std::string&& path)
	: directory(path.substr(0, path.find_last_of('/')))
{
	loadModel(std::move(path));
}

void Model::Draw(GLuint shader)
{
	for (unsigned int i = 0; i < meshes.size(); i++)
		meshes[i].Draw(shader);
}

void Model::Draw(GLuint shader, Texture& diffMap, Texture& specMap)
{
	for (unsigned int i = 0; i < meshes.size(); i++)
		meshes[i].Draw(shader, diffMap, specMap);
}

void Model::loadModel(std::string&& path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
	{
		std::cerr << "[Error: loadModel] (Assimp)" << importer.GetErrorString() << std::endl;
		return;
	}
	
	processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
	// Process each of the node's meshes.
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}

	// Process each of the node's children.
	int x = 0;
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		
		processNode(node->mChildren[i], scene);
		float percent = x++ * 100.0f / 79.0f;

	}
}


Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
	// vertex data
	std::vector<Vertex> vertices;
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		bool hasTexture = mesh->mTextureCoords[0];
		glm::vec2 texCoords = hasTexture ? glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y)
			: glm::vec2(0.0f);

		vertices.emplace_back(
			glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z),
			glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z),
			texCoords
		);
	}

	// index data
	std::vector<unsigned int> indices;
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		const aiFace& face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.emplace_back(face.mIndices[j]);
	}

	// texture data
	std::vector<Texture> textures;
	int matIndex = mesh->mMaterialIndex;
	if (matIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[matIndex];
		emplaceMaterialTextures(material, aiTextureType_DIFFUSE, TextureType::DIFFUSE, textures);
		emplaceMaterialTextures(material, aiTextureType_SPECULAR, TextureType::SPECULAR, textures);
	}

	return Mesh(vertices, indices, textures);
}

void Model::emplaceMaterialTextures(aiMaterial* mat, aiTextureType type, TextureType myType, std::vector<Texture>& outTextures)
{
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString aTexName;
		mat->GetTexture(type, i, &aTexName);
		std::string texName = aTexName.C_Str();

		bool skip = false;
		for (const std::string& otherTexName : loadedTexNames)
		{
			if (texName == otherTexName)
			{
				loadedTexNames.emplace_back(texName);
				skip = true;
				break;
			}
		}

		if (!skip)
		{
			bool hasAlpha = texName.substr(texName.find_last_of(".")) == ".png";
			std::string path = directory + "/" + texName;
			outTextures.emplace_back(myType, 0, std::move(path), hasAlpha );
		}
	}
}
