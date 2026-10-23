#pragma once

#include "Sandbox.hpp"
#include <resource/ResourceManager.hpp>
#include <Graphics/VK/Camera.hpp>

namespace Droplet::Sandbox
{
    struct SandboxAppConfig
    {
        SandboxSDL::Config wnd{};
    };

    class SandboxApp
    {
    public:
        SandboxApp(SandboxAppConfig cfg);

        void Run();

    private:
        void Init();
        void LoadResources();
        void InitVulkan();

        void MainLoop();

        void Cleanup();

    private:
        SandboxAppConfig m_config;
        SandboxSDL m_sdl;
        SandboxRenderer m_renderer;
        
        // camera
        Graphics::Camera m_camera;
        
        bool m_running = true;
    };
}
