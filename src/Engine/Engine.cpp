#include <Engine.hpp>
#include <stdexcept>

using namespace Droplet;

Engine::Engine(EngineConfig p_config) : m_renderer(p_config.WindowConfig)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Docking Branch
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

	

	ImGui_ImplSDL3_InitForVulkan(m_renderer.GetWindow());
	m_initInfo = {};
	m_renderer.GetImGuiInitInfo(m_initInfo);
	ImGui_ImplVulkan_Init(&m_initInfo);
}

Droplet::Engine::~Engine()
{
	m_renderer.WaitIdle();
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplSDL3_Shutdown();
}

DROPLET_RETURNTYPE Droplet::Engine::Update()
{
	// Time
	Time::Get().Update();

	// Inputs
	Droplet::GameInput::Get().Update();

	// Window
	while (SDL_PollEvent(&m_renderer.Event))
	{
		for (int i = 0; i < m_eventListeners.size(); i++)
		{
			m_eventListeners[i](m_renderer.Event);
		}

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
		Droplet::GameInput::Get().ProcessEvent(m_renderer.Event);
	}

	// Scenesystem
	m_sceneManager.Update(Time::Get().GetDeltaTime());

	// Scriptsystem
	Script::ScriptSystem::Get().Update(Time::Get().GetDeltaTime());

	// Rendering
	m_renderer.DrawFrame();

	// ImGui
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
	
	// Other system that need updating go here.
	
	return DROPLET_RETURNTYPE::OK;
}

SDL_Window *Droplet::Engine::GetWindow()
{
	// TODO: insert return statement here
	return m_renderer.GetWindow();
}

Graphics::Renderer &Droplet::Engine::TEMP_GetRenderer()
{
	return m_renderer;
}

Scene::SceneManager &Droplet::Engine::GetSceneManager()
{
	return m_sceneManager;
}

void Droplet::Engine::AddEventListener(EventListener p_listener)
{
	m_eventListeners.push_back(p_listener);
}
