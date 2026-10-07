#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_vulkan.h>
#include <ImGui/imgui_impl_sdl3.h>
#include <SDL3/SDL.h>
#include <cstdio>
#include <Engine/Engine.hpp>
#include "HierarchyWindow.hpp"
#include <NodeInspectorWindow.hpp>
#include <Graphics/VK/Renderer.hpp>
#include <GameInput.hpp>

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

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
	// TODO: Check if Tracy is enabled and if so, sleep for a few seconds to allow the profiler to connect before starting the engine

	ZoneScopedN("Editor");
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	Droplet::Engine engine({
		Droplet::Graphics::SDL::WindowConfig {
			640, 480, {}
		}
	});

	SDL_Window *wnd = engine.GetWindow();
	if (!wnd)
	{
		return 1;
	}

	FrameMark;

	// Run main loop
	while (engine.Update() == Droplet::DROPLET_RETURNTYPE::OK)
	{
		ZoneScopedN("Main Loop");
		FrameMark;
	}

	// I do not know but i needed to have them because warnings = errors :(
	DropletInstance *instance = Soak();
	DryOff(instance);

	return 0;
}
