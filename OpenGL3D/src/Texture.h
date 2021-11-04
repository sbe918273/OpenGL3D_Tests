#pragma once
#include <string>
#include <iostream>
#include "stb_image.h"

enum TextureType
{
	DIFFUSE,
	SPECULAR,
	OTHER,
	FLOAT
};

class Texture
{
public:
	Texture();
	Texture(const Texture& other);
	Texture(Texture&& other) noexcept;
	Texture(TextureType type, unsigned int textureUnit, std::string path);
	Texture(TextureType type, unsigned int textureUnit, std::string path, bool hasAlpha);
	Texture(TextureType type, unsigned int textureUnit, std::string path, bool hasAlpha, bool linearise);

	GLuint id = 0;
	unsigned int textureUnit;
	TextureType type;
	std::string path;

	bool hasAlpha;
	bool linearise;

	Texture& operator=(const Texture& other);
	Texture& operator=(Texture&& other) noexcept;

	void changeUnit(unsigned int textureUnit);
	void loadTextureHDR();

private:
	GLuint loadTexture(bool hasAlpha, bool linearise);

};




