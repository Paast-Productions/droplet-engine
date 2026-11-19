/**
 * @file ImGuiFrame.cpp
 * @brief ImGuiFrame implementation
 */
#ifdef _EDITOR
#include "App/ImGuiFrame.hpp"

#include <stdexcept>

#include <imgui.h>
#include <imgui_impl_sdl3.h>

#include "Vulkan/Frame/Renderer.hpp"

ImGuiFrame::ImGuiFrame(SDL_Window* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    if (!ImGui_ImplSDL3_InitForVulkan(window))
        throw std::runtime_error("ImGuiFrame: ImGui_ImplSDL3_InitForVulkan failed");
}

ImGuiFrame::~ImGuiFrame()
{
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiFrame::ProcessEvent(const SDL_Event& event)
{
    ImGui_ImplSDL3_ProcessEvent(&event);
}

bool ImGuiFrame::WantsMouse() const
{
    return ImGui::GetIO().WantCaptureMouse;
}

bool ImGuiFrame::WantsKeyboard() const
{
    return ImGui::GetIO().WantCaptureKeyboard;
}

void ImGuiFrame::Begin(Vulkan::Renderer& renderer)
{
    renderer.NewImGuiFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    // the scene stays visible where no window is docked
    ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);
}

ImDrawData* ImGuiFrame::End()
{
    ImGui::Render();
    return ImGui::GetDrawData();
}
#endif
