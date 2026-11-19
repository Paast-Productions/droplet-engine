/**
 * @file SandboxApp.hpp
 * @brief SandboxApp class declaration
 * @author Sebastian Lorensson
 */
#pragma once

#include <cstddef>
#include <string_view>

#include "Integration/Assets/Assets.hpp"
#include "Integration/Vulkan/Frame/Renderer.hpp"
#include "Integration/Platform/SDLVulkan.hpp"
#include "Integration/App/ImGuiFrame.hpp"
#include "Integration/Platform/Window.hpp"
#include "Integration/Game/Scene.hpp"

/**
* @brief Configuration struct for launching a Sandbox application.
*/
struct SandboxAppConfig
{
    SDL::Window::Config wnd{};
};

/**
 * @brief A complete sandbox application that creates a window, a scene, then renders the scene through its renderer.
 */
class SandboxApp
{
public:
    explicit SandboxApp(SandboxAppConfig cfg);
    ~SandboxApp() = default; // Vulkan is all raii so it cleans itself up

    // no copy no move
    SandboxApp(const SandboxApp&) = delete;
    SandboxApp &operator=(const SandboxApp&) = delete;
    SandboxApp(SandboxApp&&) = delete;
    SandboxApp &operator=(SandboxApp&&) = delete;

    /// @brief Application will run until window is closed or an error occurs.
    void Run();

private:
    
    /// @brief Creates resources and makes the scene
    void Init();
    
    /// @brief Runs the main loop until m_running is false
    void MainLoop();

    /// @brief Advances the game state (the spoofed camera animation) by `dt` seconds.
    void Update(float dt);

    /// @brief Switches effect `effectIndex` of `cameraName`'s PostProcessComponent on or off.
    void ToggleEffect(std::string_view cameraName, std::size_t effectIndex);


private:
    SandboxAppConfig m_config;                    ///< Config saved at construction
    SDL::Window m_window;                         ///< SDL Window abstraction
    SDL::VulkanSurfaceProvider m_surfaceProvider; ///< renderer's view of the window. platform-agnostic

    Assets::AssetStorage m_assets; ///< fake cpu resource manager
    Game::Scene m_scene;           ///< fake scene

    /// Everything on the GPU side. Declared last, so it is destroyed first.
#ifdef _EDITOR
    ImGuiFrame m_imgui; ///< the ImGui context and frame; outlives the renderer's ImGui backend
    Editor m_editor;    ///< the editor's windows and its viewport camera
#endif
    Vulkan::Renderer m_renderer; ///< Vulkan rendering, rsc management, graph building, frame construction, command execution and presentation

    // -- runtime variables --
    bool m_running = true; ///< set to false if SDL event is close or Renderer calls exit
    bool m_worldDescribed = false; ///< the first extracted world is printed once
};
