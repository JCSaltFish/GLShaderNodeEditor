#pragma once

#include "shadervar.h"

struct Uniform
{
	ShaderVar var;
	int loc;
};

class UniformBlock
{
private:
	std::string m_Name;
	std::vector<Uniform> m_Uniforms;
	int m_Binding;
	int m_Size;

public:
	UniformBlock();
	UniformBlock(const char* name, int binding);
	~UniformBlock();

public:
	void SetName(const char* name);
	std::string GetName();
	std::vector<Uniform> GetUniforms();
	void SetBinding(int binding);
	int GetBinding();
	int size();

public:
	void AddUniform(Uniform& uniform);
};
