#include "pch.h"
#include "Shader.h"

bool Shader::loadSource(const std::string& fileName, std::string& outSource)
{
	outSource = "";
	std::string temp = "";

	std::ifstream inFile;
	inFile.open(fileName);

	if (inFile.is_open())
	{
		while (std::getline(inFile, temp))
			outSource += temp + "\n";
		inFile.close();
		return true;
	}

	return false;
}

bool Shader::loadShader(const std::string&& fileName, GLenum type, const std::string&& typeString, GLuint& outShader)
{
	bool loadSuccess = true;
	std::string source = "";

	if (!loadSource(fileName, source))
	{
		std::cerr << "[Error: loadShader] Could not open " << typeString << " shader source file." << std::endl;
		loadSuccess = false;
	}

	// Create shader object and compile its source.
	outShader = glCreateShader(type);
	const GLchar* sourceString = source.c_str();
	glShaderSource(outShader, 1, &sourceString, nullptr);
	glCompileShader(outShader);

	// Check for shader compilation error.
	GLint success;
	glGetShaderiv(outShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char infoLog[512] = {};
		glGetShaderInfoLog(outShader, 512, nullptr, infoLog);
		std::cerr << "[Error: loadShader] Could not compile " << typeString << " shader." << std::endl;
		std::cerr << infoLog << std::endl;
		loadSuccess = false;
	}

	return loadSuccess;
}

bool Shader::loadProgram(GLuint& outProgram, std::string&& VertexShaderPath, std::string&& GeoShaderPath, std::string&& FragShaderPath)
{
	bool loadSuccess = true;
	GLuint vertexShader, geoShader, fragmentShader = 0;

	if (loadShader(std::move(VertexShaderPath), GL_VERTEX_SHADER, "vertex", vertexShader) &&
		loadShader(std::move(GeoShaderPath), GL_GEOMETRY_SHADER, "geometry", geoShader) &&
		loadShader(std::move(FragShaderPath), GL_FRAGMENT_SHADER, "fragment", fragmentShader))
	{
		// Create program and link shaders.
		outProgram = glCreateProgram();
		glAttachShader(outProgram, vertexShader);
		glAttachShader(outProgram, geoShader);
		glAttachShader(outProgram, fragmentShader);
		glLinkProgram(outProgram);

		// Check for linking errors.
		GLint success;
		glGetProgramiv(outProgram, GL_LINK_STATUS, &success);
		if (!success)
		{
			char infoLog[512] = {};
			glGetProgramInfoLog(outProgram, 512, nullptr, infoLog);
			std::cerr << "[Error: loadProgram] Could not link program." << std::endl;
			std::cerr << infoLog << std::endl;
			loadSuccess = false;
		}
	}
	else
		loadSuccess = false;

	glUseProgram(0);
	glDeleteShader(vertexShader);
	glDeleteShader(geoShader);
	glDeleteShader(fragmentShader);

	return loadSuccess;
}

bool Shader::loadProgram(GLuint& outProgram, std::string&& VertexShaderPath, std::string&& FragShaderPath)
{

	bool loadSuccess = true;
	GLuint vertexShader, fragmentShader = 0;

	if (loadShader(std::move(VertexShaderPath), GL_VERTEX_SHADER, "vertex", vertexShader) &&
		loadShader(std::move(FragShaderPath), GL_FRAGMENT_SHADER, "fragment", fragmentShader))
	{
		// Create program and link shaders.
		outProgram = glCreateProgram();
		glAttachShader(outProgram, vertexShader);
		glAttachShader(outProgram, fragmentShader);
		glLinkProgram(outProgram);

		// Check for linking errors.
		GLint success;
		glGetProgramiv(outProgram, GL_LINK_STATUS, &success);
		if (!success)
		{
			char infoLog[512] = {};
			glGetProgramInfoLog(outProgram, 512, nullptr, infoLog);
			std::cerr << "[Error: loadProgram] Could not link program." << std::endl;
			std::cerr << infoLog << std::endl;
			loadSuccess = false;
		}
	}
	else
		loadSuccess = false;

	glUseProgram(0);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return loadSuccess;
}
