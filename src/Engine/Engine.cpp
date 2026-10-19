#include <Engine.hpp>
#include <stdexcept>
#include <tracy/public/tracy/Tracy.hpp>
#include <SceneSystem/ComponentRegistry.hpp>
#include <SceneSystem/Components/MeshComponent.hpp>
#include <SceneSystem/Components/ScriptComponent.hpp>
#include <SceneSystem/BehaviourRegistry.hpp>
#include <SceneSystem/Behaviours/ScriptBehaviour.hpp>
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/IssueReporting.h>
#include <algorithm>


using namespace Droplet;

// Callback for traces, connect this to your own trace function if you have one
static void TraceImpl(const char *inFMT, ...)
{
	// Format the message
	va_list list;
	va_start(list, inFMT);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), inFMT, list);
	va_end(list);

	// Print to the TTY
	std::cout << buffer << std::endl;
}

#ifdef JPH_ENABLE_ASSERTS

// Callback for asserts, connect this to your own assert handler if you have one
static bool AssertFailedImpl(const char *inExpression, const char *inMessage, const char *inFile, JPH::uint inLine)
{
	// Print to the TTY
	std::cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr ? inMessage : "") << std::endl;

	// Breakpoint
	return true;
};

#endif // JPH_ENABLE_ASSERTS

Engine::Engine(EngineConfig p_config) : m_updateFlagsSetter(p_config.UpdateFlags), m_window(p_config.WindowConfig), m_renderer(m_window)
{
	ZoneScoped;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Docking Branch
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

	m_resourceManager.Initialize(std::filesystem::absolute(ROOT_PATH "/Assets"));

	ImGui_ImplSDL3_InitForVulkan(m_renderer.GetWindow());
	m_initInfo = {};
	m_renderer.GetImGuiInitInfo(m_initInfo);
	ImGui_ImplVulkan_Init(&m_initInfo);
	
	ThreadPool::GetInstance().Initialize();
	
	JPH::RegisterDefaultAllocator();
	JPH::Trace = TraceImpl;
#ifdef JPH_DEBUG
	JPH::JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)
#endif
	JPH::Factory::sInstance = new JPH::Factory();
	JPH::RegisterTypes();
    
    // --- Setup Resource Manager ---
    std::filesystem::path assetDir = std::filesystem::path(ROOT_PATH "/assets");
    m_resourceManager.Initialize(assetDir);
    // Register scanners
    m_resourceManager.RegisterAssetScanner({".fbx", ".glb", ".gltf", ".obj"}, IO::AssimpFormat::ScanAssetResources);
    m_resourceManager.RegisterAssetScanner({".png", ".jpg", ".jpeg", ".ktx", ".dds"}, IO::GliFormat::ScanAssetResources);
    m_resourceManager.RegisterAssetScanner({".slang"}, IO::SlangFormat::ScanAssetResources);
    // Register resources
    m_resourceManager.RegisterResourceType<Texture2DResource>();
    m_resourceManager.RegisterResourceType<MeshResource>();
    m_resourceManager.RegisterResourceType<SkinnedMeshResource>();
    m_resourceManager.RegisterResourceType<AnimationResource>();
    m_resourceManager.RegisterResourceType<ShaderResource>();
    // ------

	// Register engine behaviours
	RegisterBehaviours();

	// Register engine components
	RegisterComponents();
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

DROPLET_RETURNTYPE Droplet::Engine::Update(Graphics::Camera &p_camera, Graphics::RenderTarget &p_renderTarget)
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

	// Scriptsystem

	if (!EngineFlagsOwner::IsUpdateFlagSet(UpdateFlags::SkipScriptLogic))
	{
		Script::ScriptSystem::Get().Update(Time::Get().GetDeltaTime());
	}

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
	m_renderer.DrawFrame(p_camera, p_renderTarget);

	// ImGui
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	
	// Other system that need updating go here.



	FrameMark;

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

ResourceManager &Droplet::Engine::GetResourceManager()
{
	return m_resourceManager;
}

void Droplet::Engine::AddEventListener(EventListener p_listener)
{
	ZoneScoped;

	m_eventListeners.push_back(p_listener);
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

Graphics::Renderer &Engine::GetRenderer()
{
	return m_renderer;
}
