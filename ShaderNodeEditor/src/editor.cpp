#include <sstream>

#include <fonts/sourcesanspro.h> // Text font: Source Sans Pro
#include <fonts/forkawesome.h> // Icon font: Fork Awesome
#include <fonts/IconsForkAwesome.h> // Icon font header: Fork Awesome

#include "editor.h"

#include "misc/cpp/imgui_stdlib.h"
#include "tinyfiledialogs.h" // Cross-platform file dialogs library

#include "pathutil.h"

ShaderNodeEditor::ShaderNodeEditor() :
    m_NormalIconFont(0),
    m_BigIconFont(0),
    m_RenderWidth(0),
    m_RenderHeight(0),
    m_SelectedItemType(SelectedItemType::NONE),
    m_SelectedItemId(-1),
    m_StartedLinkPinId(-1),
    m_bLinkHanged(false),
    m_OnInit(true),
    m_IsPlaying(false),
    m_PingPongSwap(false)
{
    m_StartTime = std::chrono::high_resolution_clock::now();
}

ShaderNodeEditor::~ShaderNodeEditor()
{
    ImNodes::DestroyContext();

    for (auto& link : m_Links)
        delete link;
    for (auto& pin : m_Pins)
        delete pin;
    for (auto& node : m_Nodes)
        delete node;

    for (auto& program : m_Programs)
    {
        program->Destroy();
        delete program;
    }
    for (auto& framebuffer : m_Framebuffers)
    {
        framebuffer->Destroy();
        delete framebuffer;
    }
    for (auto& texture : m_Textures)
    {
        texture->Destroy();
        delete texture;
    }
}

void ShaderNodeEditor::ConfigImGui()
{
    ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(SourceSansPro_compressed_data,
        SourceSansPro_compressed_size, 17);

    static const ImWchar icons_ranges[] = { ICON_MIN_FK, ICON_MAX_FK, 0 };
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;

    m_NormalIconFont = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(ForkAwesome_compressed_data,
        ForkAwesome_compressed_size, 14, &icons_config, icons_ranges);

    ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(SourceSansPro_compressed_data,
        SourceSansPro_compressed_size, 17);
    icons_config.GlyphOffset.y += (22 - 17) * 0.5f;
    m_BigIconFont = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(ForkAwesome_compressed_data,
        ForkAwesome_compressed_size, 22, &icons_config, icons_ranges);
}

void ShaderNodeEditor::PushImGuiStyles()
{
    ImGui::PushFont(m_NormalIconFont);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 2));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 6));
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 12);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3);
    ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 2);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.94f, 0.94f, 0.94f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(0.66f, 0.66f, 0.66f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.25f, 0.25f, 0.25f, 0.4f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.4f, 0.4f, 0.4f, 0.4f));
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0.06f, 0.06f, 0.06f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.1f, 0.4f, 0.9f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Tab, ImVec4(0.06f, 0.06f, 0.06f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TabHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TabActive, ImVec4(0.25f, 0.25f, 0.25f, 0.4f));
    ImGui::PushStyleColor(ImGuiCol_ResizeGrip, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ResizeGripActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_SeparatorActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_DragDropTarget, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
}

void ShaderNodeEditor::PopImGuiStyles()
{
    ImGui::PopFont();
    ImGui::PopStyleVar(7);
    ImGui::PopStyleColor(26);
}

void ShaderNodeEditor::DeleteSelectedItem()
{
    if (m_SelectedItemType >= SelectedItemType::NODES)
    {
        int numNodes = ImNodes::NumSelectedNodes();
        int* ids = new int[numNodes];
        ImNodes::GetSelectedNodes(ids);
        for (int i = 0; i < numNodes; i++)
        {
            if (m_Nodes[ids[i]]->type != EditorNodeType::EVENT)
                DeleteNode(ids[i]);
        }
        delete[] ids;

        int numLinks = ImNodes::NumSelectedLinks();
        ids = new int[numLinks];
        ImNodes::GetSelectedLinks(ids);
        for (int i = 0; i < numLinks; i++)
            DeleteLink(ids[i]);
        delete[] ids;

        if (numNodes > 0)
        {
            UpdateNodes();
            UpdatePins();
        }
        UpdateLinks();
    }

    else if (m_SelectedItemType == SelectedItemType::PROGRAM)
        DeleteProgram(m_SelectedItemId);

    else if (m_SelectedItemType == SelectedItemType::FRAMEBUFFER)
        DeleteFramebuffer(m_SelectedItemId);

    else if (m_SelectedItemType == SelectedItemType::TEXTURE)
        DeleteTexture(m_SelectedItemId);

    m_SelectedItemType = SelectedItemType::NONE;
    m_SelectedItemId = -1;
}

void ShaderNodeEditor::AddProgram(Program* pProgram)
{
	m_Programs.push_back(pProgram);
}

void ShaderNodeEditor::DeleteProgram(int ix)
{
    bool needsUpdate = false;
    for (auto& node : m_Nodes)
    {
        if (node->type == EditorNodeType::PROGRAM)
        {
            EditorProgramNode* progNode = (EditorProgramNode*)node;
            if (progNode->target == m_Programs[ix])
            {
                DeleteNode(node->id);
                needsUpdate = true;
            }
        }
    }
    if (needsUpdate)
    {
        UpdateNodes();
        UpdatePins();
        UpdateLinks();
    }

	m_Programs[ix]->Destroy();
    delete m_Programs[ix];
	m_Programs.erase(m_Programs.begin() + ix);
}

void ShaderNodeEditor::AddFramebuffer(Framebuffer* pFramebuffer)
{
    m_Framebuffers.push_back(pFramebuffer);
}

void ShaderNodeEditor::DeleteFramebuffer(int ix)
{
    if (ix == 0)
        return;

    bool needsUpdate = false;
    for (auto& node : m_Nodes)
    {
        if (node->type == EditorNodeType::PROGRAM)
        {
            EditorProgramNode* progNode = (EditorProgramNode*)node;
            if (progNode->framebuffer == m_Framebuffers[ix])
            {
                progNode->framebuffer = m_Framebuffers[0];
                SetProgramNodeFramebuffer(progNode, 0);
                needsUpdate = true;
            }
        }
    }
    if (needsUpdate)
    {
        UpdatePins();
        UpdateLinks();
    }

    m_Framebuffers[ix]->Destroy();
    delete m_Framebuffers[ix];
    m_Framebuffers.erase(m_Framebuffers.begin() + ix);
}

void ShaderNodeEditor::AddTexture(Texture* pTex)
{
    m_Textures.push_back(pTex);
}

void ShaderNodeEditor::DeleteTexture(int ix)
{
    bool needsUpdate = false;
    for (auto& node : m_Nodes)
    {
        if (node->type == EditorNodeType::TEXTURE)
        {
            EditorTextureNode* texNode = (EditorTextureNode*)node;
            if (texNode->target == m_Textures[ix])
            {
                DeleteNode(node->id);
                needsUpdate = true;
            }
        }
    }
    if (needsUpdate)
    {
        UpdateNodes();
        UpdatePins();
        UpdateLinks();
    }

    m_Textures[ix]->Destroy();
    delete m_Textures[ix];
    m_Textures.erase(m_Textures.begin() + ix);
}

void ShaderNodeEditor::UpdateNodes()
{
    for (int i = 0; i < m_Nodes.size(); i++)
    {
        if (!m_Nodes[i])
        {
            m_Nodes.erase(m_Nodes.begin() + i);
            i--;
        }
        else
        {
            m_Nodes[i]->id = i;
            ImNodes::SetNodeScreenSpacePos(m_Nodes[i]->id, m_Nodes[i]->nodePos);
        }
    }
}

void ShaderNodeEditor::UpdatePins()
{
    for (int i = 0; i < m_Pins.size(); i++)
    {
        if (!m_Pins[i])
        {
            m_Pins.erase(m_Pins.begin() + i);
            i--;
        }
        else
            m_Pins[i]->id = i;
    }
}

void ShaderNodeEditor::UpdateLinks()
{
    for (int i = 0; i < m_Links.size(); i++)
    {
        if (!m_Links[i])
        {
            m_Links.erase(m_Links.begin() + i);
            i--;
        }
        else
            m_Links[i]->id = i;
    }
}

void ShaderNodeEditor::DeletePin(EditorPin* pin)
{
    for (auto& link : pin->connectedLinks)
    {
        if (!link) continue;
        EditorPin* connectedPin = link->pPin1 == pin ? link->pPin2 : link->pPin1;
        auto& links = connectedPin->connectedLinks;
        links.erase(std::remove(links.begin(), links.end(), link), links.end());
        m_Links[link->id] = 0;
        delete link;
    }
    m_Pins[pin->id] = 0;
    delete pin;
}

void ShaderNodeEditor::DeleteNodePinsAndLinks(int id)
{
    for (auto& pin : m_Nodes[id]->pinsIn)
    {
        if (!pin) continue;
        DeletePin(pin);
    }
    for (auto& pin : m_Nodes[id]->pinsOut)
    {
        if (!pin) continue;
        DeletePin(pin);
    }
}

void ShaderNodeEditor::DeleteNode(int id)
{
    ImNodes::ClearNodeSelection();
    m_SelectedItemType = SelectedItemType::NONE;
    m_SelectedItemId = -1;
    DeleteNodePinsAndLinks(id);
    delete m_Nodes[id];
    m_Nodes[id] = 0;
}

void ShaderNodeEditor::DeleteLink(int id, bool checkPingPongNodes)
{
    ImNodes::ClearLinkSelection();
    m_SelectedItemType = SelectedItemType::NONE;
    m_SelectedItemId = -1;

    EditorLink* link = m_Links[id];
    if (!link) return;
    auto& links1 = link->pPin1->connectedLinks;
    links1.erase(std::remove(links1.begin(), links1.end(), link), links1.end());
    if (link->pPin1->pNode->type == EditorNodeType::PINGPONG &&
        link->pPin1->type == EditorPinType::BLOCK && checkPingPongNodes)
    {
        auto node = (EditorPingPongNode*)link->pPin1->pNode;
        bool isEmpty = true;
        for (auto& pin : node->pinsIn)
        {
            if (pin->connectedLinks.size() > 0)
            {
                isEmpty = false;
                break;
            }
        }
        for (auto& pin : node->pinsOut)
        {
            if (pin->connectedLinks.size() > 0)
            {
                isEmpty = false;
                break;
            }
        }
        if (isEmpty)
        {
            node->size = 0;
            for (auto& pin : node->pinsIn)
                pin->size = 0;
            for (auto& pin : node->pinsOut)
                pin->size = 0;
        }
    }

    auto& links2 = link->pPin2->connectedLinks;
    links2.erase(std::remove(links2.begin(), links2.end(), link), links2.end());
    if (link->pPin2->pNode->type == EditorNodeType::PINGPONG &&
        link->pPin2->type == EditorPinType::BLOCK && checkPingPongNodes)
    {
        auto node = (EditorPingPongNode*)link->pPin2->pNode;
        bool isEmpty = true;
        for (auto& pin : node->pinsIn)
        {
            if (pin->connectedLinks.size() > 0)
            {
                isEmpty = false;
                break;
            }
        }
        for (auto& pin : node->pinsOut)
        {
            if (pin->connectedLinks.size() > 0)
            {
                isEmpty = false;
                break;
            }
        }
        if (isEmpty)
        {
            node->size = 0;
            for (auto& pin : node->pinsIn)
                pin->size = 0;
            for (auto& pin : node->pinsOut)
                pin->size = 0;
        }
    }

    delete m_Links[id];
    m_Links[id] = 0;
}

void ShaderNodeEditor::CreateLink(int startPinId, int endPinId)
{
    bool canCreateLink = false;
    if (m_Pins[startPinId]->type == m_Pins[endPinId]->type)
    {
        if (m_Pins[startPinId]->type == EditorPinType::FLOW)
        {
            bool needsUpdate = false;
            if (m_Pins[startPinId]->connectedLinks.size() > 0)
            {
                DeleteLink(m_Pins[startPinId]->connectedLinks[0]->id);
                needsUpdate = true;
            }
            if (m_Pins[endPinId]->connectedLinks.size() > 0)
            {
                DeleteLink(m_Pins[endPinId]->connectedLinks[0]->id);
                needsUpdate = true;
            }
            if (needsUpdate)
                UpdateLinks();
            canCreateLink = true;
        }
        else if (m_Pins[startPinId]->type == EditorPinType::BLOCK)
        {
            if (m_Pins[startPinId]->size == m_Pins[endPinId]->size)
            {
                if (!m_Pins[startPinId]->isOutput)
                {
                    if (m_Pins[startPinId]->connectedLinks.size() > 0)
                    {
                        DeleteLink(m_Pins[startPinId]->connectedLinks[0]->id);
                        UpdateLinks();
                    }
                }
                if (!m_Pins[endPinId]->isOutput)
                {
                    if (m_Pins[endPinId]->connectedLinks.size() > 0)
                    {
                        DeleteLink(m_Pins[endPinId]->connectedLinks[0]->id);
                        UpdateLinks();
                    }
                }
                canCreateLink = true;
            }
            else
            {
                bool needsUpdate = false;
                if (!m_Pins[startPinId]->isOutput)
                {
                    if (m_Pins[startPinId]->connectedLinks.size() > 0)
                    {
                        DeleteLink(m_Pins[startPinId]->connectedLinks[0]->id);
                        needsUpdate = true;
                    }
                }
                if (!m_Pins[endPinId]->isOutput)
                {
                    if (m_Pins[endPinId]->connectedLinks.size() > 0)
                    {
                        DeleteLink(m_Pins[endPinId]->connectedLinks[0]->id);
                        needsUpdate = true;
                    }
                }
                if ((m_Pins[startPinId]->pNode->type == EditorNodeType::PINGPONG
                    && m_Pins[startPinId]->size == 0))
                {
                    auto pingpongNode = (EditorPingPongNode*)m_Pins[startPinId]->pNode;
                    pingpongNode->size = m_Pins[endPinId]->size;
                    auto pin = pingpongNode->pinsIn[0];
                    pin->size = m_Pins[endPinId]->size;
                    for (auto& link : pin->connectedLinks)
                    {
                        DeleteLink(link->id, false);
                        needsUpdate = true;
                    }
                    pin = pingpongNode->pinsIn[1];
                    pin->size = m_Pins[endPinId]->size;
                    for (auto& link : pin->connectedLinks)
                    {
                        DeleteLink(link->id, false);
                        needsUpdate = true;
                    }
                    pin = pingpongNode->pinsOut[0];
                    pin->size = m_Pins[endPinId]->size;
                    for (auto& link : pin->connectedLinks)
                    {
                        DeleteLink(link->id, false);
                        needsUpdate = true;
                    }
                    pin = pingpongNode->pinsOut[1];
                    pin->size = m_Pins[endPinId]->size;
                    for (auto& link : pin->connectedLinks)
                    {
                        DeleteLink(link->id, false);
                        needsUpdate = true;
                    }
                    canCreateLink = true;
                }
                else if (m_Pins[endPinId]->pNode->type == EditorNodeType::PINGPONG
                    && m_Pins[endPinId]->size == 0)
                {
                    auto pingpongNode = (EditorPingPongNode*)m_Pins[endPinId]->pNode;
                    pingpongNode->size = m_Pins[startPinId]->size;
                    auto pin = pingpongNode->pinsIn[0];
                    pin->size = m_Pins[startPinId]->size;
                    for (auto& link : pin->connectedLinks)
                    {
                        DeleteLink(link->id, false);
                        needsUpdate = true;
                    }
                    pin = pingpongNode->pinsIn[1];
                    pin->size = m_Pins[startPinId]->size;
                    for (auto& link : pin->connectedLinks)
                    {
                        DeleteLink(link->id, false);
                        needsUpdate = true;
                    }
                    pin = pingpongNode->pinsOut[0];
                    pin->size = m_Pins[startPinId]->size;
                    for (auto& link : pin->connectedLinks)
                    {
                        DeleteLink(link->id, false);
                        needsUpdate = true;
                    }
                    pin = pingpongNode->pinsOut[1];
                    pin->size = m_Pins[startPinId]->size;
                    for (auto& link : pin->connectedLinks)
                    {
                        DeleteLink(link->id, false);
                        needsUpdate = true;
                    }
                    canCreateLink = true;
                }
                if (needsUpdate)
                    UpdateLinks();
            }
        }
        else
        {
            if (!m_Pins[startPinId]->isOutput)
            {
                if (m_Pins[startPinId]->connectedLinks.size() > 0)
                {
                    DeleteLink(m_Pins[startPinId]->connectedLinks[0]->id);
                    UpdateLinks();
                }
            }
            if (!m_Pins[endPinId]->isOutput)
            {
                if (m_Pins[endPinId]->connectedLinks.size() > 0)
                {
                    DeleteLink(m_Pins[endPinId]->connectedLinks[0]->id);
                    UpdateLinks();
                }
            }
            canCreateLink = true;
        }
    }

    if (canCreateLink)
    {
        EditorLink* link = new EditorLink;
        link->id = m_Links.size();
        link->pPin1 = m_Pins[startPinId];
        link->pPin2 = m_Pins[endPinId];
        m_Pins[startPinId]->connectedLinks.push_back(link);
        m_Pins[endPinId]->connectedLinks.push_back(link);
        m_Links.push_back(link);
    }
}

