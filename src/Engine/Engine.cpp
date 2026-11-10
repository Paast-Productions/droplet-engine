#include <Engine.hpp>
#include <stdexcept>
#include <tracy/public/tracy/Tracy.hpp>
#include <SceneSystem/ComponentRegistry.hpp>
#include <SceneSystem/BehaviourRegistry.hpp>
#include <SceneSystem/Behaviours/ScriptBehaviour.hpp>
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/IssueReporting.h>
#include <algorithm>
#include <SceneSystem/Items/BaseItems.hpp>
#include <RegisterGame.hpp>
#include <RegisterEngine.hpp>
#include <Time.hpp>

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

	// Register base items and set the item registry for the scene manager
	Droplet::Game::RegisterBaseItems(m_itemRegistry);
	m_sceneManager.SetItemRegistry(&m_itemRegistry);

	// Register engine behaviours
	Droplet::RegisterEngineBehaviours();

	// Register game behaviours
	Droplet::Game::RegisterGameBehaviours();

	// Register engine components
	Droplet::RegisterEngineComponents();

	//register game components
	Droplet::Game::RegisterGameComponents();

	// Script Init
	Script::ScriptSystem::Get().Initialize(m_sceneManager);
	GameInput::Get().SetMainWindow(m_window.Get());

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

DROPLET_RETURNTYPE Droplet::Engine::Update()
{
	ZoneScopedC(tracy::Color::PaleTurquoise1);

	// Time
	Time::Get().Update();

	// Inputs
	Droplet::GameInput::Get().Update();

	// Window
	while (SDL_PollEvent(&m_renderer.Event))
	{
#ifdef TRACY_ENABLE
		ZoneNamedNC(eventZone, "Event", tracy::Color::MistyRose1, true);

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

	// Scenesystem
	m_sceneManager.Update();

	// ResourceManager

	ResourceManager::GetInstance().Update();

	return DROPLET_RETURNTYPE::OK;
}

DROPLET_RETURNTYPE Droplet::Engine::Render(std::vector<Graphics::CameraRenderData> *camRenderDatas)
{
	ZoneScopedC(tracy::Color::SkyBlue1);

	auto renderData = m_sceneManager.GetRenderData();

	if (camRenderDatas)
	{
		for (const auto &camRenderData : *camRenderDatas)
		{
			renderData.push_back(camRenderData);
		}

		// Sort the render data by priority, higher priority first
		std::sort(renderData.begin(), renderData.end(), [](const Graphics::CameraRenderData &a, const Graphics::CameraRenderData &b) {
			return a.priority > b.priority;
		});
	}

	// Rendering
	m_renderer.Draw(renderData);

	// ImGui
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	m_sceneManager.PostRender();


	// Other system that need updating go here.
	ResourceManager::GetInstance().Update();
	Graphics::VK::IntermediaryStorage::GetInstance().DestroyUnusedResources();

	FrameMark;

	return DROPLET_RETURNTYPE::OK;
}
