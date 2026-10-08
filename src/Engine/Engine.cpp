#include <Engine.hpp>

using namespace Droplet;

Engine::Engine(EngineConfig p_config) : m_renderer(p_config.WindowConfig)
{
}

DROPLET_RETURNTYPE Droplet::Engine::Update()
{
	// Time
	Time::Get().Update();

	// Window
	while (SDL_PollEvent(&m_renderer.Event))
	{
		if (m_renderer.Event.type == SDL_EVENT_QUIT)
		{
			return DROPLET_RETURNTYPE::EXIT;
		}

		if (m_renderer.Event.type == SDL_EVENT_WINDOW_RESIZED || m_renderer.Event.type == SDL_EVENT_WINDOW_MINIMIZED)
		{
			m_renderer.ResizeWindow();
		}

		if (m_renderer.Event.type == SDL_EVENT_KEY_DOWN) 
		{
			if (m_renderer.Event.key.key == SDLK_ESCAPE) 
			{
				return DROPLET_RETURNTYPE::EXIT;
			}
		}
	}

	// Scriptsystem
	Script::ScriptSystem::Get().Update(Time::Get().GetDeltaTime());

	// Rendering
	//m_renderer.DrawFrame();
	
	// Other system that need updating go here.
	
	return DROPLET_RETURNTYPE::OK;
}

SDL_Window *Droplet::Engine::GetWindow()
{
	// TODO: insert return statement here
	return m_renderer.GetWindow();
}
