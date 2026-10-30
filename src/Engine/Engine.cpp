#include <Engine.hpp>
#include <stdexcept>
#include <tracy/public/tracy/Tracy.hpp>
#include <SceneSystem/ComponentRegistry.hpp>
#include <SceneSystem/Components/MeshComponent.hpp>
#include <SceneSystem/Components/SkeletonComponent.hpp>
#include <SceneSystem/Components/ScriptComponent.hpp>
#include <SceneSystem/Components/AudioEmitterComponent.hpp>
#include <SceneSystem/Components/AudioListenerComponent.hpp>
#include <SceneSystem/Components/PhysicsComponent.hpp>
#include <SceneSystem/Components/CameraComponent.hpp>
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

Engine::Engine(EngineConfig p_config) : m_updateFlagsSetter(p_config.UpdateFlags), m_window(p_config.WindowConfig), m_renderer(m_window.Get())
{
	ZoneScoped;
	
	ResourceManager::GetInstance().Initialize(ROOT_PATH "/Assets");
    ThreadPool::GetInstance().Initialize();
	JPH::RegisterDefaultAllocator();
	JPH::Trace = TraceImpl;
#ifdef JPH_DEBUG
	JPH::JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)
#endif
	JPH::Factory::sInstance = new JPH::Factory();
	JPH::RegisterTypes();
    
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


	Shutdown();
}

void Droplet::Engine::Shutdown()
{
	ThreadPool::GetInstance().Shutdown();
}

DROPLET_RETURNTYPE Droplet::Engine::Update(Droplet::Graphics::Camera &p_camera)
{
	ZoneScoped;

	// Time
	Time::Get().Update();

	// Inputs
	Droplet::GameInput::Get().Update();

	ImGui::Shortcut(ImGuiKey_Tab, ImGuiInputFlags_RouteGlobal);
	ImGui::Shortcut(ImGuiMod_Shift | ImGuiKey_Tab, ImGuiInputFlags_RouteGlobal);

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
	m_renderer.Draw(p_camera, renderDataVec);
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
	Scene::ComponentRegistry::RegisterComponent("AudioListenerComponent", []() { return std::make_unique<Scene::AudioListenerComponent>();});
	Scene::ComponentRegistry::RegisterComponent("AudioEmitterComponent", []() { return std::make_unique<Scene::AudioEmitterComponent>();});
	Scene::ComponentRegistry::RegisterComponent("PhysicsComponent",	[]() { return std::make_unique<Scene::PhysicsComponent>(); });
	Scene::ComponentRegistry::RegisterComponent("CameraComponent", []() { return std::make_unique<Scene::CameraComponent>(); });
	Scene::ComponentRegistry::RegisterComponent("SkeletonComponent", []() { return std::make_unique<Scene::SkeletonComponent>(); });
}