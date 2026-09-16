#include <print>
#include "Renderer.hpp"

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    std::print("Hello Triangle!\n");

	bool done = false;


	Renderer rnd;

	if (rnd.Initialize() == 1)
	{
		return 1;
	}

	while (!done)
	{
		while (SDL_PollEvent(&rnd.p_event))
		{
			if (rnd.p_event.type == SDL_EVENT_QUIT)
			{
				done = true;
			}

			if (rnd.p_event.type == SDL_EVENT_WINDOW_RESIZED || rnd.p_event.type == SDL_EVENT_WINDOW_MINIMIZED)
			{
				rnd.windowResize();
			}

			rnd.drawFrame();

			if (rnd.p_event.type == SDL_EVENT_KEY_DOWN) {
				if (rnd.p_event.key.key == SDLK_ESCAPE) {
					done = true;
				}
			}
		}
	}
    
    return 0;
}