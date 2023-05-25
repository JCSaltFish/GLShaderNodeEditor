#pragma once

#include <string>
#include <vector>

#include <GL/glew.h>

class Framebuffer
{
private:
	std::string m_Name;
	GLuint m_Framebuffer;
	int m_NumAttachments;
	bool m_HasRenderBuffer;

	int m_Width;
	int m_Height;

	bool m_NeedsInit;

	std::vector<GLuint> m_Textures;
	GLuint m_Renderbuffer;

public:
	Framebuffer();
	Framebuffer(const char* name);
	~Framebuffer();
	
public:
	void SetName(const char* name);
	void SetFramebuffer(GLuint framebuffer);
	std::string GetName();
	GLuint GetFramebuffer();
	GLuint GetTexture(int index);
	void SetNumAttachments(int n);
	int NumAttachments();
	void SetRenderbuffer(bool b);
	bool HasRenderbuffer();
	void SetSize(int width, int height);
	void GetSize(int* width, int* height);

public:
	void Initialize();
	bool NeedsInit();
	void Destroy();
};
