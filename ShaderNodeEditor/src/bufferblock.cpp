#include "bufferblock.h"

BufferBlock::BufferBlock() :
	m_Name(""),
	m_Binding(-1),
	m_Size(0)
{
}

BufferBlock::BufferBlock(const char* name, int binding) :
	m_Name(name),
	m_Binding(binding),
	m_Size(0)
{
}

BufferBlock::~BufferBlock()
{
}

void BufferBlock::SetName(const char* name)
{
	m_Name = name;
}

std::string BufferBlock::GetName()
{
	return m_Name;
}

std::vector<ShaderVar> BufferBlock::GetVars()
{
	return m_Vars;
}

void BufferBlock::SetBinding(int binding)
{
	m_Binding = binding;
}

int BufferBlock::GetBinding()
{
	return m_Binding;
}

int BufferBlock::size()
{
	return m_Size;
}

void BufferBlock::AddVar(ShaderVar& var)
{
	std::string name = var.GetName();
	var.SetName(name.substr(name.find_first_of(".") + 1).c_str());
	m_Vars.push_back(var);

	switch (var.GetType())
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
