/**
 * @file Sandbox.cpp
 * @author Sebastian Lorensson
 * @date 2026-10-16
 * @brief Sandbox main for testing Vulkan functionality such as synchronisation with pipeline barriers, events, fences and timeline semaphores.
 */
#include <Graphics/VK/Renderer.hpp>
#include <Graphics/VK/ShaderCompiler.hpp>
#include <GameInput.hpp>

#include <print>

using namespace Droplet;


/// Handle renderer's window events. Cleaning up main.
static bool ProcessRenderWindowEvents(Graphics::Renderer& rnd);


/// Main entry point
int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	// Window Config
	const Graphics::Renderer::Config renderConfig
	{
		.windowConfig = {
			.Width = 1280,
			.Height = 720,
			.Flags = 0,
			.title = "Sandbox"},
		.usingImGui = false,
	};


	Graphics::Renderer rnd{ renderConfig };


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

/// @brief Handle renderer's window events.
/// @param rnd Initialized Droplet Renderer class
/// @return true if quit message was had, otherwise false
static bool ProcessRenderWindowEvents(Graphics::Renderer& rnd)
{
	while (SDL_PollEvent(&rnd.Event))
	{
		GameInput::Get().ProcessEvent(rnd.Event);


		if (rnd.Event.type == SDL_EVENT_QUIT)
		{
			return true;
		}

		if (rnd.Event.type == SDL_EVENT_WINDOW_RESIZED || rnd.Event.type == SDL_EVENT_WINDOW_MINIMIZED)
		{
			rnd.ResizeWindow();
		}


		if (rnd.Event.type == SDL_EVENT_KEY_DOWN)
		{
			if (rnd.Event.key.key == SDLK_ESCAPE)
			{
				return true;
			}

		}
	}


	return false;
}