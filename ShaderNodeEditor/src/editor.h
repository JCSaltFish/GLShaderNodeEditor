#pragma once

#include <chrono>

#include "imgui.h"
#include "imnodes.h"

#include "editornode.h"

class ShaderNodeEditor
{
private:
	ImFont* m_NormalIconFont;
	ImFont* m_BigIconFont;

	int m_RenderWidth;
	int m_RenderHeight;

	std::vector<Program*> m_Programs;
	std::vector<Framebuffer*> m_Framebuffers;
	std::vector<Texture*> m_Textures;

	std::vector<EditorNode*> m_Nodes;
	std::vector<EditorPin*> m_Pins;
	std::vector<EditorLink*> m_Links;
	int m_StartedLinkPinId;
	bool m_bLinkHanged;
	ImVec2 m_HangPos;

private:
	bool m_OnInit;
	bool m_IsPlaying;

	bool m_PingPongSwap;

	std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTime;

public:
	ShaderNodeEditor();
	~ShaderNodeEditor();

private:
	void AddProgram(Program* pProgram);
	void DeleteProgram(int ix);

	void AddFramebuffer(Framebuffer* pFramebuffer);
	void DeleteFramebuffer(int ix);

	void AddTexture(Texture* pTex);
	void DeleteTexture(int ix);

private:
	enum class SelectedItemType
	{
		NONE,
		PROGRAM,
		FRAMEBUFFER,
		TEXTURE,
		NODES,
		NODE,
		LINK,
		PROGRAM_NODE,
		BUFFER_NODE,
		IMAGE_NODE,
		PINGPONG_NODE
	};
	SelectedItemType m_SelectedItemType;
	int m_SelectedItemId;

private:
	void ConfigImGui();
	void PushImGuiStyles();
	void PopImGuiStyles();

	void DeleteSelectedItem();

private:
	void UpdateNodes();
	void UpdatePins();
	void UpdateLinks();
	void DeletePin(EditorPin* pin);
	void DeleteNodePinsAndLinks(int id);
	void DeleteNode(int id);
	void DeleteLink(int id, bool checkPingPongNodes = true);
	void CreateLink(int startPinId, int endPinId);

private:
	EditorPin* AllocPin(ShaderVar* var);

	EditorProgramNode* CreateProgramNodePtr(int progId, const ImVec2& pos);
	void CreateProgramNode(int progId, const ImVec2& pos);
	void UpdateProgramNode(int nodeId, int progId);
	void SetProgramNodeFramebuffer(EditorProgramNode* node, int framebufferId);

	void CreateBlockNode(const ImVec2& pos, int pinId = -1);
	void CreateTextureNode(int textureId, const ImVec2& pos);
	void CreateImageNode(const ImVec2& pos);

	void CreatePingPongNode(const ImVec2& pos,
		EditorPingPongNodeType type = EditorPingPongNodeType::BUFFER);
	void UpdatePingPongNode(int nodeId, EditorPingPongNodeType type);

	void CreateTimeNode(const ImVec2& pos);
	void CreateMousePosNode(const ImVec2& pos);

private:
	ImNodesPinShape BeginPin(EditorPin* pin, float alpha);
	void EndPin();
	void InputPin(EditorNode* node, EditorPin* pin);
	void OutputPin(EditorNode* node, EditorPin* pin);

private:
	EditorPin* GetConnectedPin(EditorNode* node, EditorLink* link);
	// The target block/image node may go through multiple program/ping-pong nodes
	// before it is linked to the input pin
	void GetInputTargetNode(EditorNode*& connectedNode, EditorPinType type, int index);

	void ExecuteProgramNode(EditorProgramNode* progNode);

public:
	void Initialize();
	void SetRenderSize(int width, int height);
	void Display();
	void DrawGui();
};
