#ifndef __shaders_h__
#define __shaders_h__

std::string LoadShader(const std::string& strShaderFilename);
GLuint CreateShader(GLenum eShaderType, const std::string& strShaderFile);
GLuint CreateProgram(const std::vector<GLuint>& shaderList);

#endif
