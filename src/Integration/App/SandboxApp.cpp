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
#ifdef _EDITOR
    m_editor.Init(m_scene);
#endif
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
#ifdef _EDITOR
        const Render::RenderWorld world = Bridge::Extract(m_scene, m_editor.Cameras()); // and the editor's camera
#else
        const Render::RenderWorld world = Bridge::Extract(m_scene);
#endif
        if (!m_worldDescribed)
        {
            Bridge::Describe(world, m_scene);
            m_worldDescribed = true;
        }

        // 2. the assets the snapshot refers to that the GPU lacks or holds an out-of-date copy of, from the asset storage
        //    to the GPU: separate from rendering, so it can move to a thread pool
#ifdef _EDITOR
        // and the textures the editor's windows showed last frame
        const Assets::RequiredAssets required = Bridge::RequiredAssetsOf(world, m_renderer.ImGuiImages());
#else
        const Assets::RequiredAssets required = Bridge::RequiredAssetsOf(world);
#endif
        m_renderer.Upload(Bridge::ProvideAssets(m_assets, required, m_renderer.Resident()));

        // 3. the editor's windows (ImGui), drawn on top of the frame
        ImDrawData* overlay = nullptr;
#ifdef _EDITOR
        m_imgui.Begin(m_renderer);
        EditorContext editorContext{
            .scene = m_scene,
            .targetTexture = [this](Guid target) { return m_renderer.ImGuiTexture(target); },
            .lastFrame = m_renderer.Prepared()
        };
        m_editor.Draw(editorContext);
        overlay = m_imgui.End();
        m_renderer.SetCompositeScene(m_editor.SceneBehindEditor());
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
