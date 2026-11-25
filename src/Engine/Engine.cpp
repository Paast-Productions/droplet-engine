#include "Engine.hpp"

#include <tracy/public/tracy/Tracy.hpp>
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/IssueReporting.h>
#include <algorithm>
#include <cstddef>
#include <SceneSystem/Items/BaseItems.hpp>

#include <SceneSystem/ComponentRegistry.hpp>
#include <SceneSystem/BehaviourRegistry.hpp>
#include <SceneSystem/Behaviours/ScriptBehaviour.hpp>
#include <RegisterGame.hpp>
#include <RegisterEngine.hpp>
#include <Time.hpp>
#include <Resource/IO/AudioFormat.hpp>
#include <SceneSystem/Components/MeshComponent.hpp>
#include <SceneSystem/Components/SkeletonComponent.hpp>
#include <SceneSystem/Components/ScriptComponent.hpp>
#include <SceneSystem/Components/AudioEmitterComponent.hpp>
#include <SceneSystem/Components/AudioListenerComponent.hpp>
#include <SceneSystem/Components/PhysicsComponent.hpp>
#include <SceneSystem/Components/CameraComponent.hpp>
#include <SceneSystem/Components/PlayerComponent.hpp>
#include <SceneSystem/Components/TriggerComponent.hpp>
#include <SceneSystem/Components/DamageComponent.hpp>
#include <SceneSystem/Components/HealthComponent.hpp>
#include <SceneSystem/Behaviours/PlayerBehaviour.hpp>
#include <Graphics/Debug/DebugDraw.hpp>

#include "ResourceBridge/ResourceTransfer.hpp"

using namespace Droplet;

// Callback for traces, connect this to your own trace function if you have one
static void TraceImpl(const char* inFMT, ...)
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
static bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, JPH::uint inLine)
{
    // Print to the TTY
    std::cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr ? inMessage : "") <<
        std::endl;

    // Breakpoint
    return true;
};

#endif // JPH_ENABLE_ASSERTS

// TEMP TEMP TEMP TEMP the bridge from production's scene output and resources to the sandbox renderer
namespace
{
    // -- Assets --

    /// @brief The entry for `id` in `assets`, or nullptr
    const Bridge::ResidentResource* Find(const std::vector<Bridge::ResidentResource>& assets, GUID id)
    {
        const auto assetIt = std::ranges::find(assets, id, &Bridge::ResidentResource::id);
        return assetIt != assets.end() ? &*assetIt : nullptr;
    }

    /// @brief Adds the mesh to `uploads` once the resource manager has loaded it
    void AddMesh(ResourceManager& resMan, Bridge::ResourceUploads& uploads, GUID id)
    {
        // not loaded yet: asked for again next frame (GetResource would hand over the fallback mesh)
        if (resMan.GetState(id) != ResourceState::Ready)
            return;
        const auto* mesh = resMan.GetResource<MeshResource>(id);
        if (!mesh || mesh->GetVertexData().empty() || mesh->GetIndexData().empty())
            return; // not loaded (yet): the placeholder cube

        uploads.meshes.push_back({
            .id = id,
            .revision = 0, // no revisions yet
            .vertexData = mesh->GetVertexData(),
            .vertexStride = static_cast<std::uint32_t>(mesh->GetVertexByteSize()),
            .indices = mesh->GetIndexData()
        });
    }

