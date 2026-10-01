#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_vulkan.h>
#include <ImGui/imgui_impl_sdl3.h>
#include <SDL3/SDL.h>
#include <cstdio>

#include <tracy/public/tracy/Tracy.hpp>
//#include <tracy/public/tracy/TracyVulkan.hpp>
//#include <tracy/public/tracy/TracyLua.hpp>

class DropletInstance; // TODO: Get definition from Droplet Engine

// Initialization
[[nodiscard]] static DropletInstance *Soak()
{
	ZoneScoped;

	// TODO: Init Droplet Engine
	return nullptr;
}

static void DryOff([[maybe_unused]] DropletInstance *instance)
{
	ZoneScoped;

	// TODO: Close Droplet Engine
}

[[maybe_unused]] [[nodiscard]] static SDL_Window *InitSDL([[maybe_unused]] DropletInstance *instance)
{
	ZoneScoped;

	// TODO: Get window from Engine

	return nullptr;
}

/*static void check_vk_result(VkResult err)
{
	if (err == 0)
		return;

	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);

	if (err < 0)
		abort();
}*/

[[maybe_unused]] static void InitImGui([[maybe_unused]] DropletInstance *instance, [[maybe_unused]] SDL_Window *window)
{
	ZoneScoped;

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
[[maybe_unused]] static void NewFrame()
{
	ZoneScoped;

	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
}

[[maybe_unused]] static void SubmitFrame(SDL_Window *window)
{
	ZoneScoped;

	ImGui::Render();
	SDL_RenderPresent(SDL_GetRenderer(window));
}

[[maybe_unused]] static void DrawFrame([[maybe_unused]] DropletInstance *instance, [[maybe_unused]] SDL_Window *window)
{
	ZoneScoped;

	// Create docking space over the entire window
	ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
}


int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
	// TODO: Check if Tracy is enabled and if so, sleep for a few seconds to allow the profiler to connect before starting the engine

	ZoneScopedN("Editor");

	[[maybe_unused]] DropletInstance *instance = Soak();

	//SDL_Window *wnd = InitSDL(instance);
	//if (!wnd)
	//{
	//	return 1;
	//}

	//InitImGui(instance, wnd);

	FrameMark;

	// Run main loop
	bool done = false;
	while (!done)
	{
		ZoneScopedN("Main Loop");

		// TODO: Update engine

		/*SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL3_ProcessEvent(&event);
			if (event.type == SDL_EVENT_QUIT)
				done = true;
		}*/

		//NewFrame();

		//DrawFrame(instance, wnd);

		//SubmitFrame(wnd);

		FrameMark;
	}

	DryOff(instance);
}