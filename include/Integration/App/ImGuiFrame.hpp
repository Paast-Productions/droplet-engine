/**
 * @file ImGuiFrame.hpp
 * @brief ImGuiFrame: the engine side of ImGui (context, SDL3 platform backend, the frame). Editor builds only.
 */
#pragma once
#ifdef _EDITOR

#include <SDL3/SDL.h>

struct ImDrawData;

namespace Vulkan
{
    class Renderer;
}

/// @brief Owns the ImGui context and its frame; the renderer only draws what End() returns.
/// @details Belongs where production's Engine sits, between the renderer and the editor: the editor builds its windows
///          between Begin() and End(). Create it before the renderer's InitImGui and destroy it after the renderer.
class ImGuiFrame
{
public:
    /// @brief Creates the context (docking on) and the SDL3 platform backend for `window`
    explicit ImGuiFrame(SDL_Window* window);
    ~ImGuiFrame();

    // no copy no move
    ImGuiFrame(const ImGuiFrame&) = delete;
    ImGuiFrame& operator=(const ImGuiFrame&) = delete;
    ImGuiFrame(ImGuiFrame&&) = delete;
    ImGuiFrame& operator=(ImGuiFrame&&) = delete;

    /// @brief Hands an SDL event to ImGui
    void ProcessEvent(const SDL_Event& event);
    /// @brief ImGui uses the mouse this frame (over a window): the game should ignore it
    [[nodiscard]] bool WantsMouse() const;
    /// @brief ImGui uses the keyboard this frame (a text field has focus): the game should ignore it
    [[nodiscard]] bool WantsKeyboard() const;

    /// @brief Starts the frame: the renderer's backend, the platform backend, ImGui::NewFrame and the dock space
    void Begin(Vulkan::Renderer& renderer);
    /// @brief Ends the frame; what to pass to Renderer::Render. Valid until the next Begin()
    [[nodiscard]] ImDrawData* End();
};

#endif
