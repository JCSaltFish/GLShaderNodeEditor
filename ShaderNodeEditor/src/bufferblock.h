#pragma once

#include "shadervar.h"

class BufferBlock
{
private:
	std::string m_Name;
	std::vector<ShaderVar> m_Vars;
	int m_Binding;
	int m_Size;

public:
	BufferBlock();
	BufferBlock(const char* name, int binding);
	~BufferBlock();

public:
	void SetName(const char* name);
	std::string GetName();
	std::vector<ShaderVar> GetVars();
	void SetBinding(int binding);
	int GetBinding();
	int size();

public:
	void AddVar(ShaderVar& var);
};