EditorPin* ShaderNodeEditor::AllocPin(ShaderVar* var)
{
    auto type = EditorNodeUtil::GLTypeToPinType(var->GetType());
    EditorPin* pin;
    if (type == EditorPinType::FLOAT)
        pin = new EditorFloatPin;
    else if (type == EditorPinType::FLOAT2)
        pin = new EditorFloat2Pin;
    else if (type == EditorPinType::FLOAT3)
        pin = new EditorFloat3Pin;
    else if (type == EditorPinType::FLOAT4)
        pin = new EditorFloat4Pin;
    else if (type == EditorPinType::INT)
        pin = new EditorIntPin;
    else if (type == EditorPinType::INT2)
        pin = new EditorInt2Pin;
    else if (type == EditorPinType::INT3)
        pin = new EditorInt3Pin;
    else if (type == EditorPinType::INT4)
        pin = new EditorInt4Pin;
    else
        pin = new EditorPin;
    return pin;
}

EditorProgramNode* ShaderNodeEditor::CreateProgramNodePtr(int progId, const ImVec2& pos)
{
    Program* pProgram = m_Programs[progId];

    EditorProgramNode* node = new EditorProgramNode;
    node->type = EditorNodeType::PROGRAM;
    node->nodePos = pos;
    node->target = pProgram;
    node->framebuffer = m_Framebuffers[0];

    // Flow in & out
    {
        EditorPin* pinIn = new EditorPin;
        pinIn->id = m_Pins.size();
        pinIn->pNode = node;
        pinIn->type = EditorPinType::FLOW;
        node->pinsIn.push_back(pinIn);
        node->flowIn = pinIn;
        m_Pins.push_back(pinIn);
        EditorPin* pinOut = new EditorPin;
        pinOut->id = m_Pins.size();
        pinOut->pNode = node;
        pinOut->type = EditorPinType::FLOW;
        pinOut->isOutput = true;
        node->pinsOut.push_back(pinOut);
        node->flowOut = pinOut;
        m_Pins.push_back(pinOut);
    }

    // Uniforms
    for (auto& uniform : pProgram->GetUniforms())
    {
        EditorPin* pin = AllocPin(&uniform.var);
        pin->id = m_Pins.size();
        pin->pNode = node;
        pin->name = uniform.var.GetName();
        pin->type = EditorNodeUtil::GLTypeToPinType(uniform.var.GetType());
        node->pinsIn.push_back(pin);
        m_Pins.push_back(pin);
        if (pin->type == EditorPinType::IMAGE)
        {
            EditorPin* pinOut = new EditorPin;
            pinOut->id = m_Pins.size();
            pinOut->pNode = node;
            pinOut->name = uniform.var.GetName();
            pinOut->type = EditorPinType::IMAGE;
            pinOut->isOutput = true;
            node->pinsOut.push_back(pinOut);
            node->attachmentsPinsStartId++;
            m_Pins.push_back(pinOut);
        }
    }
    // Uniform blocks
    int index = 0;
    for (auto& uniformBlock : pProgram->GetUniformBlocks())
    {
        EditorBlockPin* pin = new EditorBlockPin;
        pin->id = m_Pins.size();
        pin->pNode = node;
        pin->name = uniformBlock.GetName();
        pin->type = EditorPinType::BLOCK;
        pin->blockPinType = EditorBlockPinType::UNIFROM_BLOCK;
        pin->index = index++;
        pin->size = uniformBlock.size();
        node->pinsIn.push_back(pin);
        m_Pins.push_back(pin);
    }
    // Buffer blocks
    index = 0;
    for (auto& bufferBlock : pProgram->GetBufferBlocks())
    {
        EditorBlockPin* pin = new EditorBlockPin;
        pin->id = m_Pins.size();
        pin->pNode = node;
        pin->name = bufferBlock.GetName();
        pin->type = EditorPinType::BLOCK;
        pin->blockPinType = EditorBlockPinType::BUFFER_BLOCK;
        pin->index = index;
        pin->size = bufferBlock.size();
        node->pinsIn.push_back(pin);
        m_Pins.push_back(pin);

        EditorBlockPin* pinOut = new EditorBlockPin;
        pinOut->id = m_Pins.size();
        pinOut->pNode = node;
        pinOut->name = bufferBlock.GetName();
        pinOut->type = EditorPinType::BLOCK;
        pinOut->blockPinType = EditorBlockPinType::BUFFER_BLOCK;
        pinOut->index = index++;
        pinOut->size = bufferBlock.size();
        pinOut->isOutput = true;
        node->pinsOut.push_back(pinOut);
        node->attachmentsPinsStartId++;
        m_Pins.push_back(pinOut);
    }

    return node;
}

void ShaderNodeEditor::CreateProgramNode(int progId, const ImVec2& pos)
{
    auto node = CreateProgramNodePtr(progId, pos);
    node->id = m_Nodes.size();
    m_Nodes.push_back(node);

    ImNodes::SetNodeScreenSpacePos(node->id, pos);

    m_SelectedItemType = SelectedItemType::PROGRAM_NODE;
    m_SelectedItemId = node->id;
    ImNodes::ClearLinkSelection();
    ImNodes::ClearNodeSelection();
    ImNodes::SelectNode(node->id);
}

void ShaderNodeEditor::UpdateProgramNode(int nodeId, int progId)
{
    // Create new node
    ImVec2 pos = m_Nodes[nodeId]->nodePos;
    auto node = CreateProgramNodePtr(progId, pos);
    node->id = nodeId;

    // Restore attributes
    EditorProgramNode* nodeOld = (EditorProgramNode*)m_Nodes[nodeId];
    auto iter = std::find(m_Framebuffers.begin(), m_Framebuffers.end(), nodeOld->framebuffer);
    if (iter != m_Framebuffers.end())
    {
        int index = iter - m_Framebuffers.begin();
        SetProgramNodeFramebuffer(node, index);
    }
    node->dispatchType = nodeOld->dispatchType;
    node->drawMode = nodeOld->drawMode;
    node->dispatchSize[0] = nodeOld->dispatchSize[0];
    node->dispatchSize[1] = nodeOld->dispatchSize[1];
    node->dispatchSize[2] = nodeOld->dispatchSize[2];

    // Restore pins and links
    for (int i = 0; i < node->pinsIn.size(); i++)
    {
        if (i < nodeOld->pinsIn.size() &&
            nodeOld->pinsIn[i]->type == node->pinsIn[i]->type &&
            nodeOld->pinsIn[i]->size == node->pinsIn[i]->size)
        {
            if (node->pinsIn[i]->type == EditorPinType::FLOAT)
            {
                auto newPin = (EditorFloatPin*)node->pinsIn[i];
                auto oldPin = (EditorFloatPin*)nodeOld->pinsIn[i];
                newPin->value = oldPin->value;
            }
            else if (node->pinsIn[i]->type == EditorPinType::FLOAT2)
            {
                auto newPin = (EditorFloat2Pin*)node->pinsIn[i];
                auto oldPin = (EditorFloat2Pin*)nodeOld->pinsIn[i];
                newPin->value[0] = oldPin->value[0];
                newPin->value[1] = oldPin->value[1];
            }
            else if (node->pinsIn[i]->type == EditorPinType::FLOAT3)
            {
                auto newPin = (EditorFloat3Pin*)node->pinsIn[i];
                auto oldPin = (EditorFloat3Pin*)nodeOld->pinsIn[i];
                newPin->value[0] = oldPin->value[0];
                newPin->value[1] = oldPin->value[1];
                newPin->value[2] = oldPin->value[2];
            }
            else if (node->pinsIn[i]->type == EditorPinType::FLOAT4)
            {
                auto newPin = (EditorFloat4Pin*)node->pinsIn[i];
                auto oldPin = (EditorFloat4Pin*)nodeOld->pinsIn[i];
                newPin->value[0] = oldPin->value[0];
                newPin->value[1] = oldPin->value[1];
                newPin->value[2] = oldPin->value[2];
                newPin->value[3] = oldPin->value[3];
            }
            else if (node->pinsIn[i]->type == EditorPinType::INT)
            {
                auto newPin = (EditorIntPin*)node->pinsIn[i];
                auto oldPin = (EditorIntPin*)nodeOld->pinsIn[i];
                newPin->value = oldPin->value;
            }
            else if (node->pinsIn[i]->type == EditorPinType::INT2)
            {
                auto newPin = (EditorInt2Pin*)node->pinsIn[i];
                auto oldPin = (EditorInt2Pin*)nodeOld->pinsIn[i];
                newPin->value[0] = oldPin->value[0];
                newPin->value[1] = oldPin->value[1];
            }
            else if (node->pinsIn[i]->type == EditorPinType::INT3)
            {
                auto newPin = (EditorInt3Pin*)node->pinsIn[i];
                auto oldPin = (EditorInt3Pin*)nodeOld->pinsIn[i];
                newPin->value[0] = oldPin->value[0];
                newPin->value[1] = oldPin->value[1];
                newPin->value[2] = oldPin->value[2];
            }
            else if (node->pinsIn[i]->type == EditorPinType::INT4)
            {
                auto newPin = (EditorInt4Pin*)node->pinsIn[i];
                auto oldPin = (EditorInt4Pin*)nodeOld->pinsIn[i];
                newPin->value[0] = oldPin->value[0];
                newPin->value[1] = oldPin->value[1];
                newPin->value[2] = oldPin->value[2];
                newPin->value[3] = oldPin->value[3];
            }

            for (auto& link : nodeOld->pinsIn[i]->connectedLinks)
            {
                if (link->pPin1 == nodeOld->pinsIn[i])
                    link->pPin1 = node->pinsIn[i];
                if (link->pPin2 == nodeOld->pinsIn[i])
                    link->pPin2 = node->pinsIn[i];
                node->pinsIn[i]->connectedLinks.push_back(link);
            }
            std::vector<EditorLink*>().swap(nodeOld->pinsIn[i]->connectedLinks);
        }
    }
    for (int i = 0; i < node->pinsOut.size(); i++)
    {
        if (i < nodeOld->pinsOut.size() &&
            nodeOld->pinsOut[i]->type == node->pinsOut[i]->type &&
            nodeOld->pinsOut[i]->size == node->pinsOut[i]->size)
        {
            for (auto& link : nodeOld->pinsOut[i]->connectedLinks)
            {
                if (link->pPin1 == nodeOld->pinsOut[i])
                    link->pPin1 = node->pinsOut[i];
                if (link->pPin2 == nodeOld->pinsOut[i])
                    link->pPin2 = node->pinsOut[i];
                node->pinsOut[i]->connectedLinks.push_back(link);
            }
            std::vector<EditorLink*>().swap(nodeOld->pinsOut[i]->connectedLinks);
        }
    }

    // Replace the old with the new one
    DeleteNodePinsAndLinks(nodeId);
    delete m_Nodes[nodeId];
    m_Nodes[nodeId] = node;
}

void ShaderNodeEditor::SetProgramNodeFramebuffer(EditorProgramNode* node, int framebufferId)
{
    int outPinId = 0;
    auto pinsOut = node->pinsOut;
    for (auto& outPin : pinsOut)
    {
        if (outPin->type == EditorPinType::TEXTURE)
        {
            DeletePin(outPin);
            node->pinsOut.erase(node->pinsOut.begin() + outPinId);
            outPinId--;
        }
        outPinId++;
    }

    Framebuffer* framebuffer = m_Framebuffers[framebufferId];
    for (int i = 0; i < framebuffer->NumAttachments(); i++)
    {
        EditorPin* pin = new EditorPin;
        pin->name = "Attachment " + std::to_string(i);
        pin->id = m_Pins.size();
        pin->isOutput = true;
        pin->type = EditorPinType::TEXTURE;
        pin->pNode = node;
        node->pinsOut.push_back(pin);
        m_Pins.push_back(pin);
    }
    node->framebuffer = framebuffer;
}

