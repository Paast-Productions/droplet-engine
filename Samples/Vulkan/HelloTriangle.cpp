#include <Graphics/VK/Renderer.hpp>
#include <GameInput.hpp>

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
	// Window Config
	Droplet::Graphics::SDL::WindowConfig windowConfig 
	{
		.Width = 1280,
		.Height = 720,
		.Flags = 0
	};
	
	Droplet::Graphics::Renderer rnd { windowConfig };

	/*if (rnd.Initialize() == 1)
	{
		return 1;
	}*/

	bool done = false;
	while (!done)
	{
		Droplet::GameInput::Get().Update();
		while (SDL_PollEvent(&rnd.Event))
		{
			Droplet::GameInput::Get().ProcessEvent(rnd.Event);

			if (rnd.Event.type == SDL_EVENT_QUIT)

			{
				done = true;
			}

			if (rnd.Event.type == SDL_EVENT_WINDOW_RESIZED || rnd.Event.type == SDL_EVENT_WINDOW_MINIMIZED)
			{
				rnd.ResizeWindow();
			}

			if (rnd.Event.type == SDL_EVENT_KEY_DOWN) 
			{
				if (rnd.Event.key.key == SDLK_ESCAPE) 
				{
					done = true;
				}
				
			}
		}
		rnd.DrawFrame();
	}
    
    return 0;
}