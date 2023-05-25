#pragma once

#include <string>

#include <GL/glew.h>

class Texture
{
private:
	std::string m_Name;
	std::string m_Path;
	GLuint m_Texture;
	int m_Width;
	int m_Height;

public:
	Texture();
	Texture(const char* path);
	~Texture();

public:
	void SetName(const char* name);
	std::string GetName();
	std::string GetPath();
	void GetSize(int* width, int* height);
	GLuint GetTexture();

public:
	void LoadFromFile(const char* path);
	void Destroy();
};
