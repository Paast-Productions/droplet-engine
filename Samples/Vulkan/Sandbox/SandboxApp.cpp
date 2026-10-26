#include "SandboxApp.hpp"
#include <filesystem>
#include <print>

namespace fs = std::filesystem;

using namespace Droplet::Sandbox;
using namespace Droplet;

SandboxApp::SandboxApp(SandboxAppConfig cfg)
    : m_config(std::move(cfg)),
      m_sdl(m_config.wnd),
      m_renderer(m_sdl.GetWindow())
{
    // window callback
    m_sdl.SetResizeCallback([this]() { m_renderer.OnResize(); });
    m_sdl.SetMinimizeCallback([this]() { m_renderer.OnMinimize(); });
    m_sdl.SetRestoreCallback([this]() { m_renderer.OnRestore(); });

    // resource man
    const fs::path rootPath = fs::current_path() / "../../../Assets";
    ResourceManager::GetInstance().Initialize(rootPath);
}

void SandboxApp::Run()
{
    Init();
    MainLoop();
    Cleanup();
}

void SandboxApp::Init()
{
    LoadResources();
    InitVulkan();
}

void SandboxApp::LoadResources()
{
    constexpr std::string_view relative = "../../../Assets";
    // initialize resource manager and register mesh    
    const fs::path rootPath = fs::current_path() / relative;
    auto rscMan = &ResourceManager::GetInstance();

    const fs::path gupta{"gupta.glb"};
    rscMan->RegisterAsset(gupta);

    auto rsc = rscMan->GetRegisteredResources();

    for (const auto entry : rsc)
    {
        std::println("{}", entry->sourceName.data());
    }
}

void SandboxApp::InitVulkan()
{
    SandboxShaderCompiler shaderCompiler;

    const char* shaderPath = "base_shader_wvp.slang";

    const auto vertexSpirv = shaderCompiler.CompileSpirv(
        "base",
        shaderPath,
        "vMain");

    const auto fragmentSpirv = shaderCompiler.CompileSpirv(
        "base",
        shaderPath,
        "fMain");

    m_renderer.CreateShaderModules(vertexSpirv, fragmentSpirv);
    m_renderer.CreateGraphicsPipeline();
}

void SandboxApp::MainLoop()
{
    while (m_running)
    {
        m_sdl.PollEvents(&m_running);
        if (!m_running)
            break;

        // TODO: create some situations that require synchronization

        // TODO: draw
        m_renderer.DrawFrame();
    }

    m_renderer.WaitIdle();
}

void SandboxApp::Cleanup()
{
}

SandboxTimelineSemaphoreApp::SandboxTimelineSemaphoreApp(SandboxAppConfig cfg)
    : m_config(std::move(cfg)),
      m_sdl(m_config.wnd),
      m_renderer(m_sdl.GetWindow())
{
    // window callback
    m_sdl.SetResizeCallback([this]() { m_renderer.OnResize(); });
    m_sdl.SetMinimizeCallback([this]() { m_renderer.OnMinimize(); });
    m_sdl.SetRestoreCallback([this]() { m_renderer.OnRestore(); });
}

void SandboxTimelineSemaphoreApp::Run()
{
    Init();
    MainLoop();
    Cleanup();
}

void SandboxTimelineSemaphoreApp::Init()
{
}

void SandboxTimelineSemaphoreApp::LoadResources()
{
}

void SandboxTimelineSemaphoreApp::MainLoop()
{
}

void SandboxTimelineSemaphoreApp::Cleanup()
{
}

void SandboxTimelineSemaphoreApp::InitShaders()
{
    static constexpr std::array<const char*, 5> shaders = {
        "game_of_life_init.comp.slang",
        "game_of_life_mutate.comp.slang",
        "game_of_life_update.comp.slang",
    };

    for (const auto& name : shaders)
    {
        const auto bytes = m_shaderCompiler.CompileSpirv(
            name, name, "main");
        
        m_shaderModules.emplace_back(m_renderer.CreateShaderModule(bytes));
    }
    
    {
        const auto bytes = m_shaderCompiler.CompileSpirv(name, name, "main");
    }

}
