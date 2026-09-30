#include <Engine.hpp>

Droplet::Engine::Engine(EngineConfig p_config) : m_renderer(p_config.WindowConfig)
{
	if (m_renderer.Initialize() == 1)
		throw std::runtime_error("Error while initializing renderer.");
}

void Droplet::Engine::Run()
{
}

void Droplet::Engine::ShutDown()
{
}

Droplet::DROPLET_RETURNTYPE Droplet::Engine::Update()
{
	while (SDL_PollEvent(&m_renderer.p_event))
	{
		if (m_renderer.p_event.type == SDL_EVENT_QUIT)
		{
			return DROPLET_RETURNTYPE::EXIT;
		}

		if (m_renderer.p_event.type == SDL_EVENT_WINDOW_RESIZED || m_renderer.p_event.type == SDL_EVENT_WINDOW_MINIMIZED)
		{
			m_renderer.windowResize();
		}

		m_renderer.drawFrame();

		if (m_renderer.p_event.type == SDL_EVENT_KEY_DOWN) 
		{
			if (m_renderer.p_event.key.key == SDLK_ESCAPE) 
			{
				return DROPLET_RETURNTYPE::EXIT;
			}
		}
	}
	
	// Other system that need updating go here.
	
	return DROPLET_RETURNTYPE::OK;
}