    /// @brief Adds the texture's top mip level to `uploads` once the resource manager has loaded it
    void AddTexture(ResourceManager& resMan, Bridge::ResourceUploads& uploads, GUID id)
    {
        // not loaded yet: asked for again next frame (GetResource would hand over the fallback texture)
        if (resMan.GetState(id) != ResourceState::Ready)
            return;
        const auto* texture = resMan.GetResource<Texture2DResource>(id);
        if (!texture)
        {
            return; // not loaded (yet): the placeholder texture
        }

        // RGBA8 only: a compressed texture is smaller than this and shows the placeholder
        const std::size_t topLevelBytes = static_cast<std::size_t>(texture->GetWidth()) * texture->GetHeight() * 4;
        const std::span<const std::byte> pixels = texture->GetPixelData();
        if (pixels.size() < topLevelBytes)
        {
            return;
        }

        uploads.textures.push_back({
            .id = id,
            .revision = 0,
            .width = texture->GetWidth(),
            .height = texture->GetHeight(),
            .format = ::Common::PixelFormat::RGBA8_UNORM, // as production's targets
            .pixels = pixels.first(topLevelBytes),
            .sampling = ::Common::TextureSampling{
                .generateMips = true, // the renderer builds the chain from the top level
                .filter = ::Common::TextureFilter::Linear,
                .maxAnisotropy = 1.0f,
                .repeat = true
            }
        });
    }

    /// @brief Adds the default scene shader; production's own shaders need bindless
    void AddShader(ResourceManager& resMan, Bridge::ResourceUploads& uploads, GUID id)
    {
        // not loaded yet: asked for again next frame (GetResource would hand over the fallback shader)
        if (resMan.GetState(id) != ResourceState::Ready)
        {
            return;
        }
        const auto* shader = resMan.GetResource<ShaderResource>(id);
        if (!shader)
        {
            return;
        }

        // the renderer reports other stages once (tessellation and geometry are not supported)
        const ShaderResource::ShaderType type = shader->GetType();
        if (type != ShaderResource::ShaderType::Vertex && type != ShaderResource::ShaderType::Fragment)
        {
            return;
        }

        // no byte code (released from RAM): uploaded empty, the renderer reports it and draws with the defaults
        const Slang::ComPtr<slang::IBlob> byteCode = shader->GetByteCode();
        const std::span<const std::byte> spirv = byteCode
            ? std::span(static_cast<const std::byte*>(byteCode->getBufferPointer()), byteCode->getBufferSize())
            : std::span<const std::byte>{};

        uploads.shaders.push_back({
            .id = id,
            .revision = 0, // no revisions yet
            .stage = type == ShaderResource::ShaderType::Vertex ? Bridge::ShaderStage::Vertex : Bridge::ShaderStage::Fragment,
            .spirv = spirv,
            .entryPoint = "main"
        });
    }

    /// @brief The uploads for what `required` needs and the GPU lacks
    /**
     * @brief Starts loading a required shader that nothing holds a handle to (e.g. an editor shader override)
     * @param p_loaded The handles kept by the engine, so the shaders stay loaded
     * @param p_id The shader's GUID
     */
    void RequestShader(std::unordered_map<GUID, StaticResourceHandle<ShaderResource>>& p_loaded, GUID p_id)
    {
        if (p_id == C_INVALID_GUID || p_loaded.contains(p_id))
        {
            return;
        }
        ResourceManager& resMan = ResourceManager::GetInstance();
        if (resMan.GetState(p_id) == ResourceState::Unloaded)
        {
            p_loaded.emplace(p_id, resMan.LoadResource<ShaderResource>(p_id));
        }
    }

    Bridge::ResourceUploads ProvideAssets(const Bridge::RequiredResources& required,
                                          const Bridge::ResidentResources& resident,
                                          std::unordered_map<GUID, StaticResourceHandle<ShaderResource>>& requestedShaders)
    {
        ResourceManager& resMan = ResourceManager::GetInstance();
        Bridge::ResourceUploads uploads;

        for (const GUID id : required.meshes)
        {
            if (!Find(resident.meshes, id))
                AddMesh(resMan, uploads, id);
        }
        for (const GUID id : required.textures)
        {
            if (!Find(resident.textures, id))
                AddTexture(resMan, uploads, id);
        }
        for (const GUID id : required.shaders)
        {
            if (!Find(resident.shaders, id) && !Find(resident.failedShaders, id))
            {
                RequestShader(requestedShaders, id);
                AddShader(resMan, uploads, id);
        }
        }
        return uploads;
    }

