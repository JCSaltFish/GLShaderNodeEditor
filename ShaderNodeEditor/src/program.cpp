#include <algorithm>

#include "program.h"

#include "shaders.h"

Program::Program() :
	m_Name(""),
	m_Program(-1),
	m_NeedInit(true)
{
}

Program::Program(const char* name) :
	m_Name(name),
	m_Program(-1),
	m_NeedInit(true)
{
}

Program::~Program()
{
	Destroy();
}

void Program::SetName(const char* name)
{
	m_Name = name;
}

std::string Program::GetName()
{
	return m_Name;
}

GLuint Program::GetProgram()
{
	return m_Program;
}

std::vector<std::string> Program::GetShaderFiles()
{
	return m_ShaderFiles;
}

std::vector<GLenum> Program::GetShaderTypes()
{
	return m_ShaderTypes;
}

std::vector<Uniform> Program::GetUniforms()
{
	return m_Uniforms;
}

std::vector<UniformBlock> Program::GetUniformBlocks()
{
	return m_UniformBlocks;
}

std::vector<BufferBlock> Program::GetBufferBlocks()
{
	return m_BufferBlocks;
}

void Program::AddShader(const char* file, GLenum type)
{
	m_ShaderFiles.push_back(file);
	m_ShaderTypes.push_back(type);
	m_NeedInit = true;
}

void Program::RemoveShader(int ix)
{
	m_ShaderFiles.erase(m_ShaderFiles.begin() + ix);
	m_ShaderTypes.erase(m_ShaderTypes.begin() + ix);
	m_NeedInit = true;
}

void Program::SetShaderType(int ix, GLenum type)
{
	m_ShaderTypes[ix] = type;
	m_NeedInit = true;
}

void Program::Initialize()
{
	if (m_Program != -1)
		glDeleteProgram(m_Program);

	// 0. Link & Compile
	std::vector<GLuint> shaderList;
	for (int i = 0; i < m_ShaderFiles.size(); i++)
		shaderList.push_back(CreateShader(m_ShaderTypes[i], LoadShader(m_ShaderFiles[i])));
	m_Program = CreateProgram(shaderList);
	std::for_each(shaderList.begin(), shaderList.end(), glDeleteShader);

	int n = 0;
	// 1. Get Uniforms
	std::vector<Uniform>().swap(m_Uniforms);
	std::vector<Uniform> uniforms;
	glGetProgramInterfaceiv(m_Program, GL_UNIFORM, GL_ACTIVE_RESOURCES, &n);
	for (int i = 0; i < n; i++)
	{
		const GLenum props[] =
		{
			GL_TYPE,
			GL_NAME_LENGTH,
			GL_ARRAY_SIZE
		};
		std::vector<GLint> values(3);
		glGetProgramResourceiv(m_Program, GL_UNIFORM, i, 3, props, 3, NULL, &values[0]);
		char* name = new char[values[1]];
		glGetProgramResourceName(m_Program, GL_UNIFORM, i, values[1], NULL, name);
		int loc = glGetUniformLocation(m_Program, name);
		m_Uniforms.push_back(Uniform{ ShaderVar(values[0], name, values[2]), loc });
		uniforms.push_back(Uniform{ ShaderVar(values[0], name, values[2]), loc });
		delete[] name;
	}

	// 2. Get Uniform Blocks
	std::vector<UniformBlock>().swap(m_UniformBlocks);
	glGetProgramInterfaceiv(m_Program, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &n);
	for (int i = 0; i < n; i++)
	{
		const GLenum props[] =
		{
			GL_NAME_LENGTH,
			GL_NUM_ACTIVE_VARIABLES,
			GL_BUFFER_BINDING
		};
		std::vector<GLint> values(3);
		glGetProgramResourceiv(m_Program, GL_UNIFORM_BLOCK, i, 3, props, 3, NULL, &values[0]);
		char* name = new char[values[0]];
		glGetProgramResourceName(m_Program, GL_UNIFORM_BLOCK, i, values[0], NULL, name);
		m_UniformBlocks.push_back(UniformBlock(name, values[2]));
		m_UniformBlocks[i].SetBinding(glGetUniformBlockIndex(m_Program, name));
		const GLenum prop = GL_ACTIVE_VARIABLES;
		std::vector<GLint> varIDs(values[1]);
		glGetProgramResourceiv(m_Program, GL_UNIFORM_BLOCK, i, 1, &prop, values[1], NULL, &varIDs[0]);
		for (int j = 0; j < values[1]; j++)
		{
			m_UniformBlocks[i].AddUniform(uniforms[varIDs[j]]);
			m_Uniforms.erase(m_Uniforms.begin() + varIDs[j]);
		}
		delete[] name;
	}

	// 3. Get Shader Storage Blocks
	std::vector<ShaderVar> bufferVars;
	glGetProgramInterfaceiv(m_Program, GL_BUFFER_VARIABLE, GL_ACTIVE_RESOURCES, &n);
	for (int i = 0; i < n; i++)
	{
		const GLenum props[] =
		{
			GL_TYPE,
			GL_NAME_LENGTH,
			GL_ARRAY_SIZE
		};
		std::vector<GLint> values(3);
		glGetProgramResourceiv(m_Program, GL_BUFFER_VARIABLE, i, 3, props, 3, NULL, &values[0]);
		char* name = new char[values[1]];
		glGetProgramResourceName(m_Program, GL_BUFFER_VARIABLE, i, values[1], NULL, name);
		int loc = glGetUniformLocation(m_Program, name);
		bufferVars.push_back(ShaderVar(values[0], name, values[2]));
		delete[] name;
	}
	std::vector<BufferBlock>().swap(m_BufferBlocks);
	glGetProgramInterfaceiv(m_Program, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &n);
	for (int i = 0; i < n; i++)
	{
		const GLenum props[] =
		{
			GL_NAME_LENGTH,
			GL_NUM_ACTIVE_VARIABLES,
			GL_BUFFER_BINDING
		};
		std::vector<GLint> values(3);
		glGetProgramResourceiv(m_Program, GL_SHADER_STORAGE_BLOCK, i, 3, props, 3, NULL, &values[0]);
		char* name = new char[values[0]];
		glGetProgramResourceName(m_Program, GL_SHADER_STORAGE_BLOCK, i, values[0], NULL, name);
		m_BufferBlocks.push_back(BufferBlock(name, values[2]));
		m_BufferBlocks[i].SetBinding(glGetProgramResourceIndex(m_Program, GL_SHADER_STORAGE_BLOCK, name));
		const GLenum prop = GL_ACTIVE_VARIABLES;
		std::vector<GLint> varIDs(values[1]);
		glGetProgramResourceiv(m_Program, GL_SHADER_STORAGE_BLOCK, i, 1, &prop, values[1], NULL, &varIDs[0]);
		for (int j = 0; j < values[1]; j++)
			m_BufferBlocks[i].AddVar(bufferVars[varIDs[j]]);
		delete[] name;
	}

	m_NeedInit = false;
}

bool Program::NeedsInit()
{
	return m_NeedInit;
}

void Program::Destroy()
{
	if (m_Program != -1)
		glDeleteProgram(m_Program);
	m_Program = -1;
}
