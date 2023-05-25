#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#include <GL/glew.h>

#include "Shadinclude.hpp"

#include "shaders.h"

std::string FindFile(const std::string& strFilename)
{
	std::ifstream testFile(strFilename.c_str());
	if (testFile.is_open())
		return strFilename;
	else
		return std::string();
}

std::string LoadShader(const std::string& strShaderFilename)
{
	/*std::string strFilename = FindFile(strShaderFilename);
	std::ifstream shaderFile(strFilename.c_str());
	std::stringstream shaderData;
	shaderData << shaderFile.rdbuf();
	shaderFile.close();

	return shaderData.str();*/
	return Shadinclude::load(strShaderFilename);
}

GLuint CreateShader(GLenum eShaderType, const std::string& strShader)
{
	GLuint shader = glCreateShader(eShaderType);
	const char* strData = strShader.c_str();
	glShaderSource(shader, 1, &strData, NULL);

	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	return shader;
}

GLuint CreateProgram(const std::vector<GLuint>& shaderList)
{
	GLuint program = glCreateProgram();

	for (size_t iLoop = 0; iLoop < shaderList.size(); iLoop++)
		glAttachShader(program, shaderList[iLoop]);

	glLinkProgram(program);

	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);

	for (size_t iLoop = 0; iLoop < shaderList.size(); iLoop++)
		glDetachShader(program, shaderList[iLoop]);

	return program;
}