void ShaderNodeEditor::CreateBlockNode(const ImVec2& pos, int pinId)
{
    EditorBlockNode* blockNode = new EditorBlockNode;
    blockNode->type = EditorNodeType::BLOCK;
    blockNode->nodePos = pos;

    int size = 0;
    if (pinId != -1)
    {
        EditorBlockPin* progPin = (EditorBlockPin*)m_Pins[pinId];
        EditorProgramNode* progNode = (EditorProgramNode*)progPin->pNode;
        if (progPin->blockPinType == EditorBlockPinType::UNIFROM_BLOCK)
        {
            UniformBlock block = progNode->target->GetUniformBlocks()[progPin->index];
            for (auto& uniform : block.GetUniforms())
            {
                EditorPin* pin = AllocPin(&uniform.var);
                pin->id = m_Pins.size();
                pin->pNode = blockNode;
                pin->name = uniform.var.GetName();
                pin->type = EditorNodeUtil::GLTypeToPinType(uniform.var.GetType());
                blockNode->pinsIn.push_back(pin);
                m_Pins.push_back(pin);
                size += EditorNodeUtil::PinTypeSize(pin->type);
            }
        }
        else
        {
            BufferBlock block = progNode->target->GetBufferBlocks()[progPin->index];
            for (auto& var : block.GetVars())
            {
                EditorPin* pin = AllocPin(&var);
                pin->id = m_Pins.size();
                pin->pNode = blockNode;
                pin->name = var.GetName();
                pin->type = EditorNodeUtil::GLTypeToPinType(var.GetType());
                blockNode->pinsIn.push_back(pin);
                m_Pins.push_back(pin);
                size += EditorNodeUtil::PinTypeSize(pin->type);
            }
        }
    }

    EditorPin* pin = new EditorBlockPin;
    pin->id = m_Pins.size();
    pin->pNode = blockNode;
    pin->name = "";
    pin->type = EditorPinType::BLOCK;
    pin->isOutput = true;
    pin->size = size;
    blockNode->pinsOut.push_back(pin);
    m_Pins.push_back(pin);
    if (pinId != -1)
    {
        if (m_Pins[pinId]->connectedLinks.size() > 0)
        {
            DeleteLink(m_Pins[pinId]->connectedLinks[0]->id);
            UpdateLinks();
        }
        CreateLink(m_Pins.size() - 1, pinId);
    }

    blockNode->id = m_Nodes.size();
    blockNode->size = size;

    glGenBuffers(1, &blockNode->ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, blockNode->ubo);
    glBufferData(GL_UNIFORM_BUFFER, blockNode->size, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glGenBuffers(1, &blockNode->ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, blockNode->ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, blockNode->size * blockNode->ssboSize, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    m_Nodes.push_back(blockNode);

    ImNodes::SetNodeScreenSpacePos(blockNode->id, pos);

    m_SelectedItemType = SelectedItemType::BUFFER_NODE;
    m_SelectedItemId = blockNode->id;
    ImNodes::ClearLinkSelection();
    ImNodes::ClearNodeSelection();
    ImNodes::SelectNode(blockNode->id);
}

void ShaderNodeEditor::CreateTextureNode(int textureId, const ImVec2& pos)
{
    EditorTextureNode* node = new EditorTextureNode;
    node->type = EditorNodeType::TEXTURE;
    node->nodePos = pos;
    node->target = m_Textures[textureId];

    EditorPin* pin = new EditorPin;
    pin->id = m_Pins.size();
    pin->pNode = node;
    pin->name = "";
    pin->type = EditorPinType::TEXTURE;
    pin->isOutput = true;
    node->pinsOut.push_back(pin);
    m_Pins.push_back(pin);

    node->id = m_Nodes.size();
    m_Nodes.push_back(node);

    ImNodes::SetNodeScreenSpacePos(node->id, pos);

    m_SelectedItemType = SelectedItemType::NODE;
    m_SelectedItemId = node->id;
    ImNodes::ClearLinkSelection();
    ImNodes::ClearNodeSelection();
    ImNodes::SelectNode(node->id);
}

void ShaderNodeEditor::CreateImageNode(const ImVec2& pos)
{
    EditorImageNode* node = new EditorImageNode;
    node->type = EditorNodeType::IMAGE;
    node->nodePos = pos;

    EditorPin* pinIn = new EditorPin;
    pinIn->name = "Texture";
    pinIn->pNode = node;
    pinIn->type = EditorPinType::TEXTURE;
    pinIn->id = m_Pins.size();
    node->pinsIn.push_back(pinIn);
    m_Pins.push_back(pinIn);

    EditorPin* pinOut1 = new EditorPin;
    pinOut1->name = "";
    pinOut1->pNode = node;
    pinOut1->type = EditorPinType::IMAGE;
    pinOut1->isOutput = true;
    pinOut1->id = m_Pins.size();
    node->pinsOut.push_back(pinOut1);
    m_Pins.push_back(pinOut1);

    EditorPin* pinOut2 = new EditorPin;
    pinOut2->name = "";
    pinOut2->pNode = node;
    pinOut2->type = EditorPinType::TEXTURE;
    pinOut2->isOutput = true;
    pinOut2->id = m_Pins.size();
    node->pinsOut.push_back(pinOut2);
    m_Pins.push_back(pinOut2);

    glGenTextures(1, &node->texture);
    glBindTexture(GL_TEXTURE_2D, node->texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    node->id = m_Nodes.size();
    m_Nodes.push_back(node);

    ImNodes::SetNodeScreenSpacePos(node->id, pos);

    m_SelectedItemType = SelectedItemType::IMAGE_NODE;
    m_SelectedItemId = node->id;
    ImNodes::ClearLinkSelection();
    ImNodes::ClearNodeSelection();
    ImNodes::SelectNode(node->id);
}

void ShaderNodeEditor::CreatePingPongNode(const ImVec2& pos, EditorPingPongNodeType type)
{
    EditorPingPongNode* node = new EditorPingPongNode;
    node->type = EditorNodeType::PINGPONG;
    node->nodePos = pos;
    node->pingpongType = type;

    EditorPin* pinIn1 = new EditorPin;
    pinIn1->id = m_Pins.size();
    pinIn1->pNode = node;
    pinIn1->name = "Buffer A";
    if (type == EditorPingPongNodeType::BUFFER)
    {
        pinIn1->type = EditorPinType::BLOCK;
        pinIn1->size = 0;
    }
    else
    {
        pinIn1->type = EditorPinType::IMAGE;
        pinIn1->size = 1;
    }
    node->pinsIn.push_back(pinIn1);
    m_Pins.push_back(pinIn1);

    EditorPin* pinIn2 = new EditorPin;
    pinIn2->id = m_Pins.size();
    pinIn2->pNode = node;
    pinIn2->name = "Buffer B";
    if (type == EditorPingPongNodeType::BUFFER)
    {
        pinIn2->type = EditorPinType::BLOCK;
        pinIn2->size = 0;
    }
    else
    {
        pinIn2->type = EditorPinType::IMAGE;
        pinIn2->size = 1;
    }
    node->pinsIn.push_back(pinIn2);
    m_Pins.push_back(pinIn2);

    EditorPin* pinOut1 = new EditorPin;
    pinOut1->id = m_Pins.size();
    pinOut1->pNode = node;
    pinOut1->name = "Out 1";
    if (type == EditorPingPongNodeType::BUFFER)
    {
        pinOut1->type = EditorPinType::BLOCK;
        pinOut1->size = 0;
    }
    else
    {
        pinOut1->type = EditorPinType::IMAGE;
        pinOut1->size = 1;
    }
    pinOut1->isOutput = true;
    node->pinsOut.push_back(pinOut1);
    m_Pins.push_back(pinOut1);

    EditorPin* pinOut2 = new EditorPin;
    pinOut2->id = m_Pins.size();
    pinOut2->pNode = node;
    pinOut2->name = "Out 2";
    if (type == EditorPingPongNodeType::BUFFER)
    {
        pinOut2->type = EditorPinType::BLOCK;
        pinOut2->size = 0;
    }
    else
    {
        pinOut2->type = EditorPinType::IMAGE;
        pinOut2->size = 1;
    }
    pinOut2->isOutput = true;
    node->pinsOut.push_back(pinOut2);
    m_Pins.push_back(pinOut2);

    if (type == EditorPingPongNodeType::BUFFER)
        node->size = 0;
    else
        node->size = 1;

    node->id = m_Nodes.size();
    m_Nodes.push_back(node);

    ImNodes::SetNodeScreenSpacePos(node->id, pos);

    m_SelectedItemType = SelectedItemType::PINGPONG_NODE;
    m_SelectedItemId = node->id;
    ImNodes::ClearLinkSelection();
    ImNodes::ClearNodeSelection();
    ImNodes::SelectNode(node->id);
}

void ShaderNodeEditor::UpdatePingPongNode(int nodeId, EditorPingPongNodeType type)
{
    EditorPingPongNode* node = (EditorPingPongNode*)m_Nodes[nodeId];

    for (auto& pin : node->pinsIn)
    {
        for (auto& link : pin->connectedLinks)
            DeleteLink(link->id);
        if (type == EditorPingPongNodeType::BUFFER)
        {
            pin->type = EditorPinType::BLOCK;
            pin->size = 0;
        }
        else
        {
            pin->type = EditorPinType::IMAGE;
            pin->size = 1;
        }
    }
    for (auto& pin : node->pinsOut)
    {
        for (auto& link : pin->connectedLinks)
            DeleteLink(link->id);
        if (type == EditorPingPongNodeType::BUFFER)
        {
            pin->type = EditorPinType::BLOCK;
            pin->size = 0;
        }
        else
        {
            pin->type = EditorPinType::IMAGE;
            pin->size = 1;
        }
    }
    UpdateLinks();

    node->pingpongType = type;
    if (type == EditorPingPongNodeType::BUFFER)
        node->size = 0;
    else
        node->size = 1;
}

void ShaderNodeEditor::CreateTimeNode(const ImVec2& pos)
{
    EditorNode* node = new EditorNode;
    node->type = EditorNodeType::TIME;
    node->nodePos = pos;

    EditorPin* pin = new EditorPin;
    pin->pNode = node;
    pin->type = EditorPinType::FLOAT;
    pin->isOutput = true;
    pin->id = m_Pins.size();
    node->pinsOut.push_back(pin);
    m_Pins.push_back(pin);

    node->id = m_Nodes.size();
    m_Nodes.push_back(node);

    ImNodes::SetNodeScreenSpacePos(node->id, pos);

    m_SelectedItemType = SelectedItemType::NODE;
    m_SelectedItemId = node->id;
    ImNodes::ClearLinkSelection();
    ImNodes::ClearNodeSelection();
    ImNodes::SelectNode(node->id);
}

void ShaderNodeEditor::CreateMousePosNode(const ImVec2& pos)
{
    EditorNode* node = new EditorNode;
    node->type = EditorNodeType::MOUSE_POS;
    node->nodePos = pos;

    EditorPin* pin = new EditorPin;
    pin->pNode = node;
    pin->type = EditorPinType::FLOAT2;
    pin->isOutput = true;
    pin->id = m_Pins.size();
    node->pinsOut.push_back(pin);
    m_Pins.push_back(pin);

    node->id = m_Nodes.size();
    m_Nodes.push_back(node);

    ImNodes::SetNodeScreenSpacePos(node->id, pos);

    m_SelectedItemType = SelectedItemType::NODE;
    m_SelectedItemId = node->id;
    ImNodes::ClearLinkSelection();
    ImNodes::ClearNodeSelection();
    ImNodes::SelectNode(node->id);
}

ImNodesPinShape ShaderNodeEditor::BeginPin(EditorPin* pin, float alpha)
{
    ImNodesPinShape pinShape = ImNodesPinShape_Triangle;
    if (pin->type == EditorPinType::FLOW)
    {
        if (pin->connectedLinks.size() > 0)
            pinShape = ImNodesPinShape_TriangleFilled;
        ImNodes::PushColorStyle(ImNodesCol_Pin, IM_COL32(225, 225, 225, alpha * 255));
        ImNodes::PushColorStyle(ImNodesCol_PinHovered, IM_COL32(255, 255, 255, alpha * 255));
    }
    else if (pin->type == EditorPinType::INT)
    {
        if (pin->connectedLinks.size() > 0)
            pinShape = ImNodesPinShape_CircleFilled;
        else
            pinShape = ImNodesPinShape_Circle;
        ImNodes::PushColorStyle(ImNodesCol_Pin, IM_COL32(33, 227, 175, alpha * 255));
        ImNodes::PushColorStyle(ImNodesCol_PinHovered, IM_COL32(135, 239, 195, alpha * 255));
    }
    else if (pin->type == EditorPinType::FLOAT)
    {
        if (pin->connectedLinks.size() > 0)
            pinShape = ImNodesPinShape_CircleFilled;
        else
            pinShape = ImNodesPinShape_Circle;
        ImNodes::PushColorStyle(ImNodesCol_Pin, IM_COL32(156, 253, 65, alpha * 255));
        ImNodes::PushColorStyle(ImNodesCol_PinHovered, IM_COL32(144, 225, 137, alpha * 255));
    }
    else if (pin->type == EditorPinType::BLOCK)
    {
        if (pin->connectedLinks.size() > 0)
            pinShape = ImNodesPinShape_CircleFilled;
        else
            pinShape = ImNodesPinShape_Circle;
        ImNodes::PushColorStyle(ImNodesCol_Pin, IM_COL32(6, 165, 239, alpha * 255));
        ImNodes::PushColorStyle(ImNodesCol_PinHovered, IM_COL32(137, 196, 247, alpha * 255));
    }
    else if (pin->type == EditorPinType::TEXTURE)
    {
        if (pin->connectedLinks.size() > 0)
            pinShape = ImNodesPinShape_CircleFilled;
        else
            pinShape = ImNodesPinShape_Circle;
        ImNodes::PushColorStyle(ImNodesCol_Pin, IM_COL32(148, 0, 0, alpha * 255));
        ImNodes::PushColorStyle(ImNodesCol_PinHovered, IM_COL32(183, 137, 137, alpha * 255));
    }
    else if (pin->type == EditorPinType::IMAGE)
    {
        if (pin->connectedLinks.size() > 0)
            pinShape = ImNodesPinShape_CircleFilled;
        else
            pinShape = ImNodesPinShape_Circle;
        ImNodes::PushColorStyle(ImNodesCol_Pin, IM_COL32(200, 130, 255, alpha * 255));
        ImNodes::PushColorStyle(ImNodesCol_PinHovered, IM_COL32(220, 170, 255, alpha * 255));
    }
    else
    {
        if (pin->connectedLinks.size() > 0)
            pinShape = ImNodesPinShape_CircleFilled;
        else
            pinShape = ImNodesPinShape_Circle;
        ImNodes::PushColorStyle(ImNodesCol_Pin, IM_COL32(252, 200, 35, alpha * 255));
        ImNodes::PushColorStyle(ImNodesCol_PinHovered, IM_COL32(255, 217, 140, alpha * 255));
    }

    return pinShape;
}

void ShaderNodeEditor::EndPin()
{
    ImNodes::PopColorStyle();
    ImNodes::PopColorStyle();
}

void ShaderNodeEditor::InputPin(EditorNode* node, EditorPin* pin)
{
    float alpha = 0.2f;
    if (m_StartedLinkPinId == -1)
        alpha = 1.0f;
    else
    {
        if (m_StartedLinkPinId == pin->id ||
            (m_Pins[m_StartedLinkPinId]->type == pin->type
                && m_Pins[m_StartedLinkPinId]->isOutput
                && m_Pins[m_StartedLinkPinId]->pNode != node))
        {
            if (m_Pins[m_StartedLinkPinId]->size == pin->size)
                alpha = 1.0f;
            else if ((m_Pins[m_StartedLinkPinId]->pNode->type == EditorNodeType::PINGPONG
                && m_Pins[m_StartedLinkPinId]->size == 0) ||
                (node->type == EditorNodeType::PINGPONG && pin->size == 0))
                alpha = 1.0f;
        }
    }
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, alpha));

    ImNodesPinShape pinShape = BeginPin(pin, alpha);

    ImNodes::BeginInputAttribute(pin->id, pinShape);
    ImGui::Dummy(ImVec2(11.0f, 1.0f));
    ImGui::SameLine();
    ImGui::Text(pin->name.c_str());
    if (pin->type == EditorPinType::FLOAT)
    {
        EditorFloatPin* p = (EditorFloatPin*)pin;
        if (pin->connectedLinks.size() == 0)
        {
            ImGui::SameLine();
            ImGui::SetNextItemWidth(50.0f);
            ImGui::InputFloat("", &p->value);
        }
    }
    if (pin->type == EditorPinType::INT)
    {
        EditorIntPin* p = (EditorIntPin*)pin;
        if (pin->connectedLinks.size() == 0)
        {
            ImGui::SameLine();
            ImGui::SetNextItemWidth(50.0f);
            ImGui::InputInt("", &p->value, 0);
        }
    }
    else if (pin->type == EditorPinType::FLOAT2)
    {
        EditorFloat2Pin* p = (EditorFloat2Pin*)pin;
        if (pin->connectedLinks.size() == 0)
        {
            ImGui::Dummy(ImVec2(11.0f, 1.0f));
            ImGui::SameLine();
            ImGui::SetNextItemWidth(100.0f);
            ImGui::InputFloat2("", p->value);
        }
    }
    else if (pin->type == EditorPinType::INT2)
    {
        EditorInt2Pin* p = (EditorInt2Pin*)pin;
        if (pin->connectedLinks.size() == 0)
        {
            ImGui::Dummy(ImVec2(11.0f, 1.0f));
            ImGui::SameLine();
            ImGui::SetNextItemWidth(100.0f);
            ImGui::InputInt2("", p->value);
        }
    }
    else if (pin->type == EditorPinType::FLOAT3)
    {
        EditorFloat3Pin* p = (EditorFloat3Pin*)pin;
        if (pin->connectedLinks.size() == 0)
        {
            ImGui::Dummy(ImVec2(11.0f, 1.0f));
            ImGui::SameLine();
            ImGui::SetNextItemWidth(150.0f);
            ImGui::InputFloat3("", p->value);
        }
    }
    else if (pin->type == EditorPinType::INT3)
    {
        EditorInt3Pin* p = (EditorInt3Pin*)pin;
        if (pin->connectedLinks.size() == 0)
        {
            ImGui::Dummy(ImVec2(11.0f, 1.0f));
            ImGui::SameLine();
            ImGui::SetNextItemWidth(150.0f);
            ImGui::InputInt3("", p->value);
        }
    }
    else if (pin->type == EditorPinType::FLOAT4)
    {
        EditorFloat4Pin* p = (EditorFloat4Pin*)pin;
        if (pin->connectedLinks.size() == 0)
        {
            ImGui::Dummy(ImVec2(11.0f, 1.0f));
            ImGui::SameLine();
            ImGui::SetNextItemWidth(200.0f);
            ImGui::InputFloat4("", p->value);
        }
    }
    else if (pin->type == EditorPinType::INT4)
    {
        EditorInt4Pin* p = (EditorInt4Pin*)pin;
        if (pin->connectedLinks.size() == 0)
        {
            ImGui::Dummy(ImVec2(11.0f, 1.0f));
            ImGui::SameLine();
            ImGui::SetNextItemWidth(200.0f);
            ImGui::InputInt4("", p->value);
        }
    }
    ImNodes::EndInputAttribute();

    EndPin();

    ImGui::PopStyleColor();
}

void ShaderNodeEditor::OutputPin(EditorNode* node, EditorPin* pin)
{
    float alpha = 0.2f;
    if (m_StartedLinkPinId == -1)
        alpha = 1.0f;
    else
    {
        if (m_StartedLinkPinId == pin->id ||
            (m_Pins[m_StartedLinkPinId]->type == pin->type
                && !m_Pins[m_StartedLinkPinId]->isOutput
                && m_Pins[m_StartedLinkPinId]->pNode != node))
        {
            if (m_Pins[m_StartedLinkPinId]->size == pin->size)
                alpha = 1.0f;
            else if ((m_Pins[m_StartedLinkPinId]->pNode->type == EditorNodeType::PINGPONG
                && m_Pins[m_StartedLinkPinId]->size == 0) ||
                (node->type == EditorNodeType::PINGPONG && pin->size == 0))
                alpha = 1.0f;
        }
    }
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, alpha));

    ImNodesPinShape pinShape = BeginPin(pin, alpha);

    ImNodes::BeginOutputAttribute(pin->id, pinShape);
    std::string name = " " + pin->name;
    ImGui::Text(name.c_str());
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(11.0f, 1.0f));
    ImNodes::EndOutputAttribute();

    EndPin();

    ImGui::PopStyleColor();
}

EditorPin* ShaderNodeEditor::GetConnectedPin(EditorNode* node, EditorLink* link)
{
    return (link->pPin1->pNode == node) ? link->pPin2 : link->pPin1;
}

