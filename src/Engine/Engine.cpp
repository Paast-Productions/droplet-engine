#include "Engine.hpp"

#include <tracy/public/tracy/Tracy.hpp>
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/IssueReporting.h>
#include <algorithm>
#include <SceneSystem/Items/BaseItems.hpp>

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

#include <SceneSystem/BehaviourRegistry.hpp>
#include <SceneSystem/Behaviours/ScriptBehaviour.hpp>
#include <SceneSystem/Behaviours/PlayerBehaviour.hpp>

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

Engine::Engine(EngineConfig p_config)
    :
    m_updateFlagsSetter(p_config.UpdateFlags),
    m_window(p_config.WindowConfig),
    m_renderer(m_window.Get()),
    // TEMP TEMP TEMP TEMP
    m_inWindow(p_config.inWinCfg),
    m_inSurfaceProvider(m_inWindow.GetSDLWindow()),
    m_inRenderer(m_inSurfaceProvider)
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

    GameInput::Get().SetMainWindow(m_window.Get());

    // TEMP TEMP TEMP TEMP
    m_inWindow.SetResizeCallback([this]() { m_inRenderer.OnResize(); });
    m_inWindow.SetMinimizeCallback([this]() { m_inRenderer.OnMinimize(); });
    m_inWindow.SetRestoreCallback([this]() { m_inRenderer.OnRestore(); });
    // GameInput::Get().SetMainWindow(m_inWindow.GetSDLWindow());
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

    const auto event = m_renderer.Event;
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

        GameInput::Get().ProcessEvent(m_renderer.Event);
    }

    // Scenesystem
    m_sceneManager.Update();

    // ResourceManager

    ResourceManager::GetInstance().Update();

    Audio::AudioSystem::GetInstance().Update();

    return DROPLET_RETURNTYPE::OK;
}

DROPLET_RETURNTYPE Engine::Render(std::vector<Graphics::CameraRenderData>* camRenderDatas)
{
    ZoneScopedC(tracy::Color::SkyBlue1)

    auto renderData = m_sceneManager.GetRenderData();

    if (camRenderDatas)
    {
        for (const auto& camRenderData : *camRenderDatas)
        {
            renderData.push_back(camRenderData);
        }

        // Sort the render data by priority, higher priority first
        std::sort(renderData.begin(), renderData.end(),
                  [](const Graphics::CameraRenderData& a, const Graphics::CameraRenderData& b)
                  {
                      return a.priority > b.priority;
                  });
    }

    // Rendering
    m_renderer.Draw(renderData);

#ifdef _EDITOR
    // ImGui
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
#endif

    m_sceneManager.PostRender();

    ResourceManager::GetInstance().Update();
    Graphics::Vulkan::IntermediaryStorage::GetInstance().DestroyUnusedResources();

    FrameMark;

    return DROPLET_RETURNTYPE::OK;
}

// TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP
// TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP
// TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP TEMP

namespace
{
    namespace
    {
        
        /// @brief The entry for id in assets, or nullptr
        const Assets::ResidentAsset* Find(const std::vector<Assets::ResidentAsset>& assets, Guid id)
        {
            const auto assetIt = std::ranges::find(assets, id, &Assets::ResidentAsset::id);
            return assetIt != assets.end()
                       ? &*assetIt
                       : nullptr;
        }
        
        /// @brief Adds the mesh to uploads if it exists in the asset storage
        void AddMesh(ResourceManager &resMan, Assets::AssetUploads& uploads, Guid id)
        {
            const auto * mesh = resMan.GetResource<MeshResource>(GUID{id.value});
            
            if (!mesh || mesh->GetVertexData().empty() || mesh->GetIndexData().empty())
                return; // not in the storage: the renderer draws its placeholder cube

            uploads.meshes.push_back({
                .id = id,
                .revision = 0, // donot' bother with revision
                .vertexData = mesh->GetVertexData(),
                .vertexStride = static_cast<std::uint32_t>(mesh->GetVertexByteSize()),
                .indices = mesh->GetIndexData()
            });
        }

        /// @brief Adds the texture to uploads if it exists in the asset storage
        void AddTexture(ResourceManager &resMan, Assets::AssetUploads& uploads, Guid id)
        {
            const auto * texture = resMan.GetResource<Texture2DResource>(GUID{id.value});

            if (!texture)
                return; // not in the storage: the renderer draws its placeholder texture

            Common::PixelFormat format = Common::PixelFormat::RGBA8_SRGB;
            
            if (texture->GetFormat() == TextureResource::TextureFormat::RGBA8_Unorm)
                format = Common::PixelFormat::RGBA8_UNORM;
            
            uploads.textures.push_back({
                .id = id,
                .revision = 0,
                .width = texture->GetWidth(),
                .height = texture->GetHeight(),
                .format = format,
                .pixels = texture->GetPixelData(),
                .sampling = Common::TextureSampling{
                    .generateMips = texture->GetMipLevels() > 1,
                    .filter = Common::TextureFilter::Linear,
                    .maxAnisotropy = 1.0f,
                    .repeat = true
                }
            });
        }

