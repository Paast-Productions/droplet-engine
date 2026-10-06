#include <Engine.hpp>

using namespace Droplet;

Engine::Engine(EngineConfig p_config) : m_renderer(p_config.WindowConfig)
{
}

void Droplet::Engine::Initialize()
{
	if (m_renderer.Initialize() == 1)
		throw std::runtime_error("Error while initializing renderer.");
}

void Droplet::Engine::ShutDown()
{
}

DROPLET_RETURNTYPE Droplet::Engine::Update()
{
	// Time
	Time::Get().Update();

	// Window
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

		if (m_renderer.p_event.type == SDL_EVENT_KEY_DOWN) 
		{
			if (m_renderer.p_event.key.key == SDLK_ESCAPE) 
			{
				return DROPLET_RETURNTYPE::EXIT;
			}
		}
	}

	// Scriptsystem
	Script::ScriptSystem::Get().Update(Time::Get().GetDeltaTime());

	// Rendering
	//m_renderer.drawFrame();
	
	// Other system that need updating go here.
	
	return DROPLET_RETURNTYPE::OK;
}

Graphics::SDL::Window &Droplet::Engine::GetWindow()
{
	// TODO: insert return statement here
	return m_renderer.m_window;
}