void ShaderNodeEditor::GetInputTargetNode(EditorNode*& connectedNode, EditorPinType type, int index)
{
    EditorNodeType targetNodeType =
        type == EditorPinType::IMAGE ? EditorNodeType::IMAGE : EditorNodeType::BLOCK;
    while (connectedNode && connectedNode->type != targetNodeType)
    {
        if (connectedNode->type == EditorNodeType::PINGPONG)
        {
            auto pingpongNode = (EditorPingPongNode*)connectedNode;
            if (m_PingPongSwap)
            {
                if (pingpongNode->pinsIn[1]->connectedLinks.size() > 0)
                {
                    connectedNode = GetConnectedPin
                    (
                        pingpongNode,
                        pingpongNode->pinsIn[1]->connectedLinks[0]
                    )->pNode;
                }
                else
                {
                    connectedNode = 0;
                    break;
                }
            }
            else
            {
                if (pingpongNode->pinsIn[0]->connectedLinks.size() > 0)
                {
                    connectedNode = GetConnectedPin
                    (
                        pingpongNode,
                        pingpongNode->pinsIn[0]->connectedLinks[0]
                    )->pNode;
                }
                else
                {
                    connectedNode = 0;
                    break;
                }
            }
        }
        else if (connectedNode->type == EditorNodeType::PROGRAM)
        {
            auto progNode = (EditorProgramNode*)connectedNode;
            if (targetNodeType == EditorNodeType::IMAGE)
            {
                int pinIndex = 0;
                for (auto& pin : progNode->pinsIn)
                {
                    if (pin->type == EditorPinType::IMAGE)
                    {
                        if (pinIndex == index)
                        {
                            if (pin->connectedLinks.size() > 0)
                            {
                                connectedNode = GetConnectedPin
                                (
                                    progNode,
                                    pin->connectedLinks[0]
                                )->pNode;
                            }
                            else
                            {
                                connectedNode = 0;
                                break;
                            }
                        }
                        pinIndex++;
                    }
                }
            }
            else
            {
                int pinIndex = 0;
                for (auto& pin : progNode->pinsIn)
                {
                    if (pin->type == EditorPinType::BLOCK)
                    {
                        EditorBlockPin* blockPin = (EditorBlockPin*)pin;
                        if (blockPin->blockPinType ==  EditorBlockPinType::BUFFER_BLOCK)
                        {
                            if (pinIndex == index)
                            {
                                if (pin->connectedLinks.size() > 0)
                                {
                                    connectedNode = GetConnectedPin
                                    (
                                        progNode,
                                        pin->connectedLinks[0]
                                    )->pNode;
                                }
                                else
                                {
                                    connectedNode = 0;
                                    break;
                                }
                            }
                            pinIndex++;
                        }
                    }
                }
            }
        }
        else
        {
            connectedNode = 0;
            break;
        }
    }
}

