#include "shadervar.h"

ShaderVar::ShaderVar() :
	m_Name(""),
	m_Type(GL_BOOL),
	m_ArraySize(1)
{
}

ShaderVar::ShaderVar(GLenum type, const char* name) :
	m_Name(name),
	m_Type(type),
	m_ArraySize(1)
{
	if (m_Name[m_Name.size() - 1] == ']')
		m_Name = m_Name.substr(0, m_Name.find_last_of('['));
}

ShaderVar::ShaderVar(GLenum type, const char* name, int size) :
	m_Name(name),
	m_Type(type),
	m_ArraySize(size)
{
	if (m_Name[m_Name.size() - 1] == ']')
		m_Name = m_Name.substr(0, m_Name.find_last_of('['));
}

ShaderVar::~ShaderVar()
{
}

void ShaderVar::SetName(const char* name)
{
	m_Name = name;
	if (m_Name[m_Name.size() - 1] == ']')
		m_Name = m_Name.substr(0, m_Name.find_last_of('['));
}

std::string ShaderVar::GetName()
{
	return m_Name;
}

void ShaderVar::SetType(GLenum type)
{
	m_Type = type;
}

GLenum ShaderVar::GetType()
{
	return m_Type;
}

void ShaderVar::SetArraySize(int size)
{
	m_ArraySize = size;
}

int ShaderVar::ArraySize()
{
	return m_ArraySize;
}