    // -- World --

    /**
     * @brief Loads a shader by its display name, once it is registered
     * @param p_handle The handle to load into; left as it is once valid
     * @param p_name The shader's display name
     */
    void LoadDefaultShader(StaticResourceHandle<ShaderResource>& p_handle, const char* p_name)
    {
        if (p_handle.IsValid())
        {
            return;
        }
        ResourceManager& resMan = ResourceManager::GetInstance();
        MetaEntry entry;
        if (resMan.GetResourceMetaEntry(std::string(p_name), entry))
        {
            p_handle = resMan.LoadResource<ShaderResource>(entry.guid);
        }
    }
}

Engine::Engine(EngineConfig p_config)
    : m_updateFlagsSetter(p_config.UpdateFlags),
      m_window(p_config.WindowConfig),
      m_surfaceProvider(m_window.GetSDLWindow()),
#ifdef _EDITOR
      m_imguiFrame(m_window.GetSDLWindow()),
#endif
      m_renderer(m_surfaceProvider)
{
    ZoneScoped;

#ifdef _EDITOR
    m_renderer.InitImGui(); // the backend needs the context ImGuiFrame created
#endif

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
    ResourceManager& resManInstance = ResourceManager::GetInstance();
    resManInstance.Initialize(assetDir);

    // Register scanners
    // TODO: Don't hardcode accepted extensions in Engine constructor (Why wouldn't we? This is where we define what assets the engine supports)
    resManInstance.RegisterAssetScanner({".fbx", ".glb", ".gltf", ".obj"}, IO::AssimpFormat::ScanAssetResources);
    resManInstance.RegisterAssetScanner({".png", ".jpg", ".jpeg", ".ktx", ".dds"}, IO::GliFormat::ScanAssetResources);
    resManInstance.RegisterAssetScanner({".slang"}, IO::SlangFormat::ScanAssetResources);
    resManInstance.RegisterAssetScanner({".ogg"}, IO::AudioFormat::ScanAssetResources);
	resManInstance.RegisterAssetScanner({".slug"}, IO::FontFormat::ScanAssetResources);
	
    // Register resources
    resManInstance.RegisterResourceType<Texture2DResource>();
    resManInstance.RegisterResourceType<MeshResource>();
    resManInstance.RegisterResourceType<SkinnedMeshResource>();
    resManInstance.RegisterResourceType<AnimationResource>();
    resManInstance.RegisterResourceType<ShaderResource>();
    resManInstance.RegisterResourceType<FontResource>();
    // Don't register SoundResource for now /Arvid

    // Register base items and set the item registry for the scene manager
    Game::RegisterBaseItems(m_itemRegistry);
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

    // Audio Init
    Audio::AudioSystem::GetInstance().Initialize();

    GameInput::Get().SetMainWindow(m_window.GetSDLWindow());
}

Engine::~Engine()
{
    ZoneScoped;

    // Make sure we detach our behaviours from Scene Manager before Scene Manager might destroy them
    for (const auto& behaviour : m_sceneManager.GetBehaviours())
    {
        if (auto* scriptBehaviour = dynamic_cast<Scene::ScriptBehaviour*>(behaviour.get()))
        {
            scriptBehaviour->AttachScript("");
        }
    }

    Shutdown();
}

void Engine::Shutdown()
{
    Audio::AudioSystem::GetInstance().Shutdown();
    ThreadPool::GetInstance().Shutdown();
    Script::ScriptSystem::Get().Shutdown();
}

