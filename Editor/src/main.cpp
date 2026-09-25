#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_vulkan.h>
#include <ImGui/imgui_impl_sdl3.h>
#include <SDL3/SDL.h>
#include <cstdio>

static void check_vk_result(VkResult err)
{
	if (err == 0)
		return;

	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);

	if (err < 0)
		abort();
}

static SDL_Window *InitSDL()
{
	SDL_WindowFlags windowFlags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE);

	SDL_Window *wnd = SDL_CreateWindow("Droplet Editor", 1280, 720, windowFlags);
	if (!wnd)
	{
		SDL_Log("Failed to create window: %s", SDL_GetError());
		return nullptr;
	}

	return wnd;
}

static void InitImGui(SDL_Window *window)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Docking Branch

	// Setup Platform/Renderer backends
	ImGui_ImplSDL3_InitForVulkan(window);

	ImGui_ImplVulkan_InitInfo init_info = {};
	// TODO
	/*init_info.Instance = YOUR_INSTANCE;
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
	init_info.CheckVkResultFn = check_vk_result;*/

	ImGui_ImplVulkan_Init(&init_info);
}

static void NewFrame()
{
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
}

static void SubmitFrame(SDL_Window *window)
{
	ImGui::Render();
	SDL_RenderPresent(SDL_GetRenderer(window));
}

static void DrawFrame([[maybe_unused]] SDL_Window *window)
{
	// Create docking space over the entire window
	ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
	SDL_Window *wnd = InitSDL();
	if (!wnd)
	{
		return 1;
	}

	InitImGui(wnd);

	// Run main loop
	bool done = false;
	while (!done)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL3_ProcessEvent(&event);
			if (event.type == SDL_EVENT_QUIT)
				done = true;
		}

		NewFrame();

		DrawFrame(wnd);

		SubmitFrame(wnd);
	}
}