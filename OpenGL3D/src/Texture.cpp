#include "pch.h"
#include "Texture.h"

Texture::Texture()
	: textureUnit(0), type(TextureType::OTHER), path(), hasAlpha(false), linearise(false)
{}

Texture::Texture(TextureType type, unsigned int textureUnit, std::string path)
	: type(type), textureUnit(textureUnit), path(path), hasAlpha(false), linearise(true)
{}

Texture::Texture(TextureType type, unsigned int textureUnit, std::string path, bool hasAlpha)
	: type(type), textureUnit(textureUnit), path(path), hasAlpha(hasAlpha), linearise(false)
{
	id = loadTexture(hasAlpha, false);
}

Texture::Texture(TextureType type, unsigned int textureUnit, std::string path, bool hasAlpha, bool linearise)
	: type(type), textureUnit(textureUnit), path(path), hasAlpha(hasAlpha), linearise(linearise)
{
	id = loadTexture(hasAlpha, linearise);
}

void Texture::changeUnit(unsigned int newTextureUnit)
{
	textureUnit = newTextureUnit;
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_2D, id);
}

GLuint Texture::loadTexture(bool hasAlpha, bool linearise)
{
	GLuint texture;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, countChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &countChannels, 0);
	if (data)
	{
		GLuint loadMode, storeMode;
		if (hasAlpha && linearise)
		{
			loadMode = GL_RGBA;
			storeMode = GL_SRGB_ALPHA;
		}
		else if (hasAlpha && !linearise)
		{
			loadMode = GL_RGBA;
			storeMode = GL_RGBA;
		}
		else if (!hasAlpha && linearise)
		{
			loadMode = GL_RGB;
			storeMode = GL_SRGB;
		}
		else
		{
			loadMode = GL_RGB;
			storeMode = GL_RGB;
		}

		if (type == TextureType::FLOAT)
			loadMode = GL_RED;

		glTexImage2D(GL_TEXTURE_2D, 0, storeMode, width, height, 0, loadMode, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(data);
	}
	else
		std::cerr << "[Error: loadTexture] Could not open " << path << "." << std::endl;

	return texture;
}

void Texture::loadTextureHDR()
{
	int width, height, countChannels;
	float* data = stbi_loadf(path.c_str(), &width, &height, &countChannels, 0);
	if (data)
	{
		glGenTextures(1, &id);
		glActiveTexture(GL_TEXTURE0 + textureUnit);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
		std::cerr << "[Error: loadTextureHDR] Could not open " << path << "." << std::endl;
}

Texture::Texture(const Texture& other)
	: textureUnit(other.textureUnit), type(other.type), path(other.path), hasAlpha(other.hasAlpha), linearise(other.linearise)
{
	id = loadTexture(hasAlpha, linearise);
}

Texture& Texture::operator=(const Texture& other)
{
	textureUnit = other.textureUnit;
	type = other.type;
	path = other.path;
	hasAlpha = other.hasAlpha;
	linearise = other.linearise;
	// load texture again so original and copy do not clash
	id = loadTexture(hasAlpha, linearise);
	return *this;
}

Texture::Texture(Texture&& other) noexcept
	: id(other.id), textureUnit(other.textureUnit), type(other.type), path(std::move(other.path)), hasAlpha(other.hasAlpha), linearise(other.linearise)
{}

Texture& Texture::operator=(Texture&& other) noexcept
{
	id = other.id;
	textureUnit = other.textureUnit;
	type = other.type;
	path = std::move(other.path);
	hasAlpha = other.hasAlpha;
	linearise = other.linearise;
	return *this;
}