void ShaderNodeEditor::ExecuteProgramNode(EditorProgramNode* progNode)
{
    // Setup program
    glUseProgram(progNode->target->GetProgram());
    if (progNode->dispatchType == EditorProgramDispatchType::ARRAY)
    {
        auto framebuffer = progNode->framebuffer->GetFramebuffer();
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        if (framebuffer == 0)
            glViewport(0, 0, m_RenderWidth, m_RenderHeight);
        else
        {
            int width, height;
            progNode->framebuffer->GetSize(&width, &height);
            glViewport(0, 0, width, height);

            int numAttachments = progNode->framebuffer->NumAttachments();
            GLenum* attachments = new GLenum[numAttachments];
            for (int i = 0; i < numAttachments; i++)
                attachments[i] = GL_COLOR_ATTACHMENT0 + i;
            glDrawBuffers(numAttachments, attachments);
            delete[] attachments;
        }
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    int textureCount = 0;
    int imageCount = 0;
    int uniformBlockIndex = 0;
    int storageBufferIndex = 0;
    // Handle inputs
    for (auto& pin : progNode->pinsIn)
    {
        if (pin->type == EditorPinType::FLOAT)
        {
            int loc = glGetUniformLocation(progNode->target->GetProgram(), pin->name.c_str());
            if (pin->connectedLinks.size() > 0)
            {
                EditorNode* connectedNode = GetConnectedPin(progNode, pin->connectedLinks[0])->pNode;
                if (connectedNode->type == EditorNodeType::TIME)
                {
                    auto currentTime = std::chrono::high_resolution_clock::now();
                    float value =
                        std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_StartTime).count();
                    value *= 0.001f;
                    glUniform1f(loc, value);
                }
            }
            else
            {
                EditorFloatPin* p = (EditorFloatPin*)pin;
                glUniform1f(loc, p->value);
            }
        }
        else if (pin->type == EditorPinType::FLOAT2)
        {
            int loc = glGetUniformLocation(progNode->target->GetProgram(), pin->name.c_str());
            if (pin->connectedLinks.size() > 0)
            {
                EditorNode* connectedNode = GetConnectedPin(progNode, pin->connectedLinks[0])->pNode;
                if (connectedNode->type == EditorNodeType::MOUSE_POS)
                {
                    auto mousePos = ImGui::GetMousePos();
                    glUniform2f(loc, mousePos.x / (float)m_RenderWidth, mousePos.y / (float)m_RenderHeight);
                }
            }
            else
            {
                EditorFloat2Pin* p = (EditorFloat2Pin*)pin;
                glUniform2f(loc, p->value[0], p->value[1]);
            }
        }
        else if (pin->type == EditorPinType::FLOAT3)
        {
            int loc = glGetUniformLocation(progNode->target->GetProgram(), pin->name.c_str());
            EditorFloat3Pin* p = (EditorFloat3Pin*)pin;
            glUniform3f(loc, p->value[0], p->value[1], p->value[2]);
        }
        else if (pin->type == EditorPinType::FLOAT4)
        {
            int loc = glGetUniformLocation(progNode->target->GetProgram(), pin->name.c_str());
            EditorFloat4Pin* p = (EditorFloat4Pin*)pin;
            glUniform4f(loc, p->value[0], p->value[1], p->value[2], p->value[3]);
        }
        else if (pin->type == EditorPinType::INT)
        {
            int loc = glGetUniformLocation(progNode->target->GetProgram(), pin->name.c_str());
            EditorIntPin* p = (EditorIntPin*)pin;
            glUniform1i(loc, p->value);
        }
        else if (pin->type == EditorPinType::INT2)
        {
            int loc = glGetUniformLocation(progNode->target->GetProgram(), pin->name.c_str());
            EditorInt2Pin* p = (EditorInt2Pin*)pin;
            glUniform2i(loc, p->value[0], p->value[1]);
        }
        else if (pin->type == EditorPinType::INT3)
        {
            int loc = glGetUniformLocation(progNode->target->GetProgram(), pin->name.c_str());
            EditorInt3Pin* p = (EditorInt3Pin*)pin;
            glUniform3i(loc, p->value[0], p->value[1], p->value[2]);
        }
        else if (pin->type == EditorPinType::INT4)
        {
            int loc = glGetUniformLocation(progNode->target->GetProgram(), pin->name.c_str());
            EditorInt4Pin* p = (EditorInt4Pin*)pin;
            glUniform4i(loc, p->value[0], p->value[1], p->value[2], p->value[3]);
        }

        else if (pin->type == EditorPinType::TEXTURE)
        {
            if (pin->connectedLinks.size() > 0)
            {
                GLuint texture = -1;

                auto connectedNode = GetConnectedPin(progNode, pin->connectedLinks[0])->pNode;
                if (connectedNode->type == EditorNodeType::PROGRAM)
                {
                    auto connectedProgNode = (EditorProgramNode*)connectedNode;
                    auto connectedPin = GetConnectedPin(progNode, pin->connectedLinks[0]);
                    int attachmentIndex = connectedProgNode->attachmentsPinsStartId;
                    for (int i = attachmentIndex; i < connectedProgNode->pinsOut.size(); i++)
                    {
                        if (connectedProgNode->pinsOut[i] == connectedPin)
                            break;
                        attachmentIndex++;
                    }

                    texture = connectedProgNode->framebuffer->GetTexture(attachmentIndex);
                }
                else if (connectedNode->type == EditorNodeType::TEXTURE)
                {
                    auto texNode = (EditorTextureNode*)connectedNode;

                    texture = texNode->target->GetTexture();
                }
                else if (connectedNode->type == EditorNodeType::IMAGE)
                {
                    auto imgNode = (EditorImageNode*)connectedNode;

                    if (imgNode->pinsIn[0]->connectedLinks.size() > 0)
                    {
                        auto texNode = (EditorTextureNode*)
                            GetConnectedPin(imgNode, imgNode->pinsIn[0]->connectedLinks[0])->pNode;
                        texture = texNode->target->GetTexture();
                    }
                    else
                        texture = imgNode->texture;
                }

                if (texture != -1)
                {
                    glActiveTexture(GL_TEXTURE0 + textureCount);
                    glBindTexture(GL_TEXTURE_2D, texture);

                    int loc = glGetUniformLocation(progNode->target->GetProgram(), pin->name.c_str());
                    glUniform1i(loc, textureCount);
                    textureCount++;
                }
            }
        }

        else if (pin->type == EditorPinType::IMAGE)
        {
            if (pin->connectedLinks.size() > 0)
            {
                EditorNode* connectedNode = GetConnectedPin(progNode, pin->connectedLinks[0])->pNode;
                GetInputTargetNode(connectedNode, EditorPinType::IMAGE, imageCount);

                if (connectedNode)
                {
                    EditorImageNode* imgNode = (EditorImageNode*)connectedNode;

                    if (imgNode->pinsIn[0]->connectedLinks.size() > 0)
                    {
                        EditorTextureNode* texNode = (EditorTextureNode*)
                            GetConnectedPin(imgNode, imgNode->pinsIn[0]->connectedLinks[0])->pNode;
                        //glBindTexture(GL_TEXTURE_2D, texNode->target->GetTexture());
                        glBindImageTexture(textureCount, texNode->target->GetTexture(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
                    }
                    else
                    {
                        //glBindTexture(GL_TEXTURE_2D, imgNode->texture);
                        glBindImageTexture(textureCount, imgNode->texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
                    }

                    int loc = glGetUniformLocation(progNode->target->GetProgram(), pin->name.c_str());
                    glUniform1i(loc, textureCount);
                    textureCount++;
                }
            }
            imageCount++;
        }

        else if (pin->type == EditorPinType::BLOCK)
        {
            EditorBlockPin* p = (EditorBlockPin*)pin;
            if (pin->connectedLinks.size() > 0)
            {
                EditorNode* connectedNode = GetConnectedPin(progNode, pin->connectedLinks[0])->pNode;
                GetInputTargetNode(connectedNode, EditorPinType::BLOCK, storageBufferIndex);

                if (connectedNode)
                {
                    EditorBlockNode* blockNode = (EditorBlockNode*)
                        GetConnectedPin(progNode, pin->connectedLinks[0])->pNode;
                    if (p->blockPinType == EditorBlockPinType::UNIFROM_BLOCK)
                    {
                        GLuint blockId =
                            glGetUniformBlockIndex(progNode->target->GetProgram(), pin->name.c_str());
                        GLuint binding = progNode->target->GetUniformBlocks()[uniformBlockIndex].GetBinding();
                        glUniformBlockBinding(progNode->target->GetProgram(), blockId, binding);
                        glBindBufferBase(GL_UNIFORM_BUFFER, binding, blockNode->ubo);

                        GLubyte* blockData = (GLubyte*)glMapBufferRange
                        (
                            GL_UNIFORM_BUFFER, 0, blockNode->size,
                            GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT
                        );
                        int offset = 0;
                        for (int i = 0; i < blockNode->pinsIn.size(); i++)
                        {
                            if (blockNode->pinsIn[i]->type == EditorPinType::FLOAT)
                            {
                                if (blockNode->pinsIn[i]->connectedLinks.size() > 0)
                                {
                                    EditorNode* connectedNode =
                                        GetConnectedPin(progNode, pin->connectedLinks[0])->pNode;
                                    if (connectedNode->type == EditorNodeType::TIME)
                                    {
                                        auto currentTime = std::chrono::high_resolution_clock::now();
                                        float value =
                                            std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_StartTime).count();
                                        value *= 0.001f;
                                        memcpy(blockData + offset, &value, sizeof(float));
                                    }
                                }
                                else
                                {
                                    auto blockPin = (EditorFloatPin*)blockNode->pinsIn[i];
                                    memcpy(blockData + offset, &blockPin->value, sizeof(float));
                                }
                                offset += sizeof(float);
                            }
                            else if (blockNode->pinsIn[i]->type == EditorPinType::FLOAT2)
                            {
                                if (blockNode->pinsIn[i]->connectedLinks.size() > 0)
                                {
                                    EditorNode* connectedNode =
                                        GetConnectedPin(progNode, pin->connectedLinks[0])->pNode;
                                    if (connectedNode->type == EditorNodeType::MOUSE_POS)
                                    {
                                        auto mousePos = ImGui::GetMousePos();
                                        float x = mousePos.x / (float)m_RenderWidth;
                                        memcpy(blockData + offset, &x, sizeof(float));
                                        offset += sizeof(float);
                                        float y = mousePos.y / (float)m_RenderHeight;
                                        memcpy(blockData + offset, &y, sizeof(float));
                                        offset += sizeof(float);
                                    }
                                }
                                else
                                {
                                    auto blockPin = (EditorFloat2Pin*)blockNode->pinsIn[i];
                                    memcpy(blockData + offset, &blockPin->value[0], sizeof(float));
                                    offset += sizeof(float);
                                    memcpy(blockData + offset, &blockPin->value[1], sizeof(float));
                                    offset += sizeof(float);
                                }
                            }
                            else if (blockNode->pinsIn[i]->type == EditorPinType::FLOAT3)
                            {
                                auto blockPin = (EditorFloat3Pin*)blockNode->pinsIn[i];
                                memcpy(blockData + offset, &blockPin->value[0], sizeof(float));
                                offset += sizeof(float);
                                memcpy(blockData + offset, &blockPin->value[1], sizeof(float));
                                offset += sizeof(float);
                                memcpy(blockData + offset, &blockPin->value[2], sizeof(float));
                                offset += sizeof(float);
                            }
                            else if (blockNode->pinsIn[i]->type == EditorPinType::FLOAT4)
                            {
                                auto blockPin = (EditorFloat4Pin*)blockNode->pinsIn[i];
                                memcpy(blockData + offset, &blockPin->value[0], sizeof(float));
                                offset += sizeof(float);
                                memcpy(blockData + offset, &blockPin->value[1], sizeof(float));
                                offset += sizeof(float);
                                memcpy(blockData + offset, &blockPin->value[2], sizeof(float));
                                offset += sizeof(float);
                                memcpy(blockData + offset, &blockPin->value[3], sizeof(float));
                                offset += sizeof(float);
                            }
                            else if (blockNode->pinsIn[i]->type == EditorPinType::INT)
                            {
                                auto blockPin = (EditorIntPin*)blockNode->pinsIn[i];
                                memcpy(blockData + offset, &blockPin->value, sizeof(int));
                                offset += sizeof(int);
                            }
                            else if (blockNode->pinsIn[i]->type == EditorPinType::INT2)
                            {
                                auto blockPin = (EditorInt2Pin*)blockNode->pinsIn[i];
                                memcpy(blockData + offset, &blockPin->value[0], sizeof(int));
                                offset += sizeof(int);
                                memcpy(blockData + offset, &blockPin->value[1], sizeof(int));
                                offset += sizeof(int);
                            }
                            else if (blockNode->pinsIn[i]->type == EditorPinType::INT3)
                            {
                                auto blockPin = (EditorInt3Pin*)blockNode->pinsIn[i];
                                memcpy(blockData + offset, &blockPin->value[0], sizeof(int));
                                offset += sizeof(int);
                                memcpy(blockData + offset, &blockPin->value[1], sizeof(int));
                                offset += sizeof(int);
                                memcpy(blockData + offset, &blockPin->value[2], sizeof(int));
                                offset += sizeof(int);
                            }
                            else if (blockNode->pinsIn[i]->type == EditorPinType::INT4)
                            {
                                auto blockPin = (EditorInt4Pin*)blockNode->pinsIn[i];
                                memcpy(blockData + offset, &blockPin->value[0], sizeof(int));
                                offset += sizeof(int);
                                memcpy(blockData + offset, &blockPin->value[1], sizeof(int));
                                offset += sizeof(int);
                                memcpy(blockData + offset, &blockPin->value[2], sizeof(int));
                                offset += sizeof(int);
                                memcpy(blockData + offset, &blockPin->value[3], sizeof(int));
                                offset += sizeof(int);
                            }
                        }

                        glUnmapBuffer(GL_UNIFORM_BUFFER);
                    }
                    else
                    {
                        glBindBuffer(GL_SHADER_STORAGE_BUFFER, blockNode->ssbo);
                        GLuint binding = progNode->target->GetBufferBlocks()[storageBufferIndex].GetBinding();
                        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, blockNode->ssbo);
                    }
                }
            }

            if (p->blockPinType == EditorBlockPinType::UNIFROM_BLOCK)
                uniformBlockIndex++;
            else
                storageBufferIndex++;
        }
    }

    // Dispatch
    if (progNode->dispatchType == EditorProgramDispatchType::ARRAY)
        glDrawArrays(progNode->drawMode, 0, progNode->dispatchSize[0]);
    else if (progNode->dispatchType == EditorProgramDispatchType::COMPUTE)
    {
        glDispatchCompute
        (
            progNode->dispatchSize[0],
            progNode->dispatchSize[1],
            progNode->dispatchSize[2]
        );
        glMemoryBarrier
        (
            GL_SHADER_IMAGE_ACCESS_BARRIER_BIT |
            GL_SHADER_STORAGE_BARRIER_BIT |
            GL_BUFFER_UPDATE_BARRIER_BIT
        );
    }
}

void ShaderNodeEditor::Initialize()
{
    ConfigImGui();

    ImNodes::CreateContext();
    ImNodes::GetIO().AltMouseButton = ImGuiMouseButton_Right;
    
    ImNodes::GetStyle().NodePadding = ImVec2(12.0f, 5.0f);
    ImNodes::GetStyle().PinOffset = -16.0f;
    ImNodes::GetStyle().PinCircleRadius = 5.0f;
    ImNodes::GetStyle().NodeCornerRounding = 6.0f;
    ImNodes::GetStyle().Colors[ImNodesCol_NodeBackground] = IM_COL32(24, 24, 24, 200);
    ImNodes::GetStyle().Colors[ImNodesCol_NodeBackgroundHovered] = IM_COL32(24, 24, 24, 200);
    ImNodes::GetStyle().Colors[ImNodesCol_NodeBackgroundSelected] = IM_COL32(24, 24, 24, 200);
    ImNodes::GetStyle().Colors[ImNodesCol_GridBackground] = IM_COL32(38, 38, 38, 255);
    ImNodes::GetStyle().Colors[ImNodesCol_GridLine] = IM_COL32(53, 53, 53, 255);

    for (auto& link : m_Links)
        delete link;
    for (auto& pin : m_Pins)
        delete pin;
    for (auto& node : m_Nodes)
        delete node;

    for (auto& program : m_Programs)
    {
        program->Destroy();
        delete program;
    }

	std::vector<Program*>().swap(m_Programs);
    std::vector<Framebuffer*>().swap(m_Framebuffers);
    std::vector<Texture*>().swap(m_Textures);

    Framebuffer* screenFramebuffer = new Framebuffer("Screen");
    screenFramebuffer->SetFramebuffer(0);
    m_Framebuffers.push_back(screenFramebuffer);

    std::vector<EditorNode*>().swap(m_Nodes);
    std::vector<EditorPin*>().swap(m_Pins);
    std::vector<EditorLink*>().swap(m_Links);

    EditorEventNode* onInitNode = new EditorEventNode;
    onInitNode->id = 0;
    onInitNode->type = EditorNodeType::EVENT;
    onInitNode->eventNodeType = EditorEventNodeType::INIT;
    onInitNode->nodePos = ImVec2(100.0f, 100.0f);
    ImNodes::SetNodeScreenSpacePos(0, onInitNode->nodePos);
    EditorPin* initOut = new EditorPin;
    initOut->id = 0;
    initOut->isOutput = true;
    initOut->pNode = onInitNode;
    m_Pins.push_back(initOut);
    onInitNode->pinsOut.push_back(initOut);
    m_Nodes.push_back(onInitNode);

    EditorEventNode* onFrameNode = new EditorEventNode;
    onFrameNode->id = 1;
    onFrameNode->type = EditorNodeType::EVENT;
    onFrameNode->eventNodeType = EditorEventNodeType::FRAME;
    onFrameNode->nodePos = ImVec2(100.0f, 200.0f);
    ImNodes::SetNodeScreenSpacePos(1, onFrameNode->nodePos);
    EditorPin* frameOut = new EditorPin;
    frameOut->id = 1;
    frameOut->isOutput = true;
    frameOut->pNode = onFrameNode;
    m_Pins.push_back(frameOut);
    onFrameNode->pinsOut.push_back(frameOut);
    m_Nodes.push_back(onFrameNode);

    m_StartTime = std::chrono::high_resolution_clock::now();
}

void ShaderNodeEditor::SetRenderSize(int width, int height)
{
	m_RenderWidth = width;
	m_RenderHeight = height;
}

void ShaderNodeEditor::Display()
{
    // Update nodes
    bool needsUpdate = false;
    for (int i = 0; i < m_Programs.size(); i++)
    {
        if (m_Programs[i]->NeedsInit() || m_OnInit)
        {
            m_Programs[i]->Initialize();
            for (auto& node : m_Nodes)
            {
                if (node->type == EditorNodeType::PROGRAM)
                {
                    EditorProgramNode* progNode = (EditorProgramNode*)node;
                    if (progNode->target == m_Programs[i])
                    {
                        UpdateProgramNode(node->id, i);
                        needsUpdate = true;
                    }
                }
            }
        }
    }
    for (int i = 0; i < m_Framebuffers.size(); i++)
    {
        if (m_Framebuffers[i]->NeedsInit())
        {
            m_Framebuffers[i]->Initialize();
            for (auto& node : m_Nodes)
            {
                if (node->type == EditorNodeType::PROGRAM)
                {
                    EditorProgramNode* progNode = (EditorProgramNode*)node;
                    if (progNode->framebuffer == m_Framebuffers[i])
                    {
                        SetProgramNodeFramebuffer(progNode, i);
                        needsUpdate = true;
                    }
                }
            }
        }
    }
    if (needsUpdate)
    {
        UpdatePins();
        UpdateLinks();
    }

    if (m_OnInit)
    {
        for (auto texture : m_Textures)
            texture->LoadFromFile(texture->GetPath().c_str());
    }

    if (!m_IsPlaying)
    {
        m_OnInit = false;
        return;
    }

    // Execute on init
    if (m_OnInit)
    {
        m_StartTime = std::chrono::high_resolution_clock::now();

        EditorProgramNode* progNode = 0;
        EditorEventNode* onInitNode = (EditorEventNode*)m_Nodes[0];
        if (onInitNode->pinsOut[0]->connectedLinks.size() > 0)
        {
            progNode = (EditorProgramNode*)
                GetConnectedPin(onInitNode, onInitNode->pinsOut[0]->connectedLinks[0])->pNode;
        }
        while (progNode)
        {
            ExecuteProgramNode(progNode);

            if (progNode->flowOut->connectedLinks.size() > 0)
            {
                progNode = (EditorProgramNode*)
                    GetConnectedPin(progNode, progNode->flowOut->connectedLinks[0])->pNode;
            }
            else
                progNode = 0;
        }

        m_OnInit = false;
    }

    // Execute on frame
    EditorProgramNode* progNode = 0;
    EditorEventNode* onFrameNode = (EditorEventNode*)m_Nodes[1];
    if (onFrameNode->pinsOut[0]->connectedLinks.size() > 0)
    {
        progNode = (EditorProgramNode*)
            GetConnectedPin(onFrameNode, onFrameNode->pinsOut[0]->connectedLinks[0])->pNode;
    }
    while (progNode)
    {
        ExecuteProgramNode(progNode);

        if (progNode->flowOut->connectedLinks.size() > 0)
        {
            progNode = (EditorProgramNode*)
                GetConnectedPin(progNode, progNode->flowOut->connectedLinks[0])->pNode;
        }
        else
            progNode = 0;
    }

    m_PingPongSwap = !m_PingPongSwap;
}

void ShaderNodeEditor::DrawGui()
{
    PushImGuiStyles();

    ImGui::SetNextWindowSize(ImVec2(1280, 800), ImGuiCond_Once);
	ImGui::Begin("Node Editor");

    // Toolbar
    {
        ImGui::PushFont(m_BigIconFont);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 4));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 4));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.0f, 0.0f, 0.0f, 0.5f));

        ImGui::BeginChild("Toolbar", ImVec2(ImGui::GetContentRegionAvail().x, 40));

        ImGui::SetCursorPosY((ImGui::GetWindowHeight() - 30) * 0.5f);
        if (ImGui::Button(ICON_FK_FLOPPY_O "   Save", ImVec2(0, 30)))
        {
            // TODO Serializing the hole thing seems imposible...
        }

        // Editor Control
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

            ImGui::SameLine();
            ImGui::BeginChild("EditorControl", ImVec2(70, 30), true, ImGuiWindowFlags_NoScrollbar);
            ImGui::SetCursorPosY((ImGui::GetWindowHeight() - ImGui::GetTextLineHeight()) * 0.5f);

            if (m_IsPlaying)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
                if (ImGui::SmallButton(ICON_FK_STOP))
                    m_IsPlaying = false;
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.8f, 0.5f, 1.0f));
                if (ImGui::SmallButton(ICON_FK_PLAY))
                {
                    m_OnInit = true;
                    m_IsPlaying = true;
                }
            }
            ImGui::PopStyleColor();

            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
            if (ImGui::SmallButton(ICON_FK_UNDO))
                m_OnInit = true;
            ImGui::PopStyleColor();

            ImGui::EndChild();
            ImGui::PopStyleColor(3);
            ImGui::PopStyleVar(2);
        }

        ImGui::EndChild();

        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(4);

        ImGui::PopFont();
    }

    // Left Panel
    {
        ImGui::BeginChild("Left Panel", ImVec2(200, ImGui::GetContentRegionAvail().y));

        // Programs
        {
            // Add button
            float xPos = ImGui::GetCursorPosX();
            ImGui::SetCursorPosX(180);
            if (ImGui::SmallButton(ICON_FK_PLUS_CIRCLE "##add_program"))
            {
                ImNodes::ClearLinkSelection();
                ImNodes::ClearNodeSelection();

                std::string name = "Program " + std::to_string(m_Programs.size() + 1);
                Program* program = new Program(name.c_str());
                AddProgram(program);

                m_SelectedItemType = SelectedItemType::PROGRAM;
                m_SelectedItemId = m_Programs.size() - 1;
            }
            ImGui::SameLine();
            ImGui::SetCursorPosX(xPos);

            // Title bar
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 4));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
            bool isNodeOpened = ImGui::CollapsingHeader("PROGRAMS", ImGuiTreeNodeFlags_SpanAvailWidth);
            ImGui::SameLine();
            ImGui::SetCursorPosX(180);
            ImGui::Text(ICON_FK_PLUS_CIRCLE);
            ImGui::PopStyleVar(3);
            ImGui::PopStyleColor(3);

            if (isNodeOpened)
            {
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.25f, 0.25f, 0.25f, 0.4f));
                for (int i = 0; i < m_Programs.size(); i++)
                {
                    // Item
                    std::string name = "\t\t" + m_Programs[i]->GetName();
                    name += "##program" + std::to_string(i);
                    bool isSelected = m_SelectedItemType == SelectedItemType::PROGRAM;
                    isSelected = isSelected && (m_SelectedItemId == i);
                    if (ImGui::Selectable(name.c_str(), &isSelected))
                    {
                        ImNodes::ClearLinkSelection();
                        ImNodes::ClearNodeSelection();
                        if (isSelected)
                        {
                            m_SelectedItemType = SelectedItemType::PROGRAM;
                            m_SelectedItemId = i;
                        }
                        else
                        {
                            m_SelectedItemType = SelectedItemType::NONE;
                            m_SelectedItemId = -1;
                        }
                    }
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                    {
                        ImGui::SetDragDropPayload("program", &i, sizeof(int));
                        ImGui::Text(m_Programs[i]->GetName().c_str());
                        ImGui::EndDragDropSource();
                    }

                    // Context menu
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 6));
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(14, 4));
                    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.1f, 0.4f, 0.9f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
                    if (ImGui::BeginPopupContextItem())
                    {
                        ImNodes::ClearLinkSelection();
                        ImNodes::ClearNodeSelection();
                        m_SelectedItemType = SelectedItemType::PROGRAM;
                        m_SelectedItemId = i;

                        if (ImGui::MenuItem("Delete", ICON_FK_TRASH, "DELETE"))
                            DeleteSelectedItem();

                        ImGui::EndPopup();
                    }
                    ImGui::PopStyleColor(2);
                    ImGui::PopStyleVar(3);
                }
                ImGui::PopStyleColor();
            }
        }

        // Framebuffers
        {
            // Add button
            float xPos = ImGui::GetCursorPosX();
            ImGui::SetCursorPosX(180);
            if (ImGui::SmallButton(ICON_FK_PLUS_CIRCLE "##add_framebuffer"))
            {
                ImNodes::ClearLinkSelection();
                ImNodes::ClearNodeSelection();

                std::string name = "Framebuffer " + std::to_string(m_Framebuffers.size());
                Framebuffer* framebuffer = new Framebuffer(name.c_str());
                AddFramebuffer(framebuffer);

                m_SelectedItemType = SelectedItemType::FRAMEBUFFER;
                m_SelectedItemId = m_Framebuffers.size() - 1;
            }
            ImGui::SameLine();
            ImGui::SetCursorPosX(xPos);

            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 4));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
            bool isNodeOpened = ImGui::CollapsingHeader("FRAMEBUFFERS", ImGuiTreeNodeFlags_SpanAvailWidth);
            ImGui::SameLine();
            ImGui::SetCursorPosX(180);
            ImGui::Text(ICON_FK_PLUS_CIRCLE);
            ImGui::PopStyleVar(3);
            ImGui::PopStyleColor(3);

            if (isNodeOpened)
            {
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.25f, 0.25f, 0.25f, 0.4f));
                for (int i = 1; i < m_Framebuffers.size(); i++)
                {
                    // Item
                    std::string name = "\t\t" + m_Framebuffers[i]->GetName();
                    name += "##framebuffer" + std::to_string(i);
                    bool isSelected = m_SelectedItemType == SelectedItemType::FRAMEBUFFER;
                    isSelected = isSelected && (m_SelectedItemId == i);
                    if (ImGui::Selectable(name.c_str(), &isSelected))
                    {
                        ImNodes::ClearLinkSelection();
                        ImNodes::ClearNodeSelection();
                        if (isSelected)
                        {
                            m_SelectedItemType = SelectedItemType::FRAMEBUFFER;
                            m_SelectedItemId = i;
                        }
                        else
                        {
                            m_SelectedItemType = SelectedItemType::NONE;
                            m_SelectedItemId = -1;
                        }
                    }

                    if (i == 0)
                        continue;
                    // Context menu
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 6));
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(14, 4));
                    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.1f, 0.4f, 0.9f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
                    if (ImGui::BeginPopupContextItem())
                    {
                        ImNodes::ClearLinkSelection();
                        ImNodes::ClearNodeSelection();
                        m_SelectedItemType = SelectedItemType::FRAMEBUFFER;
                        m_SelectedItemId = i;

                        if (ImGui::MenuItem("Delete", ICON_FK_TRASH, "DELETE"))
                            DeleteSelectedItem();

                        ImGui::EndPopup();
                    }
                    ImGui::PopStyleColor(2);
                    ImGui::PopStyleVar(3);
                }
                ImGui::PopStyleColor();
            }
        }

        ImGui::EndChild();
    }

    ImGui::SameLine();
    ImGui::BeginChild("Main Panel", ImVec2(ImGui::GetContentRegionAvail().x - 350,
        ImGui::GetContentRegionAvail().y));

    // Main Frame
    {
        ImGui::BeginChild("Main", ImVec2(ImGui::GetContentRegionAvail().x,
            ImGui::GetContentRegionAvail().y - 226));

        ImNodes::BeginNodeEditor();

        // Nodes rendering
        for (auto& node : m_Nodes)
        {
            if (ImNodes::IsNodeSelected(node->id))
            {
                ImNodes::PushStyleVar(ImNodesStyleVar_NodeBorderThickness, 2.6f);
                ImNodes::PushColorStyle(ImNodesCol_NodeOutline, IM_COL32(220, 140, 0, 255));
            }
            else
            {
                ImNodes::PushStyleVar(ImNodesStyleVar_NodeBorderThickness, 2.0f);
                ImNodes::PushColorStyle(ImNodesCol_NodeOutline, IM_COL32(24, 24, 24, 255));
            }
            // Event nodes
            if (node->type == EditorNodeType::EVENT)
            {
                ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(150, 30, 30, 225));
                ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(150, 30, 30, 225));
                ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(150, 30, 30, 225));

                EditorEventNode* eventNode = (EditorEventNode*)node;
                ImNodes::BeginNode(eventNode->id);
                // Title
                ImNodes::BeginNodeTitleBar();
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
                if (eventNode->eventNodeType == EditorEventNodeType::INIT)
                    ImGui::Text("On Init");
                else
                    ImGui::Text("On Frame");
                ImGui::PopStyleVar();
                ImNodes::EndNodeTitleBar();

                ImGui::Dummy(ImVec2(1.0f, 0.5f));
                for (auto& pin : eventNode->pinsOut)
                {
                    float alpha = 1.0f;
                    if (m_StartedLinkPinId == -1)
                        alpha = 1.0f;
                    else
                    {
                        if (m_StartedLinkPinId == pin->id ||
                            (m_Pins[m_StartedLinkPinId]->type == pin->type
                                && m_Pins[m_StartedLinkPinId]->size == pin->size
                                && !m_Pins[m_StartedLinkPinId]->isOutput
                                && m_Pins[m_StartedLinkPinId]->pNode != node))
                            alpha = 1.0f;
                        else
                            alpha = 0.2f;
                    }
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, alpha));

                    ImNodesPinShape pinShape = BeginPin(pin, alpha);

                    ImNodes::BeginOutputAttribute(pin->id, pinShape);
                    ImGui::Text(" ");
                    ImGui::SameLine();
                    ImGui::Dummy(ImVec2(11.0f, 1.0f));
                    ImNodes::EndOutputAttribute();

                    EndPin();

                    ImGui::PopStyleColor();
                }
                ImGui::Dummy(ImVec2(1.0f, 0.5f));

                ImNodes::EndNode();

                ImNodes::PopColorStyle();
                ImNodes::PopColorStyle();
                ImNodes::PopColorStyle();
            }
            // Program nodes
            if (node->type == EditorNodeType::PROGRAM)
            {
                ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(85, 85, 85, 225));
                ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(85, 85, 85, 225));
                ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(85, 85, 85, 225));

                EditorProgramNode* progNode = (EditorProgramNode*)node;
                ImNodes::BeginNode(progNode->id);
                // Title
                ImNodes::BeginNodeTitleBar();
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
                ImGui::Text(progNode->target->GetName().c_str());
                ImGui::PopStyleVar();
                ImNodes::EndNodeTitleBar();

                // Compute node width
                float nodeWidth = ImGui::CalcTextSize(progNode->target->GetName().c_str()).x;
                float nodeInWidth = 0.0f;
                float nodeOutWidth = 0.0f;
                for (auto& pin : progNode->pinsIn)
                {
                    float textWidth = ImGui::CalcTextSize(pin->name.c_str()).x + 11.0f;
                    float inputWidth = 11.0f;
                    if (pin->connectedLinks.size() == 0)
                    {
                        if (pin->type == EditorPinType::FLOAT || pin->type == EditorPinType::INT)
                            inputWidth = 50 + textWidth;
                        else if (pin->type == EditorPinType::FLOAT2 || pin->type == EditorPinType::INT2)
                            inputWidth = 100 + 11.0f;
                        else if (pin->type == EditorPinType::FLOAT3 || pin->type == EditorPinType::INT3)
                            inputWidth = 150 + 11.0f;
                        else if (pin->type == EditorPinType::FLOAT4 || pin->type == EditorPinType::INT4)
                            inputWidth = 200 + 11.0f;
                    }
                    nodeInWidth = std::max(nodeInWidth, std::max(textWidth, inputWidth));
                }
                for (auto& pin : progNode->pinsOut)
                    nodeOutWidth = std::max(nodeOutWidth, ImGui::CalcTextSize(pin->name.c_str()).x + 11.0f);
                nodeWidth = std::max(nodeWidth, nodeInWidth + nodeOutWidth);

                ImGui::Dummy(ImVec2(1.0f, 0.5f));
                // Inputs
                ImGui::BeginGroup();
                for (auto& pin : progNode->pinsIn)
                    InputPin(node, pin);
                ImGui::EndGroup();
                ImGui::SameLine();
                // Outputs
                ImGui::BeginGroup();
                for (auto& pin : progNode->pinsOut)
                {
                    float alpha = 0.2f;
                    if (m_StartedLinkPinId == -1)
                        alpha = 1.0f;
                    else
                    {
                        if (m_StartedLinkPinId == pin->id ||
                            (m_Pins[m_StartedLinkPinId]->type == pin->type
                                && !m_Pins[m_StartedLinkPinId]->isOutput
                                && m_Pins[m_StartedLinkPinId]->pNode != node))
                        {
                            if (m_Pins[m_StartedLinkPinId]->size == pin->size)
                                alpha = 1.0f;
                            else if ((m_Pins[m_StartedLinkPinId]->pNode->type == EditorNodeType::PINGPONG
                                && m_Pins[m_StartedLinkPinId]->size == 0) ||
                                (node->type == EditorNodeType::PINGPONG && pin->size == 0))
                                alpha = 1.0f;
                        }
                    }
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, alpha));

                    ImNodesPinShape pinShape = BeginPin(pin, alpha);

                    ImNodes::BeginOutputAttribute(pin->id, pinShape);
                    float width = nodeWidth - nodeInWidth - ImGui::CalcTextSize(pin->name.c_str()).x;
                    ImGui::Dummy(ImVec2(width, 1.0f));
                    ImGui::SameLine();
                    ImGui::Text(pin->name.c_str());
                    ImGui::SameLine();
                    ImGui::Dummy(ImVec2(11.0f, 1.0f));
                    ImNodes::EndOutputAttribute();

                    EndPin();

                    ImGui::PopStyleColor();
                }
                ImGui::EndGroup();
                ImGui::Dummy(ImVec2(1.0f, 0.5f));

                ImNodes::EndNode();

                ImNodes::PopColorStyle();
                ImNodes::PopColorStyle();
                ImNodes::PopColorStyle();
            }
            // Block nodes
            if (node->type == EditorNodeType::BLOCK)
            {
                ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(83, 124, 153, 225));
                ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(83, 124, 153, 225));
                ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(83, 124, 153, 225));

                EditorBlockNode* blockNode = (EditorBlockNode*)node;
                ImNodes::BeginNode(blockNode->id);
                // Title
                ImNodes::BeginNodeTitleBar();
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
                ImGui::Text("Block");
                ImGui::PopStyleVar();
                ImNodes::EndNodeTitleBar();

                ImGui::Dummy(ImVec2(1.0f, 0.5f));
                // Inputs
                ImGui::BeginGroup();
                for (auto& pin : blockNode->pinsIn)
                    InputPin(node, pin);
                ImGui::EndGroup();
                ImGui::SameLine();
                // Outputs
                ImGui::BeginGroup();
                for (auto& pin : blockNode->pinsOut)
                    OutputPin(node, pin);
                ImGui::EndGroup();
                ImGui::Dummy(ImVec2(1.0f, 0.5f));

                ImNodes::EndNode();

                ImNodes::PopColorStyle();
                ImNodes::PopColorStyle();
                ImNodes::PopColorStyle();
            }
            // Texture nodes
            if (node->type == EditorNodeType::TEXTURE)
            {
                ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(150, 130, 110, 225));
                ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(150, 130, 110, 225));
                ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(150, 130, 110, 225));

                EditorTextureNode* texNode = (EditorTextureNode*)node;
                ImNodes::BeginNode(texNode->id);

                // Title
                ImNodes::BeginNodeTitleBar();
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
                ImGui::Text("Texture");
                ImGui::PopStyleVar();
                ImNodes::EndNodeTitleBar();

                ImGui::Dummy(ImVec2(1.0f, 0.5f));
                ImGui::Image((void*)texNode->target->GetTexture(), ImVec2(100, 100));

                ImGui::SameLine();
                ImGui::BeginGroup();
                for (auto& pin : texNode->pinsOut)
                    OutputPin(node, pin);
                ImGui::EndGroup();
                ImGui::Dummy(ImVec2(1.0f, 0.5f));

                ImNodes::EndNode();

                ImNodes::PopColorStyle();
                ImNodes::PopColorStyle();
                ImNodes::PopColorStyle();
            }
            // Image nodes
            if (node->type == EditorNodeType::IMAGE)
            {
                ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(118, 32, 140, 225));
                ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(118, 32, 140, 225));
                ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(118, 32, 140, 225));

                EditorImageNode* imgNode = (EditorImageNode*)node;
                ImNodes::BeginNode(imgNode->id);

                // Title
                ImNodes::BeginNodeTitleBar();
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
                ImGui::Text("Image");
                ImGui::PopStyleVar();
                ImNodes::EndNodeTitleBar();

                ImGui::Dummy(ImVec2(1.0f, 0.5f));
                // Inputs
                ImGui::BeginGroup();
                for (auto& pin : imgNode->pinsIn)
                    InputPin(node, pin);
                ImGui::EndGroup();
                ImGui::SameLine();
                // Outputs
                ImGui::BeginGroup();
                for (auto& pin : imgNode->pinsOut)
                    OutputPin(node, pin);
                ImGui::EndGroup();
                ImGui::Dummy(ImVec2(1.0f, 0.5f));

                ImNodes::EndNode();

                ImNodes::PopColorStyle();
                ImNodes::PopColorStyle();
                ImNodes::PopColorStyle();
            }
            // Ping-pong nodes
            if (node->type == EditorNodeType::PINGPONG)
            {
                ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(83, 124, 153, 225));
                ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(83, 124, 153, 225));
                ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(83, 124, 153, 225));

                EditorPingPongNode* pingpongNode = (EditorPingPongNode*)node;
                ImNodes::BeginNode(pingpongNode->id);

                // Title
                ImNodes::BeginNodeTitleBar();
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
                ImGui::Text("Ping-Pong");
                ImGui::PopStyleVar();
                ImNodes::EndNodeTitleBar();

                ImGui::Dummy(ImVec2(1.0f, 0.5f));
                // Inputs
                ImGui::BeginGroup();
                for (auto& pin : pingpongNode->pinsIn)
                    InputPin(node, pin);
                ImGui::EndGroup();
                ImGui::SameLine();
                // Outputs
                ImGui::BeginGroup();
                for (auto& pin : pingpongNode->pinsOut)
                    OutputPin(node, pin);
                ImGui::EndGroup();
                ImGui::Dummy(ImVec2(1.0f, 0.5f));

                ImNodes::EndNode();

                ImNodes::PopColorStyle();
                ImNodes::PopColorStyle();
                ImNodes::PopColorStyle();
            }
            // Other nodes
            else if (node->type == EditorNodeType::TIME || node->type == EditorNodeType::MOUSE_POS)
            {
                if (node->type == EditorNodeType::TIME)
                {
                    ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(110, 146, 104, 225));
                    ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(110, 146, 104, 225));
                    ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(110, 146, 104, 225));
                }
                else
                {
                    ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(160, 160, 40, 225));
                    ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(160, 160, 40, 225));
                    ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(160, 160, 40, 225));
                }

                ImNodes::BeginNode(node->id);

                // Title
                ImNodes::BeginNodeTitleBar();
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
                if (node->type == EditorNodeType::TIME)
                    ImGui::Text("Time");
                else if (node->type == EditorNodeType::MOUSE_POS)
                    ImGui::Text("Mouse Position");
                ImGui::PopStyleVar();
                ImNodes::EndNodeTitleBar();

                ImGui::Dummy(ImVec2(1.0f, 0.5f));
                // Outputs
                ImGui::BeginGroup();
                for (auto& pin : node->pinsOut)
                    OutputPin(node, pin);
                ImGui::EndGroup();
                ImGui::Dummy(ImVec2(1.0f, 0.5f));

                ImNodes::EndNode();

                ImNodes::PopColorStyle();
                ImNodes::PopColorStyle();
                ImNodes::PopColorStyle();
            }
            node->nodePos = ImNodes::GetNodeScreenSpacePos(node->id);
            ImNodes::PopColorStyle();
            ImNodes::PopStyleVar();
        }

        // Links Rendering
        for (auto& link : m_Links)
        {
            int alpha = m_StartedLinkPinId == -1 ? 255 : 50;
            if (link->pPin1->type == EditorPinType::FLOW)
            {
                ImNodes::PushColorStyle(ImNodesCol_Link, IM_COL32(225, 225, 225, alpha));
                ImNodes::PushColorStyle(ImNodesCol_LinkHovered, IM_COL32(255, 255, 255, alpha));
                ImNodes::PushColorStyle(ImNodesCol_LinkSelected, IM_COL32(255, 255, 255, 255));
            }
            else if (link->pPin1->type == EditorPinType::INT)
            {
                ImNodes::PushColorStyle(ImNodesCol_Link, IM_COL32(33, 227, 175, alpha));
                ImNodes::PushColorStyle(ImNodesCol_LinkHovered, IM_COL32(135, 239, 195, alpha));
                ImNodes::PushColorStyle(ImNodesCol_LinkSelected, IM_COL32(135, 239, 195, 255));
            }
            else if (link->pPin1->type == EditorPinType::FLOAT)
            {
                ImNodes::PushColorStyle(ImNodesCol_Link, IM_COL32(156, 253, 65, alpha));
                ImNodes::PushColorStyle(ImNodesCol_LinkHovered, IM_COL32(144, 225, 137, alpha));
                ImNodes::PushColorStyle(ImNodesCol_LinkSelected, IM_COL32(144, 225, 137, 255));
            }
            else if (link->pPin1->type == EditorPinType::BLOCK)
            {
                ImNodes::PushColorStyle(ImNodesCol_Link, IM_COL32(6, 165, 239, alpha));
                ImNodes::PushColorStyle(ImNodesCol_LinkHovered, IM_COL32(137, 196, 247, alpha));
                ImNodes::PushColorStyle(ImNodesCol_LinkSelected, IM_COL32(137, 196, 247, 255));
            }
            else if (link->pPin1->type == EditorPinType::TEXTURE)
            {
                ImNodes::PushColorStyle(ImNodesCol_Link, IM_COL32(148, 0, 0, alpha));
                ImNodes::PushColorStyle(ImNodesCol_LinkHovered, IM_COL32(183, 137, 137, alpha));
                ImNodes::PushColorStyle(ImNodesCol_LinkSelected, IM_COL32(183, 137, 137, 255));
            }
            else if (link->pPin1->type == EditorPinType::IMAGE)
            {
                ImNodes::PushColorStyle(ImNodesCol_Link, IM_COL32(200, 130, 255, alpha));
                ImNodes::PushColorStyle(ImNodesCol_LinkHovered, IM_COL32(220, 170, 255, alpha));
                ImNodes::PushColorStyle(ImNodesCol_LinkSelected, IM_COL32(220, 170, 255, 255));
            }
            else
            {
                ImNodes::PushColorStyle(ImNodesCol_Link, IM_COL32(252, 200, 35, alpha));
                ImNodes::PushColorStyle(ImNodesCol_LinkHovered, IM_COL32(255, 217, 140, alpha));
                ImNodes::PushColorStyle(ImNodesCol_LinkSelected, IM_COL32(255, 217, 140, 255));
            }
            ImNodes::Link(link->id, link->pPin1->id, link->pPin2->id);
            ImNodes::PopColorStyle();
            ImNodes::PopColorStyle();
            ImNodes::PopColorStyle();
        }

        ImNodes::EndNodeEditor();
        ImGui::EndChild();

        // Node selection
        if (ImNodes::NumSelectedNodes() == 1 && ImNodes::NumSelectedLinks() == 0)
        {
            int* ids = new int;
            ImNodes::GetSelectedNodes(ids);
            int id = *ids;
            delete ids;
            if (m_Nodes[id]->type == EditorNodeType::PROGRAM)
                m_SelectedItemType = SelectedItemType::PROGRAM_NODE;
            else if (m_Nodes[id]->type == EditorNodeType::BLOCK)
                m_SelectedItemType = SelectedItemType::BUFFER_NODE;
            else if (m_Nodes[id]->type == EditorNodeType::IMAGE)
                m_SelectedItemType = SelectedItemType::IMAGE_NODE;
            else if (m_Nodes[id]->type == EditorNodeType::PINGPONG)
                m_SelectedItemType = SelectedItemType::PINGPONG_NODE;
            else
                m_SelectedItemType = SelectedItemType::NODE;
            m_SelectedItemId = id;
        }
        else if (ImNodes::NumSelectedNodes() > 1 || ImNodes::NumSelectedLinks() > 0)
            m_SelectedItemType = SelectedItemType::NODES;
        else if (m_SelectedItemType >= SelectedItemType::NODES)
            m_SelectedItemType = SelectedItemType::NONE;

        // Start link
        if (ImNodes::IsLinkStarted(&m_StartedLinkPinId))
        {
            if (m_Pins[m_StartedLinkPinId]->type == EditorPinType::FLOW)
                ImNodes::GetStyle().Colors[ImNodesCol_Link] = IM_COL32(225, 225, 225, 255);
            else if (m_Pins[m_StartedLinkPinId]->type == EditorPinType::INT)
                ImNodes::GetStyle().Colors[ImNodesCol_Link] = IM_COL32(33, 227, 175, 255);
            else if (m_Pins[m_StartedLinkPinId]->type == EditorPinType::FLOAT)
                ImNodes::GetStyle().Colors[ImNodesCol_Link] = IM_COL32(156, 253, 65, 255);
            else if (m_Pins[m_StartedLinkPinId]->type == EditorPinType::BLOCK)
                ImNodes::GetStyle().Colors[ImNodesCol_Link] = IM_COL32(6, 165, 239, 255);
            else if (m_Pins[m_StartedLinkPinId]->type == EditorPinType::TEXTURE)
                ImNodes::GetStyle().Colors[ImNodesCol_Link] = IM_COL32(148, 0, 0, 255);
            else if (m_Pins[m_StartedLinkPinId]->type == EditorPinType::IMAGE)
                ImNodes::GetStyle().Colors[ImNodesCol_Link] = IM_COL32(200, 130, 255, 255);
            else
                ImNodes::GetStyle().Colors[ImNodesCol_Link] = IM_COL32(252, 200, 35, 255);
        }
        // Drop link
        if (ImNodes::IsLinkDropped())
        {
            ImGui::OpenPopup("editor_context_menu_nodes");
            m_bLinkHanged = true;
            m_HangPos = ImGui::GetMousePos();
        }

        // Link creation
        int startPinId, endPinId;
        if (ImNodes::IsLinkCreated(&startPinId, &endPinId))
        {
            m_StartedLinkPinId = -1;
            CreateLink(startPinId, endPinId);
        }

        // Context menu
        if (ImGui::GetMouseDragDelta(ImGuiMouseButton_Right).x == 0.0f
            && ImGui::GetMouseDragDelta(ImGuiMouseButton_Right).y == 0.0f
            && ImGui::IsMouseReleased(ImGuiMouseButton_Right)
            && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows))
        {
            int id = -1;
            if (ImNodes::IsNodeHovered(&id))
            {
                ImNodes::ClearNodeSelection();
                ImNodes::ClearLinkSelection();
                ImNodes::SelectNode(id);
                m_SelectedItemType = SelectedItemType::NODE;
                m_SelectedItemId = id;
                ImGui::OpenPopup("editor_context_menu_node");
            }
            else if (ImNodes::IsLinkHovered(&id))
            {
                ImNodes::ClearNodeSelection();
                ImNodes::ClearLinkSelection();
                ImNodes::SelectLink(id);
                m_SelectedItemType = SelectedItemType::LINK;
                m_SelectedItemId = id;
                ImGui::OpenPopup("editor_context_menu_link");
            }
            else
            {
                ImGui::OpenPopup("editor_context_menu_nodes");
                m_HangPos = ImGui::GetMousePos();
            }
        }
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 6));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(14, 4));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.1f, 0.4f, 0.9f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
        if (ImGui::BeginPopup("editor_context_menu_node"))
        {
            if (m_Nodes[m_SelectedItemId]->type != EditorNodeType::EVENT)
            {
                if (ImGui::MenuItem("Delete", ICON_FK_TRASH, "DELETE"))
                {
                    DeleteNode(m_SelectedItemId);
                    UpdateNodes();
                    UpdatePins();
                    UpdateLinks();
                }
            }
            else
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
        else if (ImGui::BeginPopup("editor_context_menu_link"))
        {
            if (ImGui::MenuItem("Delete", ICON_FK_TRASH, "DELETE"))
            {
                DeleteLink(m_SelectedItemId);
                UpdateLinks();
            }
            ImGui::EndPopup();
        }
        else if (ImGui::BeginPopup("editor_context_menu_nodes"))
        {
            if (m_StartedLinkPinId != -1)
            {
                EditorPin* pin = m_Pins[m_StartedLinkPinId];
                if (pin->type == EditorPinType::BLOCK &&
                    pin->pNode->type == EditorNodeType::PROGRAM)
                {
                    if (!pin->isOutput)
                    {
                        if (ImGui::MenuItem("Block"))
                            CreateBlockNode(m_HangPos, m_StartedLinkPinId);
                        if (ImGui::MenuItem("Ping-Pong"))
                        {
                            CreatePingPongNode(m_HangPos);
                            CreateLink(m_Nodes.back()->pinsOut[0]->id, pin->id);
                        }
                    }
                    else
                    {
                        if (ImGui::MenuItem("Ping-Pong"))
                        {
                            CreatePingPongNode(m_HangPos);
                            CreateLink(m_Nodes.back()->pinsIn[0]->id, pin->id);
                        }
                    }
                }
                else if (pin->type == EditorPinType::BLOCK &&
                    pin->pNode->type == EditorNodeType::PINGPONG)
                {
                    if (ImGui::MenuItem("Ping-Pong"))
                    {
                        CreatePingPongNode(m_HangPos);
                        if (!pin->isOutput)
                            CreateLink(m_Nodes.back()->pinsOut[0]->id, pin->id);
                        else
                            CreateLink(m_Nodes.back()->pinsIn[0]->id, pin->id);
                    }
                }
                else if (pin->type == EditorPinType::FLOW && m_Programs.size() > 0)
                {
                    for (int i = 0; i < m_Programs.size(); i++)
                    {
                        if (ImGui::MenuItem(m_Programs[i]->GetName().c_str()))
                        {
                            CreateProgramNode(i, m_HangPos);
                            EditorProgramNode* newNode = (EditorProgramNode*)m_Nodes.back();
                            if (pin->isOutput)
                                CreateLink(pin->id, newNode->flowIn->id);
                            else
                                CreateLink(newNode->flowOut->id, pin->id);
                        }
                    }
                }
                else if (pin->type == EditorPinType::TEXTURE &&
                    !pin->isOutput &&
                    m_Textures.size() > 0)
                {
                    for (int i = 0; i < m_Textures.size(); i++)
                    {
                        std::string name = "Texture: " + m_Textures[i]->GetName();
                        if (ImGui::MenuItem(name.c_str()))
                        {
                            CreateTextureNode(i, m_HangPos);
                            CreateLink(m_Nodes.back()->pinsOut[0]->id, pin->id);
                        }
                    }
                }
                else if (pin->type == EditorPinType::TEXTURE &&
                    pin->isOutput &&
                    pin->pNode->type == EditorNodeType::TEXTURE)
                {
                    if (ImGui::MenuItem("Image"))
                    {
                        CreateImageNode(m_HangPos);
                        CreateLink(m_Nodes.back()->pinsIn[0]->id, pin->id);
                    }
                }
                else if (pin->type == EditorPinType::IMAGE &&
                    pin->pNode->type == EditorNodeType::PROGRAM &&
                    !pin->isOutput)
                {
                    if (ImGui::MenuItem("Image"))
                    {
                        CreateImageNode(m_HangPos);
                        CreateLink(m_Nodes.back()->pinsOut[0]->id, pin->id);
                    }
                    if (ImGui::MenuItem("Ping-Pong"))
                    {
                        CreatePingPongNode(m_HangPos, EditorPingPongNodeType::IMAGE);
                        CreateLink(m_Nodes.back()->pinsOut[0]->id, pin->id);
                    }
                }
                else if (pin->type == EditorPinType::IMAGE &&
                    pin->pNode->type == EditorNodeType::PINGPONG)
                {
                    if (!pin->isOutput)
                    {
                        if (ImGui::MenuItem("Image"))
                        {
                            CreateImageNode(m_HangPos);
                            CreateLink(m_Nodes.back()->pinsOut[0]->id, pin->id);
                        }
                    }
                    if (ImGui::MenuItem("Ping-Pong"))
                    {
                        CreatePingPongNode(m_HangPos, EditorPingPongNodeType::IMAGE);
                        if (!pin->isOutput)
                            CreateLink(m_Nodes.back()->pinsOut[0]->id, pin->id);
                        else
                            CreateLink(m_Nodes.back()->pinsIn[0]->id, pin->id);
                    }
                }
                else if (pin->type == EditorPinType::FLOAT && !pin->isOutput)
                {
                    if (ImGui::MenuItem("Time"))
                    {
                        CreateTimeNode(m_HangPos);
                        CreateLink(m_Nodes.back()->pinsOut[0]->id, pin->id);
                    }
                }
                else if (pin->type == EditorPinType::FLOAT2 && !pin->isOutput)
                {
                    if (ImGui::MenuItem("Mouse Position"))
                    {
                        CreateMousePosNode(m_HangPos);
                        CreateLink(m_Nodes.back()->pinsOut[0]->id, pin->id);
                    }
                }
                else
                    ImGui::CloseCurrentPopup();
            }
            else
            {
                if (m_Programs.size() > 0)
                {
                    for (int i = 0; i < m_Programs.size(); i++)
                    {
                        if (ImGui::MenuItem(m_Programs[i]->GetName().c_str()))
                            CreateProgramNode(i, m_HangPos);
                    }
                    ImGui::Separator();
                }
                if (m_Textures.size() > 0)
                {
                    for (int i = 0; i < m_Textures.size(); i++)
                    {
                        std::string name = "Texture: " + m_Textures[i]->GetName();
                        if (ImGui::MenuItem(name.c_str()))
                            CreateTextureNode(i, m_HangPos);
                    }
                    ImGui::Separator();
                }
                if (ImGui::MenuItem("Block"))
                    CreateBlockNode(m_HangPos);
                if (ImGui::MenuItem("Image"))
                    CreateImageNode(m_HangPos);
                if (ImGui::MenuItem("Ping-Pong"))
                    CreatePingPongNode(m_HangPos);
                ImGui::Separator();
                if (ImGui::MenuItem("Time"))
                    CreateTimeNode(m_HangPos);
                if (ImGui::MenuItem("Mouse Position"))
                    CreateMousePosNode(m_HangPos);
            }
            ImGui::EndPopup();
        }
        else if (m_bLinkHanged)
        {
            m_bLinkHanged = false;
            m_StartedLinkPinId = -1;
        }
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(3);

        // Drag and drop creation
        if (ImGui::BeginDragDropTarget())
        {
            if (auto payload = ImGui::AcceptDragDropPayload("program"))
            {
                IM_ASSERT(payload->DataSize == sizeof(int));
                int id = *(const int*)payload->Data;
                auto mousePos = ImGui::GetMousePos();
                CreateProgramNode(id, mousePos);
            }
            else if (auto payload = ImGui::AcceptDragDropPayload("texture"))
            {
                IM_ASSERT(payload->DataSize == sizeof(int));
                int id = *(const int*)payload->Data;
                auto mousePos = ImGui::GetMousePos();
                CreateTextureNode(id, mousePos);
            }
            ImGui::EndDragDropTarget();
        }

        // Delete key event
        if (ImGui::IsKeyPressed(ImGuiKey_Delete, false))
            DeleteSelectedItem();
    }

    // Bottom Panel
    {
        ImGui::BeginChild("Bottom Panel", ImVec2(ImGui::GetContentRegionAvail().x, 216));
        if (ImGui::BeginTabBar("BottomTabBar"))
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            if (ImGui::BeginTabItem("Textures"))
            {
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.25f, 0.25f, 0.25f, 0.4f));
                ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));
                ImGui::BeginChild("TexturesSubFrame");

                ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x + 6, ImGui::GetCursorPos().y + 6));
                if (ImGui::IconButton(ICON_FK_PLUS, "  Add##tex", ImVec4(0.4f, 0.8f, 0.4f, 1.0f), ImVec2(70, 25)))
                {
                    const char* filterItems[5] = { "*.jpg", "*.jpeg", "*.png", "*.bmp", "*.tga" };
                    const char* filterDesc = "Image Files (*.jpg;*.jpeg;*.png;*.bmp;*.tga)";
                    auto paths_c = tinyfd_openFileDialog("Load Texture", "", 5, filterItems, filterDesc, 1);
                    if (paths_c)
                    {
                        std::stringstream ssPaths(paths_c);
                        std::string path;
                        while (std::getline(ssPaths, path, '|'))
                        {
                            Texture* tex = new Texture(PathUtil::UniversalPath(path).c_str());
                            m_Textures.push_back(tex);
                        }
                    }
                }

                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6);
                ImGui::Separator();

                // Textures browser
                {
                    ImGui::BeginChild("TextureBrowser");

                    ImGui::Dummy(ImVec2(1, 10));
                    for (int i = 0; i < m_Textures.size(); i++)
                    {
                        ImGui::Dummy(ImVec2(10, 140));
                        ImGui::SameLine();
                        ImGui::BeginGroup();
                        std::string idStr = "##texture" + std::to_string(i);
                        ImGui::Button(idStr.c_str(), ImVec2(120, 140));

                        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                        {
                            ImGui::SetDragDropPayload("texture", &i, sizeof(int));
                            ImGui::Text(m_Textures[i]->GetName().c_str());
                            ImGui::EndDragDropSource();
                        }

                        // Context menu
                        bool itemDeleted = false;
                        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 6));
                        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(14, 4));
                        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.1f, 0.4f, 0.9f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
                        if (ImGui::BeginPopupContextItem())
                        {
                            ImNodes::ClearLinkSelection();
                            ImNodes::ClearNodeSelection();
                            m_SelectedItemType = SelectedItemType::TEXTURE;
                            m_SelectedItemId = i;

                            if (ImGui::MenuItem("Delete", ICON_FK_TRASH, "DELETE"))
                            {
                                DeleteSelectedItem();
                                itemDeleted = true;
                            }

                            ImGui::EndPopup();
                        }
                        ImGui::PopStyleColor(2);
                        ImGui::PopStyleVar(3);

                        if (!itemDeleted)
                        {
                            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
                            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 130);
                            ImGui::Image((void*)m_Textures[i]->GetTexture(), ImVec2(100, 100));
                            ImGui::Dummy(ImVec2(2, 1));
                            ImGui::SameLine();
                            ImGui::SetNextItemWidth(108);
                            ImGui::TextClipped(m_Textures[i]->GetName().c_str());
                        }
                        ImGui::EndGroup();

                        ImGui::SameLine();
                        if (ImGui::GetContentRegionAvail().x < 130)
                        {
                            ImGui::NewLine();
                            ImGui::NewLine();
                        }
                    }
                    ImGui::NewLine();
                    ImGui::Dummy(ImVec2(1, 10));

                    ImGui::EndChild();
                }

                ImGui::EndChild();
                ImGui::PopStyleColor();
                ImGui::PopStyleColor();
                ImGui::EndTabItem();
            }
            ImGui::PopStyleVar();
            ImGui::EndTabBar();
        }
        ImGui::EndChild();
    }

    ImGui::EndChild();

    // Right Panel
    {
        ImGui::SameLine();
        ImGui::BeginChild("Right Panel", ImVec2(344, ImGui::GetContentRegionAvail().y));

        if (m_SelectedItemType == SelectedItemType::PROGRAM)
        {
            // Section 1: Basic info
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 4));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
            bool isNodeOpened = ImGui::CollapsingHeader("Program", ImGuiTreeNodeFlags_SpanAvailWidth);
            ImGui::PopStyleVar(3);
            ImGui::PopStyleColor(3);

            if (isNodeOpened)
            {
                // Name
                ImGui::Text("\t\tName");
                ImGui::SameLine(160);
                ImGui::SetNextItemWidth(150);
                std::string name = m_Programs[m_SelectedItemId]->GetName();
                if (ImGui::InputText("##progName", &name))
                    m_Programs[m_SelectedItemId]->SetName(name.c_str());
            }

            // Section 2: Shaders
            // Add Button
            float xPos = ImGui::GetCursorPosX();
            ImGui::SetCursorPosX(325);
            if (ImGui::SmallButton(ICON_FK_PLUS_CIRCLE "##add_shader"))
            {
                auto paths_c = tinyfd_openFileDialog("Add Shader", "", 0, 0, 0, 1);
                if (paths_c)
                {
                    std::stringstream ssPaths(paths_c);
                    std::string path;
                    while (std::getline(ssPaths, path, '|'))
                        m_Programs[m_SelectedItemId]->AddShader(PathUtil::UniversalPath(path).c_str(),
                            GL_VERTEX_SHADER);
                }
            }
            ImGui::SameLine();
            ImGui::SetCursorPosX(xPos);

            // Title bar
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 4));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
            isNodeOpened = ImGui::CollapsingHeader("Shaders", ImGuiTreeNodeFlags_SpanAvailWidth);
            ImGui::SameLine();
            ImGui::SetCursorPosX(325);
            ImGui::Text(ICON_FK_PLUS_CIRCLE);
            ImGui::PopStyleVar(3);
            ImGui::PopStyleColor(3);

            if (isNodeOpened)
            {
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.25f, 0.25f, 0.25f, 0.4f));
                std::vector<std::string> shaders = m_Programs[m_SelectedItemId]->GetShaderFiles();
                for (int i = 0; i < shaders.size(); i++)
                {
                    // Name
                    std::string name = shaders[i];
                    name = "\t" + name.substr(name.find_last_of('/') + 1);
                    ImGui::SetNextItemWidth(150);
                    ImGui::TextClipped(name.c_str());

                    // Type
                    int iVar = 0;
                    GLenum type = m_Programs[m_SelectedItemId]->GetShaderTypes()[i];
                    if (type == GL_VERTEX_SHADER)
                        iVar = 0;
                    else if (type == GL_FRAGMENT_SHADER)
                        iVar = 1;
                    else if (type == GL_COMPUTE_SHADER)
                        iVar = 2;
                    const char* items = "Vertex\0Fragment\0Compute\0";
                    std::string idStr = "##shadertype" + std::to_string(i);
                    ImGui::SameLine(160);
                    ImGui::SetNextItemWidth(150);
                    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));
                    if (ImGui::Combo(idStr.c_str(), &iVar, items))
                    {
                        if (iVar == 0)
                            m_Programs[m_SelectedItemId]->SetShaderType(i, GL_VERTEX_SHADER);
                        else if (iVar == 1)
                            m_Programs[m_SelectedItemId]->SetShaderType(i, GL_FRAGMENT_SHADER);
                        else if (iVar == 2)
                            m_Programs[m_SelectedItemId]->SetShaderType(i, GL_COMPUTE_SHADER);
                    }
                    ImGui::PopStyleColor();

                    // Delete button
                    ImGui::SameLine();
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
                    idStr = ICON_FK_TRASH "##shader" + std::to_string(i);
                    bool itemDeleted = false;
                    if (ImGui::SmallButton(idStr.c_str()))
                    {
                        m_Programs[m_SelectedItemId]->RemoveShader(i);
                        itemDeleted = true;
                    }
                    ImGui::PopStyleColor(3);
                    ImGui::PopStyleVar();
                    if (itemDeleted)
                        break;
                }
                ImGui::PopStyleColor();
            }
        }

        else if (m_SelectedItemType == SelectedItemType::FRAMEBUFFER)
        {
            if (m_SelectedItemId != 0)
            {
                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 4));
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
                ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
                bool isNodeOpened = ImGui::CollapsingHeader("Framebuffer", ImGuiTreeNodeFlags_SpanAvailWidth);
                ImGui::PopStyleVar(3);
                ImGui::PopStyleColor(3);

                if (isNodeOpened)
                {
                    // Name
                    ImGui::Text("\t\tName");
                    ImGui::SameLine(160);
                    ImGui::SetNextItemWidth(150);
                    std::string name = m_Framebuffers[m_SelectedItemId]->GetName();
                    if (ImGui::InputText("##framebufferName", &name))
                        m_Framebuffers[m_SelectedItemId]->SetName(name.c_str());

                    // Size
                    int x, y;
                    m_Framebuffers[m_SelectedItemId]->GetSize(&x, &y);
                    ImGui::Text("\t\tWidth");
                    ImGui::SameLine(160);
                    ImGui::SetNextItemWidth(150);
                    if (ImGui::DragInt("##framebufferSizeX", &x, 1.0f, 0, 4096))
                    {
                        if (x < 0) x = 0;
                        if (x > 4096) x = 4096;
                        m_Framebuffers[m_SelectedItemId]->SetSize(x, y);
                    }
                    ImGui::Text("\t\tHeight");
                    ImGui::SameLine(160);
                    ImGui::SetNextItemWidth(150);
                    if (ImGui::DragInt("##framebufferSizeY", &y, 1.0f, 0, 4096))
                    {
                        if (y < 0) y = 0;
                        if (y > 4096) y = 4096;
                        m_Framebuffers[m_SelectedItemId]->SetSize(x, y);
                    }

                    // Attachments
                    ImGui::Text("\t\tAttachments");
                    ImGui::SameLine(160);
                    ImGui::SetNextItemWidth(150);
                    int iVal = m_Framebuffers[m_SelectedItemId]->NumAttachments();
                    if (ImGui::SliderInt("##framebufferAttachments", &iVal, 0, 8))
                    {
                        if (iVal < 0) iVal = 0;
                        if (iVal > 8) iVal = 8;
                        m_Framebuffers[m_SelectedItemId]->SetNumAttachments(iVal);
                    }

                    // Renderbuffer
                    ImGui::Text("\t\tRenderbuffer");
                    ImGui::SameLine(160);
                    ImGui::SetNextItemWidth(150);
                    bool hasRenderbuffer = m_Framebuffers[m_SelectedItemId]->HasRenderbuffer();
                    if (ImGui::Checkbox("##framebufferRenderbuffer", &hasRenderbuffer))
                        m_Framebuffers[m_SelectedItemId]->SetRenderbuffer(hasRenderbuffer);
                }
            }
        }

        else if (m_SelectedItemType == SelectedItemType::PROGRAM_NODE)
        {
            // title bar
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 4));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
            bool isNodeOpened = ImGui::CollapsingHeader("Program Node", ImGuiTreeNodeFlags_SpanAvailWidth);
            ImGui::PopStyleVar(3);
            ImGui::PopStyleColor(3);

            if (isNodeOpened)
            {
                EditorProgramNode* node = (EditorProgramNode*)m_Nodes[m_SelectedItemId];

                // Dispatch type
                ImGui::Text("\t\tDispatch Type");
                ImGui::SameLine(160);
                ImGui::SetNextItemWidth(150);
                int iVal = (int)node->dispatchType;
                const char* items = "Array\0Compute\0";
                ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));
                if (ImGui::Combo("##progNodeDispatchType", &iVal, items))
                {
                    node->drawMode = GL_POINTS;
                    if (iVal == 0)
                    {
                        node->dispatchType = EditorProgramDispatchType::ARRAY;
                        node->dispatchSize[0] = 0;
                        node->dispatchSize[1] = 0;
                        node->dispatchSize[2] = 0;
                    }
                    else if (iVal == 1)
                    {
                        node->dispatchType = EditorProgramDispatchType::COMPUTE;
                        node->dispatchSize[0] = 1;
                        node->dispatchSize[1] = 1;
                        node->dispatchSize[2] = 1;
                        SetProgramNodeFramebuffer(node, 0);
                    }
                }
                ImGui::PopStyleColor();

                if (node->dispatchType == EditorProgramDispatchType::ARRAY)
                {
                    // Framebuffer
                    ImGui::Text("\t\tFramebuffer");
                    ImGui::SameLine(160);
                    ImGui::SetNextItemWidth(150);
                    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));
                    if (ImGui::BeginCombo("##progNodeFramebuffer", node->framebuffer->GetName().c_str()))
                    {
                        int index = 0;
                        for (auto& framebuffer : m_Framebuffers)
                        {
                            const bool is_selected = (node->framebuffer == framebuffer);
                            if (ImGui::Selectable(framebuffer->GetName().c_str(), is_selected))
                                SetProgramNodeFramebuffer(node, index);
                            if (is_selected)
                                ImGui::SetItemDefaultFocus();
                            index++;
                        }
                        ImGui::EndCombo();
                    }
                    ImGui::PopStyleColor();

                    // Draw mode
                    ImGui::Text("\t\tDraw Mode");
                    ImGui::SameLine(160);
                    ImGui::SetNextItemWidth(150);
                    iVal = EditorNodeUtil::GLDrawModeToIndex(node->drawMode);
                    items =
                        "Points\0"
                        "Line Strip\0"
                        "Line Loop\0"
                        "Lines\0"
                        "Line Strip Adjacency\0"
                        "Lines Adjacency\0"
                        "Triangle Strip\0"
                        "Triangle Fan\0"
                        "Triangles\0"
                        "Triangle Strip Adjacency\0"
                        "Triangles Adjacency\0";
                    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));
                    if (ImGui::Combo("##progNodeDrawMode", &iVal, items))
                        node->drawMode = EditorNodeUtil::IndexToGLDrawMode(iVal);
                    ImGui::PopStyleColor();

                    // Size
                    ImGui::Text("\t\tSize");
                    ImGui::SameLine(160);
                    ImGui::SetNextItemWidth(150);
                    iVal = node->dispatchSize[0];
                    if (ImGui::DragInt("##progNodeDrawSize", &iVal, 1.0f))
                    {
                        if (iVal < 0) iVal = 0;
                        node->dispatchSize[0] = iVal;
                    }
                }

                else if (node->dispatchType == EditorProgramDispatchType::COMPUTE)
                {
                    int size[3]{};
                    size[0] = node->dispatchSize[0];
                    size[1] = node->dispatchSize[1];
                    size[2] = node->dispatchSize[2];
                    // Dispatch size
                    ImGui::Text("\t\tWork Group Size X");
                    ImGui::SameLine(160);
                    ImGui::SetNextItemWidth(150);
                    if (ImGui::DragInt("##progNodeWorkGroupSizeX", &size[0], 1.0f))
                    {
                        if (size[0] < 0) size[0] = 0;
                        node->dispatchSize[0] = size[0];
                    }
                    ImGui::Text("\t\tWork Group Size Y");
                    ImGui::SameLine(160);
                    ImGui::SetNextItemWidth(150);
                    if (ImGui::DragInt("##progNodeWorkGroupSizeY", &size[1], 1.0f))
                    {
                        if (size[1] < 0) size[1] = 0;
                        node->dispatchSize[1] = size[1];
                    }
                    ImGui::Text("\t\tWork Group Size Z");
                    ImGui::SameLine(160);
                    ImGui::SetNextItemWidth(150);
                    if (ImGui::DragInt("##progNodeWorkGroupSizeZ", &size[2], 1.0f))
                    {
                        if (size[2] < 0) size[2] = 0;
                        node->dispatchSize[2] = size[2];
                    }
                }
            }
        }

        else if (m_SelectedItemType == SelectedItemType::BUFFER_NODE)
        {
            // Get Selection Id
            int id;
            ImNodes::GetSelectedNodes(&id);
            EditorBlockNode* node = (EditorBlockNode*)m_Nodes[id];

            bool needsUpdate = false;

            // Add Button
            float xPos = ImGui::GetCursorPosX();
            ImGui::SetCursorPosX(325);
            if (ImGui::SmallButton(ICON_FK_PLUS_CIRCLE "##add_var"))
            {
                EditorPin* newPin = new EditorFloatPin;
                newPin->type = EditorPinType::FLOAT;
                newPin->pNode = node;
                newPin->name = "new_var" + std::to_string(node->pinsIn.size());
                newPin->id = m_Pins.size();
                m_Pins.push_back(newPin);
                node->pinsIn.push_back(newPin);
                node->size += EditorNodeUtil::PinTypeSize(newPin->type);
                node->pinsOut[0]->size = node->size;
                auto links = node->pinsOut[0]->connectedLinks;
                for (auto& link : links)
                    DeleteLink(link->id);
                UpdateLinks();
                needsUpdate = true;
            }
            ImGui::SameLine();
            ImGui::SetCursorPosX(xPos);

            // Title bar
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 4));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
            bool isNodeOpened = ImGui::CollapsingHeader("Variables", ImGuiTreeNodeFlags_SpanAvailWidth);
            ImGui::SameLine();
            ImGui::SetCursorPosX(325);
            ImGui::Text(ICON_FK_PLUS_CIRCLE);
            ImGui::PopStyleVar(3);
            ImGui::PopStyleColor(3);

            if (isNodeOpened)
            {
                int pinIndex = 0;
                for (auto& pinIn : node->pinsIn)
                {
                    // name
                    ImGui::SetNextItemWidth(150);
                    std::string idStr = "##buffer_var_name" + std::to_string(pinIn->id);
                    ImGui::InputText(idStr.c_str(), &pinIn->name);

                    // Type
                    int iVar = (int)pinIn->type - 1;
                    EditorPinType type = pinIn->type;
                    const char* items = "float\0float2\0float3\0float4\0int\0int2\0int3\0int4\0";
                    idStr = "##buffer_var_type" + std::to_string(pinIn->id);
                    ImGui::SameLine(160);
                    ImGui::SetNextItemWidth(150);
                    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));
                    if (ImGui::Combo(idStr.c_str(), &iVar, items))
                    {
                        EditorPin* newPin = 0;
                        if (iVar == 0)
                        {
                            newPin = new EditorFloatPin;
                            newPin->type = EditorPinType::FLOAT;
                        }
                        else if (iVar == 1)
                        {
                            newPin = new EditorFloat2Pin;
                            newPin->type = EditorPinType::FLOAT2;
                        }
                        else if (iVar == 2)
                        {
                            newPin = new EditorFloat3Pin;
                            newPin->type = EditorPinType::FLOAT3;
                        }
                        else if (iVar == 3)
                        {
                            newPin = new EditorFloat4Pin;
                            newPin->type = EditorPinType::FLOAT4;
                        }
                        else if (iVar == 4)
                        {
                            newPin = new EditorIntPin;
                            newPin->type = EditorPinType::INT;
                        }
                        else if (iVar == 5)
                        {
                            newPin = new EditorInt2Pin;
                            newPin->type = EditorPinType::INT2;
                        }
                        else if (iVar == 6)
                        {
                            newPin = new EditorInt3Pin;
                            newPin->type = EditorPinType::INT3;
                        }
                        else if (iVar == 7)
                        {
                            newPin = new EditorInt4Pin;
                            newPin->type = EditorPinType::INT4;
                        }

                        if (newPin)
                        {
                            node->size -= EditorNodeUtil::PinTypeSize(pinIn->type);
                            newPin->pNode = node;
                            newPin->name = pinIn->name;
                            newPin->id = pinIn->id;
                            int id = pinIn->id;
                            DeletePin(pinIn);
                            m_Pins[id] = newPin;
                            pinIn = newPin;
                            node->size += EditorNodeUtil::PinTypeSize(newPin->type);
                            node->pinsOut[0]->size = node->size;
                            auto links = node->pinsOut[0]->connectedLinks;
                            for (auto& link : links)
                                DeleteLink(link->id);
                            UpdatePins();
                            UpdateLinks();
                            needsUpdate = true;
                        }
                    }
                    ImGui::PopStyleColor();

                    // Delete button
                    ImGui::SameLine();
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
                    idStr = ICON_FK_TRASH "##block_var" + std::to_string(pinIn->id);
                    if (ImGui::SmallButton(idStr.c_str()))
                    {
                        node->size -= EditorNodeUtil::PinTypeSize(pinIn->type);
                        node->pinsOut[0]->size = node->size;
                        auto links = node->pinsOut[0]->connectedLinks;
                        for (auto& link : links)
                            DeleteLink(link->id);
                        m_Pins[pinIn->id] = 0;
                        DeletePin(pinIn);
                        node->pinsIn.erase(node->pinsIn.begin() + pinIndex);
                        pinIndex--;
                        UpdatePins();
                        UpdateLinks();
                        needsUpdate = true;
                    }
                    ImGui::PopStyleColor(3);
                    ImGui::PopStyleVar();

                    pinIndex++;
                }
            }

            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 4));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
            isNodeOpened = ImGui::CollapsingHeader("Storage Buffer", ImGuiTreeNodeFlags_SpanAvailWidth);
            ImGui::PopStyleVar(3);
            ImGui::PopStyleColor(3);

            if (isNodeOpened)
            {
                ImGui::Text("\t\tSize");
                ImGui::SameLine(160);
                ImGui::SetNextItemWidth(150);
                int iVal = node->ssboSize;
                if (ImGui::InputInt("##blockBuffer_Size", &iVal, 0))
                {
                    node->ssboSize = iVal;
                    needsUpdate = true;
                }
            }

            if (needsUpdate)
            {
                glBindBuffer(GL_UNIFORM_BUFFER, node->ubo);
                glBufferData(GL_UNIFORM_BUFFER, node->size, NULL, GL_DYNAMIC_DRAW);
                glBindBuffer(GL_UNIFORM_BUFFER, 0);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, node->ssbo);
                glBufferData(GL_SHADER_STORAGE_BUFFER, node->size * node->ssboSize, NULL, GL_DYNAMIC_DRAW);
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
            }
        }

        else if (m_SelectedItemType == SelectedItemType::IMAGE_NODE)
        {
            // Get Selection Id
            int id;
            ImNodes::GetSelectedNodes(&id);
            EditorImageNode* node = (EditorImageNode*)m_Nodes[id];

            bool needsUpdate = false;

            // title bar
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 4));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
            bool isNodeOpened = ImGui::CollapsingHeader("Image Node", ImGuiTreeNodeFlags_SpanAvailWidth);
            ImGui::PopStyleVar(3);
            ImGui::PopStyleColor(3);

            if (isNodeOpened)
            {
                // Size
                int x = node->sizeX, y = node->sizeY;
                ImGui::Text("\t\tWidth");
                ImGui::SameLine(160);
                ImGui::SetNextItemWidth(150);
                if (ImGui::DragInt("##imageSizeX", &x, 1.0f, 0, 4096))
                {
                    if (x < 0) x = 0;
                    if (x > 4096) x = 4096;
                    node->sizeX = x;

                    needsUpdate = true;
                }
                ImGui::Text("\t\tHeight");
                ImGui::SameLine(160);
                ImGui::SetNextItemWidth(150);
                if (ImGui::DragInt("##imageSizeY", &y, 1.0f, 0, 4096))
                {
                    if (y < 0) y = 0;
                    if (y > 4096) y = 4096;
                    node->sizeY = y;

                    needsUpdate = true;
                }
            }

            if (needsUpdate)
            {
                glGenTextures(1, &node->texture);
                glBindTexture(GL_TEXTURE_2D, node->texture);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, node->sizeX, node->sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }

        else if (m_SelectedItemType == SelectedItemType::PINGPONG_NODE)
        {
            // Get Selection Id
            int id;
            ImNodes::GetSelectedNodes(&id);
            EditorPingPongNode* node = (EditorPingPongNode*)m_Nodes[id];

            // title bar
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 4));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
            bool isNodeOpened = ImGui::CollapsingHeader("Ping-Pong Node", ImGuiTreeNodeFlags_SpanAvailWidth);
            ImGui::PopStyleVar(3);
            ImGui::PopStyleColor(3);

            // Buffers type
            ImGui::Text("\t\tBuffers Type");
            ImGui::SameLine(160);
            ImGui::SetNextItemWidth(150);
            int iVal = (int)node->pingpongType;
            const char* items = "Block\0Image\0";
            ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.13f, 0.13f, 0.13f, 1.0f));
            if (ImGui::Combo("##pingpongNodeBuffersType", &iVal, items))
            {
                if (iVal == 0)
                    UpdatePingPongNode(node->id, EditorPingPongNodeType::BUFFER);
                else
                    UpdatePingPongNode(node->id, EditorPingPongNodeType::IMAGE);
            }
            ImGui::PopStyleColor();
        }

        ImGui::EndChild();
    }

    ImGui::End();

    PopImGuiStyles();
}
