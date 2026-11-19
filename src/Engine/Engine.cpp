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
#include <Graphics/DebugDraw.hpp>

// TEMP TEMP TEMP TEMP
#include "AssetProvider.hpp"

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
    // -- GUIDs --

    Guid ToSandboxGuid(GUID p_guid) { return Guid{ p_guid }; }
    GUID ToProductionGuid(Guid p_guid) { return GUID{ p_guid.value }; }

    // -- Assets --

    /// @brief The entry for `id` in `assets`, or nullptr
    const Assets::ResidentAsset* Find(const std::vector<Assets::ResidentAsset>& assets, Guid id)
    {
        const auto assetIt = std::ranges::find(assets, id, &Assets::ResidentAsset::id);
        return assetIt != assets.end() ? &*assetIt : nullptr;
    }

    /// @brief Adds the mesh to `uploads` once the resource manager has loaded it
    void AddMesh(ResourceManager& resMan, Assets::AssetUploads& uploads, Guid id)
    {
        const auto* mesh = resMan.GetResource<MeshResource>(ToProductionGuid(id));
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
    void AddTexture(ResourceManager& resMan, Assets::AssetUploads& uploads, Guid id)
    {
        const auto* texture = resMan.GetResource<Texture2DResource>(ToProductionGuid(id));
        if (!texture)
            return; // not loaded (yet): the placeholder texture

        // RGBA8 only: a compressed texture is smaller than this and shows the placeholder
        const std::size_t topLevelBytes = static_cast<std::size_t>(texture->GetWidth()) * texture->GetHeight() * 4;
        const std::span<const std::byte> pixels = texture->GetPixelData();
        if (pixels.size() < topLevelBytes)
            return;

        uploads.textures.push_back({
            .id = id,
            .revision = 0,
            .width = texture->GetWidth(),
            .height = texture->GetHeight(),
            .format = Common::PixelFormat::RGBA8_SRGB, // colour space later
            .pixels = pixels.first(topLevelBytes),
            .sampling = Common::TextureSampling{
                .generateMips = true, // the renderer builds the chain from the top level
                .filter = Common::TextureFilter::Linear,
                .maxAnisotropy = 1.0f,
                .repeat = true
            }
        });
    }

    /// @brief Adds the default scene shader; production's own shaders need bindless
    void AddShader(Assets::AssetUploads& uploads, Guid id)
    {
        if (id != Render::DEFAULT_SCENE_SHADER)
            return;
        uploads.shaders.push_back({
            .id = id,
            .revision = 1,
            .shader = Common::ShaderDesc{ .moduleName = "droplet_base", .sourcePath = "Shaders/droplet_base.slang" }
        });
    }

    /// @brief The uploads for what `required` needs and the GPU lacks
    Assets::AssetUploads ProvideAssets(const Assets::RequiredAssets& required, const Assets::ResidentAssets& resident)
    {
        ResourceManager& resMan = ResourceManager::GetInstance();
        Assets::AssetUploads uploads;

        for (const Guid id : required.meshes)
        {
            if (!Find(resident.meshes, id))
                AddMesh(resMan, uploads, id);
        }
        for (const Guid id : required.textures)
        {
            if (!Find(resident.textures, id))
                AddTexture(resMan, uploads, id);
        }
        for (const Guid id : required.shaders)
        {
            if (!Find(resident.shaders, id) && !Find(resident.failedShaders, id))
                AddShader(uploads, id);
        }
        return uploads;
    }

    // -- World --

    /// @brief DebugDraw's batched meshes as dynamic draws; returns their indices
    std::vector<std::uint32_t> AddDebugDraws(Render::RenderWorld& world)
    {
        Graphics::Debug::DebugDraw& debugDraw = Graphics::Debug::DebugDraw::Get();
        debugDraw.PrepareRender(0); // not initialised: builds the meshes, uploads nothing

        std::vector<std::uint32_t> indices;
        const auto add = [&](std::span<const std::byte> vertices, Render::DynamicTopology topology, bool depthTest)
        {
            if (vertices.empty())
                return;
            indices.push_back(static_cast<std::uint32_t>(world.dynamicDraws.size()));
            world.dynamicDraws.push_back({
                .layout = Render::DynamicVertexLayout::P3C4,
                .topology = topology,
                .vertexData = { vertices.begin(), vertices.end() },
                .depthTest = depthTest });
        };
        add(debugDraw.WireMesh(true), Render::DynamicTopology::Lines, true);
        add(debugDraw.FillMesh(true), Render::DynamicTopology::Triangles, true);
        add(debugDraw.WireMesh(false), Render::DynamicTopology::Lines, false);
        add(debugDraw.FillMesh(false), Render::DynamicTopology::Triangles, false);

        debugDraw.ClearDrawCommands();
        return indices;
    }

    /// @brief A view and a target per camera, an object per draw
    void AddCameras(Render::RenderWorld& world, std::span<const Graphics::CameraRenderData> p_cameras,
                    std::span<const std::uint32_t> debugDraws)
    {
        for (const Graphics::CameraRenderData& camera : p_cameras)
        {
            const Guid targetId = ToSandboxGuid(camera.renderTarget.GetColourImageGuid());
            const vk::Extent2D extent = camera.renderTarget.GetExtent();
            if (!targetId.IsValid() || extent.width == 0 || extent.height == 0)
                continue;
            if (std::ranges::any_of(world.targets, [&](const Common::RenderTargetDesc& desc) { return desc.id == targetId; }))
                continue; // one writer per target

            world.targets.push_back({ .id = targetId, .width = extent.width, .height = extent.height });

            Render::RenderView view;
            view.view = camera.camera.GetViewMatrix();
            view.proj = camera.camera.GetProjectionMatrix();
            view.clearColor = camera.clearColor;
            view.targetId = targetId;
            if (camera.debug)
                view.dynamicVisible.assign(debugDraws.begin(), debugDraws.end());

            // already culled by the scene
            for (const Graphics::RenderData& object : camera.renderData)
            {
                const Guid textureId = ToSandboxGuid(object.texData.GUID);
                view.visible.push_back(static_cast<std::uint32_t>(world.objects.size()));
                world.objects.push_back({
                    .transform = object.transform,
                    .meshId = ToSandboxGuid(object.meshData.GUID),
                    .imageTexture = {
                        .kind = textureId.IsValid() ? Common::ImageRef::Kind::Texture : Common::ImageRef::Kind::None,
                        .id = textureId },
                    .transparent = object.transparent }); // shaderId invalid: the default shader
            }
            world.views.push_back(std::move(view));
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
    // TODO: (RESOURCES) Don't hardcode accepted extensions in Engine constructor (Why wouldn't we? This is where we define what assets the engine supports)
    resManInstance.RegisterAssetScanner({".fbx", ".glb", ".gltf", ".obj"}, IO::AssimpFormat::ScanAssetResources);
    resManInstance.RegisterAssetScanner({".png", ".jpg", ".jpeg", ".ktx", ".dds"}, IO::GliFormat::ScanAssetResources);
    resManInstance.RegisterAssetScanner({".slang"}, IO::SlangFormat::ScanAssetResources);
    resManInstance.RegisterAssetScanner({".ogg"}, IO::AudioFormat::ScanAssetResources);

    // Register resources
    resManInstance.RegisterResourceType<Texture2DResource>();
    resManInstance.RegisterResourceType<MeshResource>();
    resManInstance.RegisterResourceType<SkinnedMeshResource>();
    resManInstance.RegisterResourceType<AnimationResource>();
    resManInstance.RegisterResourceType<ShaderResource>();
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

DROPLET_RETURNTYPE Engine::Render(std::span<const Graphics::CameraRenderData> p_cameras)
{
    ZoneScopedC(tracy::Color::SkyBlue1)

    // the scene's cameras and the editor's; the render graph orders the passes
    std::vector<Graphics::CameraRenderData> cameras = m_sceneManager.GetRenderData();
    cameras.insert(cameras.end(), p_cameras.begin(), p_cameras.end());

    Render::RenderWorld world;
    world.time = 0.0f; // TODO: the engine's elapsed time in seconds
    const std::vector<std::uint32_t> debugDraws = AddDebugDraws(world);
    AddCameras(world, cameras, debugDraws);

    ImDrawData* overlay = nullptr;
#ifdef _EDITOR
    const Assets::RequiredAssets required = Bridge::RequiredAssetsOf(world, m_renderer.ImGuiImages());
    m_renderer.SetCompositeScene(false); // the editor's windows show the cameras
#else
    const Assets::RequiredAssets required = Bridge::RequiredAssetsOf(world);
#endif
    m_renderer.Upload(ProvideAssets(required, m_renderer.Resident()));

#ifdef _EDITOR
    overlay = m_imguiFrame.End();
#endif
    m_renderer.Render(world, overlay);

    m_sceneManager.PostRender(); // clears the cameras' render data
    ResourceManager::GetInstance().Update();

    FrameMark;
    return DROPLET_RETURNTYPE::OK;
}

#ifdef _EDITOR
ImTextureRef Engine::GetImGuiTexture(GUID p_image)
{
    return ImTextureRef(m_renderer.ImGuiTexture(ToSandboxGuid(p_image)));
}
#endif
