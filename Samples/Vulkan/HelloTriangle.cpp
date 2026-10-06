#include <Graphics/VK/Renderer.hpp>
#include <Graphics/SDL/Event.hpp>
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
	
	Droplet::Graphics::SDL::Window win { windowConfig };
	Droplet::Graphics::SDL::Event event {};	
	
	Droplet::Graphics::Renderer rnd { win.Get() };

	bool done = false;
	while (!done)
	{
		Droplet::GameInput::Get().Update();
		while (SDL_PollEvent(event.Get()))
		{
			Droplet::GameInput::Get().ProcessEvent(*event.Get());

			if (event.Get()->type == SDL_EVENT_QUIT)

			{
				done = true;
			}

			if (event.Get()->type == SDL_EVENT_WINDOW_RESIZED || event.Get()->type == SDL_EVENT_WINDOW_MINIMIZED)
			{
				rnd.ResizeWindow();
			}

			if (event.Get()->type == SDL_EVENT_KEY_DOWN) 
			{
				if (event.Get()->key.key == SDLK_ESCAPE) 
				{
					done = true;
				}
				
			}
		}
		rnd.DrawFrame();
	}
    
    return 0;
}
