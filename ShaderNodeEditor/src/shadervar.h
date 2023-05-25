#pragma once

#include <string>
#include <vector>

#include <GL/glew.h>

class ShaderVar
{
protected:
	std::string m_Name;
	GLenum m_Type;
	int m_ArraySize;

public:
	ShaderVar();
	ShaderVar(GLenum type, const char* name);
	ShaderVar(GLenum type, const char* name, int size);
	~ShaderVar();

public:
	void SetName(const char* name);
	std::string GetName();
	void SetType(GLenum type);
	GLenum GetType();
	void SetArraySize(int size);
	int ArraySize();
};