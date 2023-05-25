#pragma once

#include "uniform.h"
#include "bufferblock.h"

class Program
{
private:
	std::string m_Name;
	GLuint m_Program;
	std::vector<std::string> m_ShaderFiles;
	std::vector<GLenum> m_ShaderTypes;

	std::vector<Uniform> m_Uniforms;
	std::vector<UniformBlock> m_UniformBlocks;
	std::vector<BufferBlock> m_BufferBlocks;

	bool m_NeedInit;

public:
	Program();
	Program(const char* name);
	~Program();

public:
	void SetName(const char* name);
	std::string GetName();
	GLuint GetProgram();
	std::vector<std::string> GetShaderFiles();
	std::vector<GLenum> GetShaderTypes();
	
	std::vector<Uniform> GetUniforms();
	std::vector<UniformBlock> GetUniformBlocks();
	std::vector<BufferBlock> GetBufferBlocks();

public:
	void AddShader(const char* file, GLenum type);
	void RemoveShader(int ix);
	void SetShaderType(int ix, GLenum type);
	void Initialize();
	bool NeedsInit();
	void Destroy();
};
