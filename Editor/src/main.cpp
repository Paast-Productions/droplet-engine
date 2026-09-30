#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_vulkan.h>
#include <ImGui/imgui_impl_sdl3.h>
#include <SDL3/SDL.h>
#include <cstdio>
#include <Graphics/VK/Renderer.hpp>

class DropletInstance; // TODO: Get definition from Droplet Engine

Droplet::Graphics::SDL::WindowConfig config =
{
	.Width = 640,
	.Height = 400
};
Renderer rend(config);

// Initialization
[[nodiscard]] static DropletInstance *Soak()
{
	// TODO: Init Droplet Engine
	return nullptr;
}

static void DryOff([[maybe_unused]] DropletInstance *instance)
{
	// TODO: Close Droplet Engine
}

static void InitImGui([[maybe_unused]] DropletInstance *instance, [[maybe_unused]] SDL_Window *window)
{
	// TODO
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Docking Branch

	// Setup Platform/Renderer backends
	ImGui_ImplSDL3_InitForVulkan(window);

	ImGui_ImplVulkan_InitInfo init_info = rend.GetImGuiInitInfo();

	ImGui_ImplVulkan_Init(&init_info);


	// TODO: Hook into engine's SDL_PollEvent() loop to call ImGui_ImplSDL3_ProcessEvent() for each event
}

// Frame
static void NewFrame()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
}

static void SubmitFrame(SDL_Window *window)
{
	ImGui::Render();
	SDL_RenderPresent(SDL_GetRenderer(window));
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
	DropletInstance *instance = Soak();

	rend.Initialize();

	SDL_Window *wnd = rend.GetWindow();
	if (!wnd)
	{
		return 1;
	}

	InitImGui(instance, wnd);

	// Run main loop
	bool done = false;
	while (!done)
	{
		// TODO: Update engine

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL3_ProcessEvent(&event);
			if (event.type == SDL_EVENT_QUIT)
				done = true;
		}

		NewFrame();

		rend.drawFrame();
		ImGui::ShowDemoWindow();
		ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

		SubmitFrame(wnd);
	}

	rend.WaitIdle();
	ImGui_ImplVulkan_Shutdown();

	DryOff(instance);
}