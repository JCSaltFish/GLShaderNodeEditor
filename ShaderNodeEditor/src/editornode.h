#pragma once

#include <GL/glew.h>

#include "program.h"
#include "framebuffer.h"
#include "texture.h"

enum class EditorPinType
{
	FLOW,
	FLOAT, FLOAT2, FLOAT3, FLOAT4,
	INT, INT2, INT3, INT4,
	ARRAY, // TODO Support array
	BLOCK,
	TEXTURE,
	IMAGE
};

enum class EditorBlockPinType
{
	UNIFROM_BLOCK,
	BUFFER_BLOCK
};

enum class EditorNodeType
{
	NODE,
	EVENT,
	PROGRAM,
	FRAMEBUFFER,
	TEXTURE,
	IMAGE,
	BLOCK,
	PINGPONG,
	TIME,
	MOUSE_POS
};

enum class EditorEventNodeType
{
	INIT,
	FRAME
};

enum class EditorProgramDispatchType
{
	ARRAY,
	COMPUTE
};

enum class EditorPingPongNodeType
{
	BUFFER,
	IMAGE
};

struct EditorNode;
struct EditorPin;

struct EditorNode
{
	int id = -1;
	EditorNodeType type = EditorNodeType::NODE;
	std::vector<EditorPin*> pinsIn;
	std::vector<EditorPin*> pinsOut;
	ImVec2 nodePos;
};

struct EditorEventNode : public EditorNode
{
	EditorEventNodeType eventNodeType = EditorEventNodeType::INIT;
};

struct EditorProgramNode : public EditorNode
{
	Program* target = 0;
	Framebuffer* framebuffer = 0;
	int attachmentsPinsStartId = 1;
	EditorPin* flowIn{};
	EditorPin* flowOut{};

	EditorProgramDispatchType dispatchType = EditorProgramDispatchType::ARRAY;
	GLenum drawMode = GL_POINTS;
	int dispatchSize[3]{};
};

struct EditorTextureNode : public EditorNode
{
	Texture* target = 0;
};

struct EditorImageNode : public EditorNode
{
	int sizeX = 0;
	int sizeY = 0;
	GLuint texture = -1;

	~EditorImageNode()
	{
		if (texture != -1)
			glDeleteTextures(1, &texture);
	}
};

struct EditorBlockNode : public EditorNode
{
	int size = 0;
	GLuint ubo = -1;
	GLuint ssbo = -1;
	int ssboSize = 1;

	~EditorBlockNode()
	{
		if (ubo != -1)
			glDeleteBuffers(1, &ubo);
		if (ssbo != -1)
			glDeleteBuffers(1, &ssbo);
	}
};

struct EditorPingPongNode : public EditorNode
{
	EditorPingPongNodeType pingpongType = EditorPingPongNodeType::BUFFER;
	int size = 0;
};

struct EditorLink
{
	int id = -1;
	EditorPin* pPin1 = 0;
	EditorPin* pPin2 = 0;
};

struct EditorPin
{
	int id = -1;
	EditorPinType type = EditorPinType::FLOW;
	int size = 1;
	bool isOutput = false;
	std::string name = "";
	EditorNode* pNode = 0;
	std::vector<EditorLink*> connectedLinks;
};

struct EditorFloatPin : public EditorPin
{
	float value = 0.0f;
};

struct EditorFloat2Pin : public EditorPin
{
	float value[2]{};
};

struct EditorFloat3Pin : public EditorPin
{
	float value[3]{};
};

struct EditorFloat4Pin : public EditorPin
{
	float value[4]{};
};

struct EditorIntPin : public EditorPin
{
	int value = 0;
};

struct EditorInt2Pin : public EditorPin
{
	int value[2]{};
};

struct EditorInt3Pin : public EditorPin
{
	int value[3]{};
};

struct EditorInt4Pin : public EditorPin
{
	int value[4]{};
};

struct EditorBlockPin : public EditorPin
{
	EditorBlockPinType blockPinType = EditorBlockPinType::UNIFROM_BLOCK;
	int index = -1;
};

namespace EditorNodeUtil
{
	inline EditorPinType GLTypeToPinType(GLenum type)
	{
		switch (type)
		{
		case GL_BOOL:
		case GL_INT:
		case GL_UNSIGNED_INT:
			return EditorPinType::INT;
		case GL_BOOL_VEC2:
		case GL_INT_VEC2:
		case GL_UNSIGNED_INT_VEC2:
			return EditorPinType::INT2;
		case GL_BOOL_VEC3:
		case GL_INT_VEC3:
		case GL_UNSIGNED_INT_VEC3:
			return EditorPinType::INT3;
		case GL_BOOL_VEC4:
		case GL_INT_VEC4:
		case GL_UNSIGNED_INT_VEC4:
			return EditorPinType::INT4;

		case GL_FLOAT:
			return EditorPinType::FLOAT;
		case GL_FLOAT_VEC2:
			return EditorPinType::FLOAT2;
		case GL_FLOAT_VEC3:
			return EditorPinType::FLOAT3;
		case GL_FLOAT_VEC4:
			return EditorPinType::FLOAT4;

		case GL_SAMPLER_2D:
			return EditorPinType::TEXTURE;
		case GL_IMAGE_2D:
			return EditorPinType::IMAGE;

		default:
			return EditorPinType::FLOAT;
		}
	}

	inline int PinTypeSize(EditorPinType type)
	{
		switch (type)
		{
		case EditorPinType::FLOAT:
			return sizeof(float);
		case EditorPinType::FLOAT2:
			return sizeof(float) * 2;
		case EditorPinType::FLOAT3:
			return sizeof(float) * 3;
		case EditorPinType::FLOAT4:
			return sizeof(float) * 4;
		case EditorPinType::INT:
			return sizeof(int);
		case EditorPinType::INT2:
			return sizeof(int) * 2;
		case EditorPinType::INT3:
			return sizeof(int) * 3;
		case EditorPinType::INT4:
			return sizeof(int) * 4;
		default:
			return 0;
		}
	}

	inline int GLDrawModeToIndex(GLenum drawMode)
	{
		switch (drawMode)
		{
		case GL_POINTS:
			return 0;
		case GL_LINE_STRIP:
			return 1;
		case GL_LINE_LOOP:
			return 2;
		case GL_LINES:
			return 3;
		case GL_LINE_STRIP_ADJACENCY:
			return 4;
		case GL_LINES_ADJACENCY:
			return 5;
		case GL_TRIANGLE_STRIP:
			return 6;
		case GL_TRIANGLE_FAN:
			return 7;
		case GL_TRIANGLES:
			return 8;
		case GL_TRIANGLE_STRIP_ADJACENCY:
			return 9;
		default:
			return 0;
		}
	}

	inline GLenum IndexToGLDrawMode(int index)
	{
		switch (index)
		{
		case 0:
			return GL_POINTS;
		case 1:
			return GL_LINE_STRIP;
		case 2:
			return GL_LINE_LOOP;
		case 3:
			return GL_LINES;
		case 4:
			return GL_LINE_STRIP_ADJACENCY;
		case 5:
			return GL_LINES_ADJACENCY;
		case 6:
			return GL_TRIANGLE_STRIP;
		case 7:
			return GL_TRIANGLE_FAN;
		case 8:
			return GL_TRIANGLES;
		case 9:
			return GL_TRIANGLE_STRIP_ADJACENCY;
		default:
			return GL_POINTS;
		}
	}
};
