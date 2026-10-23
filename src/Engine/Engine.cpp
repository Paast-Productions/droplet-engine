#include <Engine.hpp>
#include <stdexcept>
#include <tracy/public/tracy/Tracy.hpp>
#include <SceneSystem/ComponentRegistry.hpp>
#include <SceneSystem/Components/MeshComponent.hpp>
#include <SceneSystem/Components/ScriptComponent.hpp>
#include <SceneSystem/BehaviourRegistry.hpp>
#include <SceneSystem/Behaviours/ScriptBehaviour.hpp>
#include <algorithm>

using namespace Droplet;

Engine::Engine(EngineConfig p_config) : m_updateFlagsSetter(p_config.UpdateFlags), m_window(p_config.WindowConfig), m_renderer(m_window.Get())
{
	ZoneScoped;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Docking Branch
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

	ResourceManager::GetInstance().Initialize(std::filesystem::absolute(ROOT_PATH "/Assets"));

	ImGui_ImplSDL3_InitForVulkan(m_window.Get());
	m_initInfo = {};
	m_renderer.GetImGuiInitInfo(m_initInfo);
	ImGui_ImplVulkan_Init(&m_initInfo);
    
    ThreadPool::GetInstance().Initialize();
    
    // --- Setup Resource Manager ---
    std::filesystem::path assetDir = std::filesystem::path(ROOT_PATH "/Assets");
	// HACK: DON'T DO THIS
    ResourceManager &resManInstance = ResourceManager::GetInstance();
	resManInstance.Initialize(assetDir);
    // Register scanners
    // TODO: Don't hardcode accepted extensions in Engine constructor
    resManInstance.RegisterAssetScanner({".fbx", ".glb", ".gltf", ".obj"}, IO::AssimpFormat::ScanAssetResources);
    resManInstance.RegisterAssetScanner({".png", ".jpg", ".jpeg", ".ktx", ".dds"}, IO::GliFormat::ScanAssetResources);
    resManInstance.RegisterAssetScanner({".slang"}, IO::SlangFormat::ScanAssetResources);
	
    // Register resources
    resManInstance.RegisterResourceType<Texture2DResource>();
    resManInstance.RegisterResourceType<MeshResource>();
    resManInstance.RegisterResourceType<SkinnedMeshResource>();
	resManInstance.RegisterResourceType<AnimationResource>();
    resManInstance.RegisterResourceType<ShaderResource>();
    // ------

	// Register engine behaviours
	RegisterBehaviours();

	// Register engine components
	RegisterComponents();

	// Script Init
	ScriptSystem::Get().Initialize(m_sceneManager);
}

Droplet::Engine::~Engine()
{
	ZoneScoped;

	// Make sure we detach our behaviours from Scene Manager before Scene Manager might destroy them
	for (const auto &behaviour : m_sceneManager.GetBehaviours())
	{
		if (auto *scriptBehaviour = dynamic_cast<Scene::ScriptBehaviour *>(behaviour.get()))
		{
			scriptBehaviour->AttachScript("");
		}
	}

	m_renderer.WaitIdle();
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplSDL3_Shutdown();

	Shutdown();
}

void Droplet::Engine::Shutdown()
{
	ThreadPool::GetInstance().Shutdown();
}

DROPLET_RETURNTYPE Droplet::Engine::Update(Droplet::Graphics::RenderTarget &p_renderTarget, Droplet::Graphics::Camera &p_camera)
{
	ZoneScoped;

	// Time
	Time::Get().Update();

	// Inputs
	Droplet::GameInput::Get().Update();

	// Window
	while (SDL_PollEvent(&m_renderer.Event))
	{
#ifdef TRACY_ENABLE
		ZoneNamedN(eventZone, "Event", true);

		constexpr int eventStrMaxLen = 256;
		char eventStrBuf[eventStrMaxLen]{};
		int eventStrLen = SDL_GetEventDescription(&m_renderer.Event, eventStrBuf, eventStrMaxLen);

		ZoneTextV(eventZone, eventStrBuf, eventStrLen);
#endif
		// Call all event listeners
		for (int i = 0; i < m_eventListeners.size(); i++)
		{
			m_eventListeners[i](m_renderer.Event);
		}

		switch (m_renderer.Event.type)
		{
		case SDL_EVENT_QUIT:
			FrameMark;
			return DROPLET_RETURNTYPE::EXIT;

		case SDL_EVENT_KEY_DOWN:
			if (m_renderer.Event.key.key == SDLK_ESCAPE) 
			{
				FrameMark;
				return DROPLET_RETURNTYPE::EXIT;
			}
			break;

		case SDL_EVENT_WINDOW_RESIZED:
		case SDL_EVENT_WINDOW_MINIMIZED:
			m_renderer.ResizeWindow();
			break;

		default:
			break;
		}

		Droplet::GameInput::Get().ProcessEvent(m_renderer.Event);
	}

	for (const auto &behaviour : m_sceneManager.GetBehaviours())
	{
		if (behaviour && behaviour->IsActive())
		{
			behaviour->PreUpdate();
		}
	}

	// Scenesystem
	m_sceneManager.Update(Time::Get().GetDeltaTime());
	
	std::vector<Graphics::RenderData> renderDataVec = m_sceneManager.GetRenderData();

	// Script Behaviours
	for (const auto &behaviour : m_sceneManager.GetBehaviours())
	{
		if (behaviour && behaviour->IsActive())
		{
			behaviour->PostUpdate();
		}
	}

	// ResourceManager

	ResourceManager::Get().Update();

	// Rendering
	m_renderer.Draw(p_renderTarget, p_camera, renderDataVec);
	//m_renderer.DrawFrame(p_camera, p_renderTarget);

	// ImGui
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	
	// Other system that need updating go here.
    ResourceManager::GetInstance().Update();



	FrameMark;

	return DROPLET_RETURNTYPE::OK;
}

void Engine::RegisterBehaviours()
{
	// Register engine behaviours
	Scene::BehaviourRegistry::RegisterBehaviour("ScriptBehaviour",	[]() { return std::make_unique<Scene::ScriptBehaviour>(); });
}

void Engine::RegisterComponents()
{
	// Register engine components
	Scene::ComponentRegistry::RegisterComponent("MeshComponent",	[]() { return std::make_unique<Scene::MeshComponent>(); });
	Scene::ComponentRegistry::RegisterComponent("ScriptComponent",	[]() { return std::make_unique<Scene::ScriptComponent>(); });
}

Graphics::RenderTarget Engine::CreateRenderTarget(std::uint32_t p_width, std::uint32_t p_height)
{
	return m_renderer.CreateRenderTarget(p_width, p_height);
}

VkDescriptorSet Engine::RegisterImGuiTexture(Graphics::RenderTarget& p_renderTarget)
{
	return m_renderer.RegisterImGuiTexture(p_renderTarget);
}
