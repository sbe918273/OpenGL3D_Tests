#pragma once

#include <iostream>
#include <fstream>
#include <string>


namespace Shader
{
bool loadSource(const std::string & fileName, std::string & outSource);
bool loadShader(const std::string&& fileName, GLenum type, const std::string&& typeString, GLuint& outShader);
bool loadProgram(GLuint& outProgram, std::string&& VertexShaderPath, std::string&& FragShaderPath);
bool loadProgram(GLuint& outProgram, std::string&& VertexShaderPath, std::string&& GeoShaderPath, std::string&& FragShaderPath);
}