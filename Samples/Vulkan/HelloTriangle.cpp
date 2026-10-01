#include <Graphics/VK/Renderer.hpp>
#include <GameInput.hpp>

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
	// Window Config
	Droplet::Graphics::SDL::WindowConfig windowConfig 
	{
		.Width = 640,
		.Height = 480,
		.Flags = 0
	};

	Renderer rnd 
	{
		windowConfig
	};

	if (rnd.Initialize() == 1)
	{
		return 1;
	}

	bool done = false;
	while (!done)
	{
		Droplet::GameInput::Get().Update();
		while (SDL_PollEvent(&rnd.p_event))
		{
			Droplet::GameInput::Get().ProcessEvent(rnd.p_event);

			if (rnd.p_event.type == SDL_EVENT_QUIT)
			{
				done = true;
			}

			if (rnd.p_event.type == SDL_EVENT_WINDOW_RESIZED || rnd.p_event.type == SDL_EVENT_WINDOW_MINIMIZED)
			{
				rnd.windowResize();
			}

			if (rnd.p_event.type == SDL_EVENT_KEY_DOWN) 
			{
				if (rnd.p_event.key.key == SDLK_ESCAPE) 
				{
					done = true;
				}
				
			}

			//if (rnd.p_event.type == SDL_EVENT_MOUSE_MOTION)
			//{
			//	//float xOffset = -rnd.p_event.motion.xrel;
			//	//float yOffset = rnd.p_event.motion.yrel;

			//	//rnd.ProcessMouseMovement(xOffset, yOffset);
			//}
		}
		rnd.drawFrame();
	}
    
    return 0;
}