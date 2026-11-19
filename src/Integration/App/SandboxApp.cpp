/**
 * @file SandboxApp.cpp
 * @brief SandboxApp class implementation
 * @author Sebastian Lorensson
 * @details SandboxApp is the application layer responsible for managing the state of the whole application.
 */
#include "Integration/App/SandboxApp.hpp"

#include <cstdint>
#include <print>

#include "Bridge/AssetProvider.hpp"
#include "Bridge/Extract.hpp"
#include "Core/Time.hpp"   // Delta timing
#include "Spoof/Spoof.hpp" // Creates a Scene from scratch

#ifdef _EDITOR
#include <string>

#include <imgui.h>
#endif

SandboxApp::SandboxApp(SandboxAppConfig cfg)
    : m_config(cfg),
    m_window(m_config.wnd),
    m_surfaceProvider(m_window.GetSDLWindow()),
#ifdef _EDITOR
    m_imgui(m_window.GetSDLWindow()),
#endif
    m_renderer(m_surfaceProvider)
{
#ifdef _EDITOR
    // ImGui sees every event first; the game ignores the keyboard while ImGui has it
    m_renderer.InitImGui();
    m_window.SetEventCallback([this](const SDL_Event& event) { m_imgui.ProcessEvent(event); });
#endif
    // window callback functions
    m_window.SetResizeCallback([this]() { m_renderer.OnResize(); });
    m_window.SetMinimizeCallback([this]() { m_renderer.OnMinimize(); });
    m_window.SetRestoreCallback([this]() { m_renderer.OnRestore(); });
    m_window.SetKeyDownCallback([this](SDL_Keycode key)
        {
#ifdef _EDITOR
            if (m_imgui.WantsKeyboard())
                return;
#endif
            // toggles different camera effects mapped to different cameras
            switch (key)
            {
            case SDLK_1: ToggleEffect(Spoof::CameraMainName, 0); break;
            case SDLK_2: ToggleEffect(Spoof::CameraMainName, 1); break;
            case SDLK_3: ToggleEffect(Spoof::CameraMainName, 2); break;
            case SDLK_4: ToggleEffect(Spoof::CameraMainName, 3); break;
            case SDLK_5: ToggleEffect(Spoof::CameraMainName, 4); break;
            case SDLK_6: ToggleEffect(Spoof::CameraAName, 0); break;
            case SDLK_7: ToggleEffect(Spoof::CameraCName, 0); break;

            default: break;
            }
        });
}

void SandboxApp::Run()
{
    Init();
    MainLoop();
}

void SandboxApp::Init()
{
    m_assets = Spoof::MakeResources();
    m_scene = Spoof::MakeScene(m_assets);
}

void SandboxApp::MainLoop()
{
    Timer time;

    while (m_running)
    {
        m_window.PollEvents(&m_running);
        if (!m_running)
            break;

        // nothing is drawn while the window is minimized. We wait for a maximize or close event
        if (m_renderer.IsMinimized())
        {
            m_window.WaitForEvent();
            continue;
        }

        // time gets clamped by timer so tick isn't huge in case of lag spikes or un-minimized window
        const float dt = static_cast<float>(time.tick());

        Update(dt);

        // 1. the scene as a snapshot, by GUID
        const Render::RenderWorld world = Bridge::Extract(m_scene);
        if (!m_worldDescribed)
        {
            Bridge::Describe(world, m_scene);
            m_worldDescribed = true;
        }

        // 2. the assets the snapshot refers to that the GPU lacks or holds an out-of-date copy of, from the asset storage
        //    to the GPU: separate from rendering, so it can move to a thread pool
        m_renderer.Upload(Bridge::ProvideAssets(m_assets, Bridge::RequiredAssetsOf(world), m_renderer.Resident()));

        // 3. the editor's windows (ImGui), drawn on top of the frame
        ImDrawData* overlay = nullptr;
#ifdef _EDITOR
        m_imgui.Begin(m_renderer);
        DrawEditor();
        overlay = m_imgui.End();
        m_renderer.SetCompositeScene(m_sceneBehindEditor);
#endif

        // 4. draw the snapshot
        m_renderer.Render(world, overlay);
    }
}

void SandboxApp::ToggleEffect(std::string_view cameraName, std::size_t effectIndex)
{
    Game::Node* camera = m_scene.FindNode(cameraName);
    if (!camera || !camera->postProcess || effectIndex >= camera->postProcess->effects.size())
        return;

    Game::PostProcessComponent::Effect& effect = camera->postProcess->effects[effectIndex];
    effect.enabled = !effect.enabled;
    std::println("{}: {} {}", cameraName, Common::ToString(effect.effect), effect.enabled ? "on" : "off");
}

void SandboxApp::Update(float dt)
{
    static float elapsed = 0.0f;
    elapsed += dt;
    m_scene.Animate(elapsed);
}

#ifdef _EDITOR
void SandboxApp::DrawEditor()
{
    ImGui::ShowDemoWindow();

    ImGui::SetNextWindowPos({20.0f, 20.0f}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({280.0f, 150.0f}, ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Stats"))
    {
        const ImGuiIO& io = ImGui::GetIO();
        ImGui::Text("%.2f ms  %.0f fps", 1000.0f / io.Framerate, io.Framerate);
        // editor mode: the window only shows the editor, the cameras are in their viewports
        ImGui::Checkbox("Scene behind the editor", &m_sceneBehindEditor);
        for (const Render::DrawList& view : m_renderer.Prepared().views)
            ImGui::BulletText("%s: %zu draws", view.name.c_str(), view.items.size());
    }
    ImGui::End();

    // cameras recording to viewports: each window shows a camera's render target
    ImGui::SetNextWindowPos({20.0f, 190.0f}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({440.0f, 280.0f}, ImGuiCond_FirstUseEver);
    DrawViewport(Spoof::CameraMainName);
    ImGui::SetNextWindowPos({480.0f, 190.0f}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({300.0f, 330.0f}, ImGuiCond_FirstUseEver);
    DrawViewport(Spoof::CameraAName);
}

void SandboxApp::DrawViewport(std::string_view cameraName)
{
    const std::string title = std::format("Viewport: {}", cameraName);
    if (ImGui::Begin(title.c_str()))
    {
        Game::Node* camera = m_scene.FindNode(cameraName);
        Common::RenderTargetDesc* target = camera && camera->camera ? m_scene.FindTarget(camera->camera->output) : nullptr;
        const ImVec2 space = ImGui::GetContentRegionAvail();
        if (!target)
        {
            ImGui::TextUnformatted("No camera or render target");
        }
        else if (space.x >= 1.0f && space.y >= 1.0f)
        {
            // the camera renders at the window's size: the target follows the window, the projection its aspect. The
            // renderer replaces the target when it sees the new size, from the next frame's world
            ImVec2 pixelScale = ImGui::GetIO().DisplayFramebufferScale;
            const auto width = static_cast<std::uint32_t>(space.x * pixelScale.x);
            const auto height = static_cast<std::uint32_t>(space.y * pixelScale.y);
            if (target->width != width || target->height != height)
            {
                target->width = width;
                target->height = height;
                camera->camera->aspect = static_cast<float>(width) / static_cast<float>(height);
            }
            ImGui::Image(ImTextureRef(m_renderer.ImGuiTexture(target->id)), space);
        }
    }
    ImGui::End();
}
#endif
