#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_vulkan.h>
#include <ImGui/imgui_impl_sdl3.h>
#include <SDL3/SDL.h>
#include <cstdio>
#include <Engine/Engine.hpp>
#include "HierarchyWindow.hpp"
#include <NodeInspectorWindow.hpp>

class DropletInstance; // TODO: Get definition from Droplet Engine

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

[[nodiscard]] static SDL_Window *InitSDL([[maybe_unused]] DropletInstance *instance)
{
	// TODO: Get window from Engine

	return nullptr;
}

//static void check_vk_result(VkResult err)
//{
//	if (err == 0)
//		return;
//
//	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
//
//	if (err < 0)
//		abort();
//}

static void InitImGui([[maybe_unused]] DropletInstance *instance, [[maybe_unused]] SDL_Window *window)
{
	// TODO
	/*IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Docking Branch

	// Setup Platform/Renderer backends
	ImGui_ImplSDL3_InitForVulkan(window);

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = YOUR_INSTANCE;
	init_info.PhysicalDevice = YOUR_PHYSICAL_DEVICE;
	init_info.Device = YOUR_DEVICE;
	init_info.QueueFamily = YOUR_QUEUE_FAMILY;
	init_info.Queue = YOUR_QUEUE;
	init_info.PipelineCache = YOUR_PIPELINE_CACHE;
	init_info.DescriptorPool = YOUR_DESCRIPTOR_POOL;
	init_info.MinImageCount = 2;
	init_info.ImageCount = 2;
	init_info.Allocator = YOUR_ALLOCATOR;
	init_info.PipelineInfoMain.RenderPass = wd->RenderPass;
	init_info.PipelineInfoMain.Subpass = 0;
	init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.CheckVkResultFn = check_vk_result;

	ImGui_ImplVulkan_Init(&init_info);*/

	// TODO: Hook into engine's SDL_PollEvent() loop to call ImGui_ImplSDL3_ProcessEvent() for each event
}

// Frame
//static void NewFrame()
//{
//	ImGui_ImplSDL3_NewFrame();
//	ImGui::NewFrame();
//}

//static void SubmitFrame(SDL_Window *window)
//{
//	ImGui::Render();
//	SDL_RenderPresent(SDL_GetRenderer(window));
//}

//static void DrawFrame([[maybe_unused]] DropletInstance *instance, [[maybe_unused]] SDL_Window *window)
//{
//	// Create docking space over the entire window
//	ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
//}


int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
	Droplet::Engine engine({
		Droplet::Graphics::SDL::WindowConfig {
			640, 480, {}
		}
		});

	SDL_Window *wnd = engine.GetWindow().Get();
	if (!wnd)
	{
		return 1;
	}

	//InitImGui(instance, wnd);

	// Run main loop
	while (engine.Update() == Droplet::DROPLET_RETURNTYPE::OK)
	{
		//NewFrame();
		//SubmitFrame(wnd);
	}

	// I do not know but i needed to have them because warnings = errors :(
	DropletInstance *instance = Soak();
	SDL_Window *notRealWnd = InitSDL(instance);
	InitImGui(instance, notRealWnd);
	//DrawFrame(instance, notRealWnd);
	DryOff(instance);

	return 0;
}
