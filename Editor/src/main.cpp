#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_vulkan.h>
#include <ImGui/imgui_impl_sdl3.h>
#include <SDL3/SDL.h>
#include <cstdio>
#include <Graphics/VK/Renderer.hpp>
#include <GameInput.hpp>


#include <tracy/public/tracy/Tracy.hpp>
//#include <tracy/public/tracy/TracyVulkan.hpp>
//#include <tracy/public/tracy/TracyLua.hpp>

class DropletInstance; // TODO: Get definition from Droplet Engine

Droplet::Graphics::SDL::WindowConfig config =
{
	.Width = 1280,
	.Height = 720,
	.Flags = 0
};
 
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

[[maybe_unused]] static void InitImGui([[maybe_unused]] SDL_Window *window)
{
	ZoneScoped;

	// TODO
	

	// Setup Platform/Renderer backends
	ImGui_ImplSDL3_InitForVulkan(window);

	//ImGui_ImplVulkan_InitInfo init_info = g_rend.GetImGuiInitInfo();

	//ImGui_ImplVulkan_Init(&init_info);


	// TODO: Hook into engine's SDL_PollEvent() loop to call ImGui_ImplSDL3_ProcessEvent() for each event
}

// Frame
[[maybe_unused]] static void NewFrame()
{
	ZoneScoped;
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
}

[[maybe_unused]] static void SubmitFrame(SDL_Window *window)
{
	ZoneScoped;

	ImGui::EndFrame();
	SDL_RenderPresent(SDL_GetRenderer(window));
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
	// TODO: Check if Tracy is enabled and if so, sleep for a few seconds to allow the profiler to connect before starting the engine

	ZoneScopedN("Editor");
	bool show_demo_window = true;
	bool show_another_window = true;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	DropletInstance *instance = Soak();

	Droplet::Graphics::Renderer rend(config);

	SDL_Window *wnd = rend.GetWindow();
	if (!wnd)
	{
		return 1;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Docking Branch
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    
	ImGui_ImplSDL3_InitForVulkan(wnd);

	ImGui_ImplVulkan_InitInfo init_info = {};
	rend.GetImGuiInitInfo(init_info);
	ImGui_ImplVulkan_Init(&init_info);

	//InitImGui(wnd);

	// Run main loop
	bool done = false;
	while (!done)
	{
		ZoneScopedN("Main Loop");

		// TODO: Update engine

		Droplet::GameInput::Get().Update();

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL3_ProcessEvent(&event);
			if (event.type == SDL_EVENT_QUIT)
			{
				done = true;
			}

			Droplet::GameInput::Get().ProcessEvent(event);
		}

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		if (show_demo_window)
		{
			ImGui::ShowDemoWindow(&show_demo_window);	
		}

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
		{
			static float s_f = 0.0f;
			static int s_counter = 0;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("float", &s_f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", reinterpret_cast<float *>(&clear_color)); // Edit 3 floats representing a color

			if (ImGui::Button("Button"))  // Buttons return true when clicked (most widgets return true when edited/activated)
			{
				s_counter++;	
			}
			
			ImGui::SameLine();
			ImGui::Text("counter = %d", s_counter);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			ImGui::End();
		}
		

		if (show_another_window)
		{
			ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
			{
				show_another_window = false;	
			}
			ImGui::End();
		}
		
		rend.DrawFrame();

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	rend.WaitIdle();
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplSDL3_Shutdown();

	DryOff(instance);
}