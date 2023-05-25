#include "framebuffer.h"

Framebuffer::Framebuffer() :
	m_Name(""),
	m_Framebuffer(-1),
	m_NumAttachments(0),
	m_HasRenderBuffer(false),
	m_NeedsInit(true),
	m_Renderbuffer(-1),
	m_Width(800),
	m_Height(600)
{
}

Framebuffer::Framebuffer(const char* name) :
	m_Name(name),
	m_Framebuffer(-1),
	m_NumAttachments(0),
	m_HasRenderBuffer(false),
	m_NeedsInit(true),
	m_Renderbuffer(-1),
	m_Width(800),
	m_Height(600)
{
}

Framebuffer::~Framebuffer()
{
	Destroy();
}

void Framebuffer::SetName(const char* name)
{
	m_Name = name;
}

void Framebuffer::SetFramebuffer(GLuint framebuffer)
{
	Destroy();
	m_Framebuffer = framebuffer;
	m_NeedsInit = true;
}

std::string Framebuffer::GetName()
{
	return m_Name;
}

GLuint Framebuffer::GetFramebuffer()
{
	return m_Framebuffer;
}

GLuint Framebuffer::GetTexture(int index)
{
	if (index < m_Textures.size())
		return m_Textures[index];
	return -1;
}

void Framebuffer::SetNumAttachments(int n)
{
	m_NumAttachments = n;
	m_NeedsInit = true;
}

int Framebuffer::NumAttachments()
{
	return m_NumAttachments;
}

void Framebuffer::SetRenderbuffer(bool b)
{
	m_HasRenderBuffer = b;
	m_NeedsInit = true;
}

bool Framebuffer::HasRenderbuffer()
{
	return m_HasRenderBuffer;
}

void Framebuffer::SetSize(int width, int height)
{
	m_Width = width;
	m_Height = height;

	m_NeedsInit = true;
}

void Framebuffer::GetSize(int* width, int* height)
{
	*width = m_Width;
	*height = m_Height;
}

void Framebuffer::Initialize()
{
	if (m_Framebuffer == 0)
	{
		m_NeedsInit = false;
		return;
	}

	// Delete old framebuffer
	if (m_Framebuffer != -1)
		glDeleteFramebuffers(1, &m_Framebuffer);
	glDeleteTextures(m_Textures.size(), m_Textures.data());
	std::vector<GLuint>().swap(m_Textures);
	if (m_Renderbuffer != -1)
		glDeleteRenderbuffers(1, &m_Renderbuffer);

	// Create new framebuffer
	glGenFramebuffers(1, &m_Framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_Framebuffer);

	// Create attachments
	for (int i = 0; i < m_NumAttachments; i++)
	{
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, texture, 0);
		m_Textures.push_back(texture);
	}

	// Create renderbuffer
	if (m_HasRenderBuffer)
	{
		glGenRenderbuffers(1, &m_Renderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, m_Renderbuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_Width, m_Height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_Renderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_NeedsInit = false;
}

bool Framebuffer::NeedsInit()
{
	return m_NeedsInit;
}

void Framebuffer::Destroy()
{
	if (m_Framebuffer == 0)
		return;

	if (m_Framebuffer != -1)
		glDeleteFramebuffers(1, &m_Framebuffer);
	m_Framebuffer = -1;
	glDeleteTextures(m_Textures.size(), m_Textures.data());
	std::vector<GLuint>().swap(m_Textures);
	if (m_Renderbuffer != -1)
	{
		glDeleteRenderbuffers(1, &m_Renderbuffer);
		m_Renderbuffer = -1;
	}
}