        /// @brief Adds the shader to uploads if it exists in the asset storage
        void AddShader(ResourceManager &rscMan, Assets::AssetUploads& uploads, Guid id)
        {
            // TODO: find out how shaders work atm and make it work™
            // const auto * shader = rscMan.GetResource<ShaderResource>(GUID{id.value});
            //
            // if (!shader)
            //     return; // not in the storage: the renderer uses its default shader
            //
            // uploads.shaders.push_back({
            //     .id = id, 
            //     .revision = 0, 
            //     .shader = *shader
            // });
        }
        
        Render::RenderWorld ToRenderWorld(std::span<const Graphics::CameraRenderData> p_cameras)
        {
            using namespace Droplet::Graphics;
            
            // RenderData -> RenderObject
            auto toRenderObject = [](const RenderData& object) -> Render::RenderObject
            {
                Render::RenderObject output{};

                output.meshId = Guid{(object.meshData.GUID)};
                output.transform = object.transform;
                output.shaderId = Guid{object.vsData.GUID}; // TODO: see how pipelines are handled
                output.imageTexture = Common::ImageRef{
                    .kind = Common::ImageRef::Kind::Texture, // TODO: change this when mesh can carry a render target texture
                    .id = Guid{object.texData.GUID}
                };

                return output;
            };

            // CameraRenderData -> RenderView
            auto toRenderView = [](const CameraRenderData& view) -> Render::RenderView
            {
                Render::RenderView output{};

                output.view = view.camera.GetViewMatrix();
                output.proj = view.camera.GetProjectionMatrix();
                output.needsHistory = false;

                return output;
            };

            // std::vector<CameraRenderData> -> Render::RenderWorld
            auto toRenderWorld = [toRenderView, toRenderObject](std::span<const CameraRenderData> data)
            {
                Render::RenderWorld output;

                for (const auto& view : data)
                {
                    for (const auto& object : view.renderData)
                    {
                        output.objects.emplace_back(toRenderObject(object));
                    }
                    output.views.emplace_back(toRenderView(view));
                }

                return output;
            };

            Render::RenderWorld world = toRenderWorld(p_cameras);
            
            return world;
        }
    }

    namespace
    {
        /// @brief Adds a valid `id` to `ids` unless it is already there (a world names the same asset many times).
        void AddUniqueId(std::vector<Guid>& ids, Guid id)
        {
            if (id.IsValid() && std::ranges::find(ids, id) == ids.end())
                ids.push_back(id);
        }

        /// @brief Adds the texture `image` refers to; a render target or no image is not an asset.
        void AddRequiredTexture(Assets::RequiredAssets& required, const Common::ImageRef& image)
        {
            if (image.kind == Common::ImageRef::Kind::Texture)
                AddUniqueId(required.textures, image.id);
        }
    }

    Assets::AssetUploads ProvideAssets(
        const Assets::RequiredAssets& required,
        const Assets::ResidentAssets& resident)
    {
        auto& assets = ResourceManager::GetInstance();

        Assets::AssetUploads uploads;

        // a mesh or texture the GPU lacks, or holds an older revision of
        for (const Guid id : required.meshes)
        {
            const Assets::ResidentAsset* onGpu = Find(resident.meshes, id);
            if (!onGpu /* unsure if resource manager tracks revisions || assets.MeshRevision(id) > onGpu->revision*/)
                AddMesh(assets, uploads, id);
        }
        for (const Guid id : required.textures)
        {
            const Assets::ResidentAsset* onGpu = Find(resident.textures, id);
            if (!onGpu)
                AddTexture(assets, uploads, id);
        }

        // a shader the GPU has not seen, or one that did not compile at an older revision; compiled ones stay
        for (const Guid id : required.shaders)
        {
            if (Find(resident.shaders, id))
                continue;
            const Assets::ResidentAsset* failed = Find(resident.failedShaders, id);
            if (!failed)
                AddShader(assets, uploads, id);
        }

        return uploads;
    }
}

void Engine::Render(std::span<const Graphics::CameraRenderData> p_cameras)
{
    using namespace Droplet::Graphics;
    // order shouldn't matter because render graph sorts it out
    std::vector<CameraRenderData> renderLists = m_sceneManager.GetRenderData();
    renderLists.assign(p_cameras.begin(), p_cameras.end());
    
    // 1. convert scene output to RenderWorld (temporarily)
    Render::RenderWorld world = ToRenderWorld(renderLists);

    // until render targets can be created this is how to draw the main menu TODO: fix this
    static constexpr Guid mainTargetId = Guid::FromName("target/mainCamera");
    int width, height;
    SDL_GetWindowSizeInPixels(m_window.Get(), &width, &height);
    const Common::RenderTargetDesc mainTarget{
        .id = mainTargetId,
        .width = static_cast<uint32_t>(width),
        .height = static_cast<uint32_t>(height),
    };
    world.targets.emplace_back(mainTarget);
    world.views.front().targetId = mainTargetId;

    // 2. Scuffed way of upload/bridge
    Assets::AssetUploads uploads = ProvideAssets(Bridge::RequiredAssetsOf(world), m_inRenderer.Resident());
    
    
    
    m_inRenderer.Upload(uploads);


    // 3. Submit to renderer
    m_inRenderer.Render(world);
}
