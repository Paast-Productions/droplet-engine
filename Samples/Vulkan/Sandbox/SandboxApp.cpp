#include "SandboxApp.hpp"
#include <filesystem>
#include <print>

namespace fs = std::filesystem;

using namespace Droplet::Sandbox;
using namespace Droplet;

SandboxApp::SandboxApp(SandboxAppConfig cfg) :
    m_config(std::move(cfg)),
    m_sdl(m_config.wnd),
    m_renderer(m_sdl.GetWindow()),
    m_camera({0.0f, 0.0f, -5.0f})
{
    const std::filesystem::path rootPath = std::filesystem::current_path() / "../../../Assets";

    ResourceManager::GetInstance().Initialize(rootPath);
    
    m_sdl.SetResizeCallback([this]() { m_renderer.OnResize(); });
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
    const std::filesystem::path rootPath = fs::current_path() / relative;
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
    
    const char * shaderPath = "triangle_gradient.slang";

    const auto vertexSpirv = shaderCompiler.CompileSpirv(
        "triangle",
        shaderPath,
        "vertexMain");

    const auto fragmentSpirv = shaderCompiler.CompileSpirv(
        "triangle",
        shaderPath,
        "fragmentMain");

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
