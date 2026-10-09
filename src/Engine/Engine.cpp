#include <Engine.hpp>
#include <SceneSystem/ComponentRegistry.hpp>
#include <SceneSystem/Components/MeshComponent.hpp>
#include <SceneSystem/Components/ScriptComponent.hpp>

using namespace Droplet;

Engine::Engine(EngineConfig p_config) : m_renderer(p_config.WindowConfig)
{
	// Register engine components
	RegisterComponents();
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

	// Scenesystem
	m_sceneManager.Update(Time::Get().GetDeltaTime());

	// Scriptsystem
	Script::ScriptSystem::Get().Update(Time::Get().GetDeltaTime());

	// Rendering
	m_renderer.DrawFrame();
	
	// Other system that need updating go here.
	
	return DROPLET_RETURNTYPE::OK;
}

SDL_Window *Droplet::Engine::GetWindow()
{
	// TODO: insert return statement here
	return m_renderer.GetWindow();
}

Scene::SceneManager &Droplet::Engine::GetSceneManager()
{
	return m_sceneManager;
}

void Engine::RegisterComponents()
{
	// Register engine components
	Scene::ComponentRegistry::RegisterComponent("MeshComponent",	[]() { return std::make_shared<Scene::MeshComponent>(); });
	Scene::ComponentRegistry::RegisterComponent("ScriptComponent",	[]() { return std::make_shared<Scene::ScriptComponent>(); });
}
