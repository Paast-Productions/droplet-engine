/**
 * @file SandboxApp.cpp
 * @brief SandboxApp class implementation
 * @author Sebastian Lorensson
 * @details SandboxApp is the application layer responsible for managing the state of the whole application.
 */
#include "Integration/App/SandboxApp.hpp"

#include <cstdint>
#include <print>

#include "Integration/Bridge/AssetProvider.hpp"
#include "Integration/Bridge/Extract.hpp"
#include "Integration/Core/Time.hpp"   // Delta timing
#include "Integration/Spoof/Spoof.hpp" // Creates a Scene from scratch

SandboxApp::SandboxApp(SandboxAppConfig cfg)
    : m_config(cfg),
    m_window(m_config.wnd),
    m_surfaceProvider(m_window.GetSDLWindow()),
    m_renderer(m_surfaceProvider)
{
    // window callback functions
    m_window.SetResizeCallback([this]() { m_renderer.OnResize(); });
    m_window.SetMinimizeCallback([this]() { m_renderer.OnMinimize(); });
    m_window.SetRestoreCallback([this]() { m_renderer.OnRestore(); });
    m_window.SetKeyDownCallback([this](SDL_Keycode key)
        {
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

        // 3. draw the snapshot
        m_renderer.Render(world);
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
