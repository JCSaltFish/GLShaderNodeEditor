#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture() :
	m_Name(""),
	m_Path(""),
	m_Texture(-1),
	m_Width(0),
	m_Height(0)
{
}

Texture::Texture(const char* path) :
	m_Name(""),
	m_Path(path),
	m_Texture(-1),
	m_Width(0),
	m_Height(0)
{
	LoadFromFile(path);
	std::string name = path;
	name = name.substr(name.find_last_of('/') + 1);
	m_Name = name.substr(0, name.find_last_of('.'));
}

Texture::~Texture()
{
	Destroy();
}

void Texture::SetName(const char* name)
{
	m_Name = name;
}

std::string Texture::GetName()
{
	return m_Name;
}

std::string Texture::GetPath()
{
	return m_Path;
}

void Texture::GetSize(int* width, int* height)
{
	*width = m_Width;
	*height = m_Height;
}

GLuint Texture::GetTexture()
{
	return m_Texture;
}

void Texture::LoadFromFile(const char* path)
{
	int n;
	auto data = stbi_load(path, &m_Width, &m_Height, &n, 4);

	if (m_Texture == -1)
		glGenTextures(1, &m_Texture);
	glBindTexture(GL_TEXTURE_2D, m_Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(data);

	m_Path = path;
}

void Texture::Destroy()
{
	if (m_Texture != -1)
	{
		glDeleteTextures(1, &m_Texture);
		m_Texture = -1;
	}
	m_Width = 0;
	m_Height = 0;
	m_Path = "";
}