DROPLET_RETURNTYPE Engine::Update()
{
    ZoneScopedC(tracy::Color::PaleTurquoise1);

    // Time
    Time::Get().Update();

    // Inputs
    GameInput::Get().Update();

    // Window
    SDL_Event event{};
    while (SDL_PollEvent(&event))
    {
#ifdef TRACY_ENABLE
        ZoneNamedNC(eventZone, "Event", tracy::Color::MistyRose1, true);

        constexpr int eventStrMaxLen = 256;
        char eventStrBuf[eventStrMaxLen]{};
        int eventStrLen = SDL_GetEventDescription(&event, eventStrBuf, eventStrMaxLen);

        ZoneTextV(eventZone, eventStrBuf, eventStrLen);
#endif
#ifdef _EDITOR
        m_imguiFrame.ProcessEvent(event); // before the listeners, which may ask ImGui whether it wants input
#endif
        // Call all event listeners
        for (int i = 0; i < m_eventListeners.size(); i++)
        {
            m_eventListeners[i](event);
        }

        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            FrameMark;
            return DROPLET_RETURNTYPE::EXIT;

        case SDL_EVENT_WINDOW_RESIZED:
            m_renderer.OnResize();
            break;

        case SDL_EVENT_WINDOW_MINIMIZED:
            m_renderer.OnMinimize();
            break;

        case SDL_EVENT_WINDOW_RESTORED:
        case SDL_EVENT_WINDOW_MAXIMIZED: // un-minimising a maximised window reports MAXIMIZED
            m_renderer.OnRestore();
            break;

        default:
            break;
        }

        GameInput::Get().ProcessEvent(event);
    }

    // Scenesystem
    m_sceneManager.Update();

    // ResourceManager

    ResourceManager::GetInstance().Update();

    Audio::AudioSystem::GetInstance().Update();

    return DROPLET_RETURNTYPE::OK;
}

void Engine::BeginFrame()
{
#ifdef _EDITOR
    m_imguiFrame.Begin(m_renderer);
#endif
}

DROPLET_RETURNTYPE Engine::Render(std::span<const Common::CameraRenderData> p_cameras)
{
    using namespace Droplet::Graphics;
    ZoneScopedC(tracy::Color::SkyBlue1)

    // the scene's cameras and the editor's. // TODO: check post-process
    // what an object without loaded shaders is drawn with
    LoadDefaultShader(m_defaultVertexShader, "Base.VS");
    LoadDefaultShader(m_defaultFragmentShader, "Base.FS");

    Render::RenderWorld world{
        .cameras = m_sceneManager.GetRenderData(),
        .defaultVertexShader = m_defaultVertexShader.GetGUID(),
        .defaultFragmentShader = m_defaultFragmentShader.GetGUID(),
        .time = Time::Get().GetRuntime() // seconds since start, for shaders
    };
    world.cameras.insert(world.cameras.end(), p_cameras.begin(), p_cameras.end());

    // DebugDraw's meshes, in the cameras that show debug drawing
    const std::vector<std::shared_ptr<const Common::DynamicMeshData>> debugMeshes = Graphics::Debug::DebugDraw::GetFrameMeshes();
    for (Common::CameraRenderData& camera : world.cameras)
    {
        if (!camera.debug)
        {
            // only draw debug geometry on cameras that are flagged for debug drawing
            continue;
        }
        for (const std::shared_ptr<const Common::DynamicMeshData>& mesh : debugMeshes)
        {
            camera.renderData.push_back({.transform = glm::mat4{1.0f}, .dynamicMesh = mesh});
        }
    }

    // ImGui gets drawn as an overlay. Skip if we're not in editor mode
    ImDrawData* overlay = nullptr;
#ifdef _EDITOR
    overlay = m_imguiFrame.End();
    const Bridge::RequiredResources required = Bridge::RequiredResourcesOf(
        world, Vulkan::Renderer::ImGuiImages(*overlay));
    m_renderer.SetCompositeScene(false); // the editor's windows show the cameras
#else
    const BridgeRequiredAssets required = Bridge::RequiredResourcesOf(world);
#endif
    m_renderer.Upload(ProvideAssets(required, m_renderer.Residents(), m_requestedShaders));

    m_renderer.Render(world, overlay);

    m_sceneManager.PostRender(); // clears the cameras' render data
    
    // ResourceManager::GetInstance().Update();

    return DROPLET_RETURNTYPE::OK;
}
