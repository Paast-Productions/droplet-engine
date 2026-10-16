/**
 * @file Sandbox.cpp
 * @author Sebastian Lorensson
 * @date 2026-10-16
 * @brief Sandbox main for testing Vulkan functionality such as synchronisation with pipeline barriers, events, fences and timeline semaphores.
 */
#include "Sandbox.hpp"

#include <print>

using namespace Droplet;
using namespace Droplet::Sandbox;


/// Main entry point
int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{

	SandboxApp::AppConfig cfg{	};
	SandboxApp app(cfg);

	// Window Config
	Graphics::SDL::Window window
	{
		Graphics::SDL::WindowConfig
		{
			.Width = 1280,
			.Height = 720,
			.Flags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE,
			.title = "Sandbox"
		}
	};

	if (!window.Get())
		return -1;

	constexpr Graphics::Renderer::Config renderConfig
	{
		.usingImGui = false
	};

	Graphics::Renderer rnd{ window, renderConfig };
	// prepare shader


	bool done = false;
	while (!done)
	{
		// Game needs to reset its inputs and update mouse pos every frame
		GameInput::Get().Update();

		if (ProcessRenderWindowEvents(rnd))
		{
			done = true;
			std::print("Process now exiting\n");
		}

		rnd.DrawFrame();
	}
}