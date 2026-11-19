/**
 * @file Editor.cpp
 * @brief Editor implementation.
 */
#ifdef _EDITOR
#include "App/Editor.hpp"

#include <format>
#include <string>

#include <imgui.h>


namespace
{
    constexpr Guid EDITOR_VIEWPORT_TARGET = Guid::FromName("target/editor_viewport");

    /// @brief `size` (the image's) scaled to fit `space` (the window's), keeping its aspect
    ImVec2 Fit(float width, float height, ImVec2 space)
    {
        const float aspect = width / height;
        ImVec2 fitted{ space.x, space.x / aspect };
        if (fitted.y > space.y)
            fitted = { space.y * aspect, space.y };
        return fitted;
    }
}

void Editor::Init(const Game::Scene& scene)
{
    Bridge::ExternalCamera camera{
        .name = "Editor Camera",
        .target = Common::RenderTargetDesc{ .id = EDITOR_VIEWPORT_TARGET, .width = 640, .height = 360 }
    };
    // where the game is seen from, to start with
    for (const Game::Node& node : scene.nodes)
    {
        if (node.camera && node.camera->present)
        {
            camera.transform = node.transform;
            camera.camera = *node.camera;
            m_gameCamera = node.name;
            break;
        }
    }
    // a few metres behind it: the game camera's marker then moves in view instead of through the lens
    constexpr float PULL_BACK_METRES = 4.0f;
    camera.transform.position -= camera.transform.rotation * glm::vec3{ 0.0f, 0.0f, -1.0f } * PULL_BACK_METRES;
    camera.camera.present = false;
    camera.camera.aspect = static_cast<float>(camera.target.width) / static_cast<float>(camera.target.height);
    m_cameras = { camera };
}

void Editor::Draw(EditorContext& context)
{
    // the scene stays visible where no window is docked
    ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

    ImGui::ShowDemoWindow();

    ImGui::SetNextWindowPos({ 20.0f, 20.0f }, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({ 280.0f, 150.0f }, ImGuiCond_FirstUseEver);
    DrawStats(context);

    ImGui::SetNextWindowPos({ 20.0f, 190.0f }, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({ 440.0f, 280.0f }, ImGuiCond_FirstUseEver);
    DrawViewport(context);

    // a game camera, at its own size
    ImGui::SetNextWindowPos({ 480.0f, 190.0f }, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({ 340.0f, 260.0f }, ImGuiCond_FirstUseEver);
    DrawGameView(context);
}

void Editor::DrawStats(const EditorContext& context)
{
    if (ImGui::Begin("Stats"))
    {
        const ImGuiIO& io = ImGui::GetIO();
        ImGui::Text("%.2f ms  %.0f fps", 1000.0f / io.Framerate, io.Framerate);
        // editor mode: the window only shows the editor, the cameras are in their windows
        ImGui::Checkbox("Scene behind the editor", &m_sceneBehindEditor);
        for (const Render::DrawList& view : context.lastFrame.views)
            ImGui::BulletText("%s: %zu draws", view.name.c_str(), view.items.size());
    }
    ImGui::End();
}

void Editor::DrawViewport(const EditorContext& context)
{
    if (ImGui::Begin("Viewport") && !m_cameras.empty())
    {
        Bridge::ExternalCamera& camera = m_cameras.front();
        const ImVec2 space = ImGui::GetContentRegionAvail();
        if (space.x >= 1.0f && space.y >= 1.0f)
        {
            // the camera renders at the window's size in pixels: its target follows the window, its projection the aspect
            const ImVec2 pixelScale = ImGui::GetIO().DisplayFramebufferScale;
            const auto width = static_cast<std::uint32_t>(space.x * pixelScale.x);
            const auto height = static_cast<std::uint32_t>(space.y * pixelScale.y);
            if (camera.target.width != width || camera.target.height != height)
            {
                camera.target.width = width;
                camera.target.height = height;
                camera.camera.aspect = static_cast<float>(width) / static_cast<float>(height);
            }
            ImGui::Image(ImTextureRef(context.targetTexture(camera.target.id)), space);
        }
    }
    ImGui::End();
}

void Editor::DrawGameView(const EditorContext& context)
{
    const Game::Scene& scene = context.scene;
    if (ImGui::Begin("Game"))
    {
        const Game::Node* camera = nullptr;
        if (ImGui::BeginCombo("Camera", m_gameCamera.c_str()))
        {
            for (const Game::Node& node : scene.nodes)
            {
                if (node.camera && ImGui::Selectable(node.name.c_str(), node.name == m_gameCamera))
                    m_gameCamera = node.name;
            }
            ImGui::EndCombo();
        }
        for (const Game::Node& node : scene.nodes)
        {
            if (node.camera && node.name == m_gameCamera)
                camera = &node;
        }

        const Common::RenderTargetDesc* target = camera ? scene.FindTarget(camera->camera->output) : nullptr;
        if (!target || target->width == 0 || target->height == 0)
            ImGui::TextUnformatted("No camera or render target");
        else
            ImGui::Image(ImTextureRef(context.targetTexture(target->id)),
                         Fit(static_cast<float>(target->width), static_cast<float>(target->height),
                             ImGui::GetContentRegionAvail()));
    }
    ImGui::End();
}
#endif
