#include "uniform.h"

UniformBlock::UniformBlock() :
	m_Name(""),
	m_Binding(-1),
	m_Size(0)
{
}

UniformBlock::UniformBlock(const char* name, int binding) :
	m_Name(name),
	m_Binding(binding),
	m_Size(0)
{
}

UniformBlock::~UniformBlock()
{
}

void UniformBlock::SetName(const char* name)
{
	m_Name = name;
}

std::string UniformBlock::GetName()
{
	return m_Name;
}

std::vector<Uniform> UniformBlock::GetUniforms()
{
	return m_Uniforms;
}

void UniformBlock::SetBinding(int binding)
{
	m_Binding = binding;
}

int UniformBlock::GetBinding()
{
	return m_Binding;
}

int UniformBlock::size()
{
	return m_Size;
}

void UniformBlock::AddUniform(Uniform& uniform)
{
	std::string name = uniform.var.GetName();
	uniform.var.SetName(name.substr(name.find_first_of(".") + 1).c_str());
	m_Uniforms.push_back(uniform);

	switch (uniform.var.GetType())
	{
	case GL_BOOL:
	case GL_INT:
	case GL_UNSIGNED_INT:
		m_Size += sizeof(int);
		break;
	case GL_BOOL_VEC2:
	case GL_INT_VEC2:
	case GL_UNSIGNED_INT_VEC2:
		m_Size += sizeof(int) * 2;
		break;
	case GL_BOOL_VEC3:
	case GL_INT_VEC3:
	case GL_UNSIGNED_INT_VEC3:
		m_Size += sizeof(int) * 3;
		break;
	case GL_BOOL_VEC4:
	case GL_INT_VEC4:
	case GL_UNSIGNED_INT_VEC4:
		m_Size += sizeof(int) * 4;
		break;

	case GL_FLOAT:
		m_Size += sizeof(float);
		break;
	case GL_FLOAT_VEC2:
		m_Size += sizeof(float) * 2;
		break;
	case GL_FLOAT_VEC3:
		m_Size += sizeof(float) * 3;
		break;
	case GL_FLOAT_VEC4:
		m_Size += sizeof(float) * 4;
		break;
	}
}
