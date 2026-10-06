#include <Engine.hpp>

using namespace Droplet;

Engine::Engine() : 
	m_mainWindow(
	{
		.Width = 1280,
		.Height = 720,
		.Flags = 0
	}),
	m_renderer(m_mainWindow.Get()) {}

DROPLET_RETURNTYPE Droplet::Engine::Update()
{
	// Time
	Time::Get().Update();

	// Window
	while (SDL_PollEvent(m_eventListener.Get()))
	{
		if (m_eventListener.Get()->type == SDL_EVENT_QUIT)
		{
			return DROPLET_RETURNTYPE::EXIT;
		}

		if (m_eventListener.Get()->type == SDL_EVENT_WINDOW_RESIZED || m_eventListener.Get()->type == SDL_EVENT_WINDOW_MINIMIZED)
		{
			m_renderer.ResizeWindow();
		}

		if (m_eventListener.Get()->type == SDL_EVENT_KEY_DOWN) 
		{
			if (m_eventListener.Get()->key.key == SDLK_ESCAPE) 
			{
				return DROPLET_RETURNTYPE::EXIT;
			}
		}
	}

	// Scriptsystem
	Script::ScriptSystem::Get().Update(Time::Get().GetDeltaTime());

	// Rendering
	m_renderer.DrawFrame();
	
	// Other system that need updating go here.
	
	return DROPLET_RETURNTYPE::OK;
}

SDL_Window *Droplet::Engine::GetWindow() const
{
	// TODO: insert return statement here
	return m_mainWindow.Get();
}
