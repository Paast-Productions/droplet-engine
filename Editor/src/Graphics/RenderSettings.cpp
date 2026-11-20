#include "Graphics/RenderSettings.hpp"

#include <IconFontCppHeaders/IconsLucide.h>

#include <ImGui/imgui.h>
#include <ImGui/imgui_stdlib.h>
#include <ShaderOverride.hpp>

using namespace Droplet::Editor::Graphics;

float RenderSettings::m_clearColor[3] = {0,0,0};

RenderSettings::~RenderSettings()
{

}

void RenderSettings::InitImpl()
{
	SetName("Render Settings");
	ToggleToolbar(false);
}

void RenderSettings::RenderImpl()
{
    using namespace ImGui;

    //Get current scene
    std::shared_ptr<Droplet::Scene::Scene> scene = m_editorContext->GetActiveScene().lock();
    Droplet::Editor::Scene::EditorCamera &editorCamera = m_editorContext->GetEditorCamera();

    SeparatorText("Shader Overrides");

    VertexFlag vFlag = ShaderOverride::GetVertexFlag();
    FragmentFlag fFlag = ShaderOverride::GetFragmentFlag();

    SeparatorText("Vertex Shader");

    PushID(0);
    for (int i = 0; i < IM_ARRAYSIZE(m_vertexOptions); i++) {
        RadioButton(m_fragmentOptions[i].c_str(), (int *)&vFlag, i);
    }
    PopID();

    SeparatorText("Fragment Shader");

    PushID(1);
    for (int i = 0; i < IM_ARRAYSIZE(m_fragmentOptions); i++) {
        RadioButton(m_fragmentOptions[i].c_str(), (int *)&fFlag, i);
    }
    PopID();

    ShaderOverride::SetVertexFlag(vFlag);
    ShaderOverride::SetFragmentFlag(fFlag);

    SeparatorText("Clear Color");

    ImGuiColorEditFlags colorEdiFlags =
        ImGuiColorEditFlags_NoSmallPreview |
        ImGuiColorEditFlags_NoTooltip |
        ImGuiColorEditFlags_NoLabel |
        ImGuiColorEditFlags_NoSidePreview |
        ImGuiColorEditFlags_NoAlpha |
        ImGuiColorEditFlags_PickerHueBar;

    glm::vec4 color = editorCamera.GetClearColor();

    m_clearColor[0] = color.r;
    m_clearColor[1] = color.g;
    m_clearColor[2] = color.b;

    ColorPicker3("Background Color", m_clearColor, colorEdiFlags);
    SameLine();

    PushID(0);
    if (Button("Reset")) {
        m_clearColor[0] = 0.0f;
        m_clearColor[1] = 0.0f;
        m_clearColor[2] = 0.0f;
    }
    PopID();

    SeparatorText("Field of View");

    float fov = editorCamera.GetFOV();

    SliderFloat("##", &fov, 20.0f, 120.0f);
    SameLine();

    PushID(1);
    if (Button("Reset")) {
        fov = 75.0f;
    }
    PopID();

    editorCamera.SetFOV(fov);
    editorCamera.SetClearColor3(m_clearColor[0], m_clearColor[1], m_clearColor[2]);
}

void RenderSettings::RenderToolbar()
{

}