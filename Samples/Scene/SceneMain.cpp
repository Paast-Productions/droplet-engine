#include <iostream>
#include <memory>
#include <random>
#include <string_view>

#include "SceneSystem/SceneManager.hpp"
#include "SceneSystem/Scene.hpp"
#include "SceneSystem/Node.hpp"
#include "SceneSystem/Component.hpp"
#include "SceneSystem/Components/MeshComponent.hpp"
#include "SceneSystem/Components/ScriptComponent.hpp"
#include "SceneSystem/Serialization/SceneSerializer.hpp"
#include "SceneSystem/Serialization/SerializationContext.hpp"
#include "SceneSystem/ComponentRegistry.hpp"
#include "SceneSystem/ComponentFactory.hpp"
#include "Core/IoManager.hpp"
#include "GameInput.hpp"
#include <SDL3/SDL_init.h>

#include <SceneSystem/Physics/Physics.hpp>
#include <SceneSystem/Physics/PhysicsHelpers.hpp>
#include <SceneSystem/Components/PhysicsComponent.hpp>
#include <resource/ResourceManager.hpp>
#include <core/ThreadPool.hpp>
#include <resource/types/MeshResource.hpp>
#include <Jolt/Core/Factory.h>
#include <Jolt/RegisterTypes.h>
#include <Time.hpp>

using namespace Droplet::Scene;

static void OctreeTest()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::printf("Failed (SDL_INIT)");
        return;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Octree Test",
        1280,
        720,
        SDL_WINDOW_RESIZABLE
    );

    if (!window)
    {
        std::printf("Failed to create window: %s\n", SDL_GetError());
        return;
    }

    SceneManager sm;
    sm.LoadScene("Game");

    std::shared_ptr<Scene> s = sm.GetScene("Game"); // TODO: Make weak pointer
    if (!s)
    {
        std::cerr << "Failed to load Game scene\n";
        return;
    }

    sm.ActivateScene("Game");

    std::srand(std::time(0));
    for (int i = 0; i < 1000; i++)
    {
        float x = -100.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 200.0f));
        float y = -100.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 200.0f));
        float z = -100.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 200.0f));

        auto node = s->AddNode("Rock" + std::to_string(i));
        node->GetTransform().SetPosition(glm::vec3(x, y, z));
    }
    std::printf("Done spawning rocks\n");

    Droplet::GameInput &gi = Droplet::GameInput::Get();
    while (false)
    {
        gi.Update();

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            gi.ProcessEvent(event);
        }

        if (gi.KeyPressed(Droplet::Key::KeyEscape))
        {
            break;
        }

        if (gi.KeyPressed(Droplet::Key::KeyP))
        {
            continue;
        }

        if (gi.KeyPressed(Droplet::Key::KeyP))
        {
            const std::vector<std::shared_ptr<Node>> roots = s->GetRoots(); // TODO: Make weak pointer
            for (int i = 0; i < 100; i++)
            {
                int idx = rand() % 1000;
                float x = -100.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 200.0f));
                float y = -100.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 200.0f));
                float z = -100.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 200.0f));
                roots[idx]->GetTransform().SetPosition(glm::vec3(x, y, z));
            }
        }

        if (gi.KeyPressed(Droplet::Key::KeyT))
        {
            s->PrintOctree();
        }
        if (gi.KeyPressed(Droplet::Key::KeyY))
        {
            s->PrintOctreeTree();
        }

        sm.Update(1.0f / 60.0f);
    }
}

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

static void PhysicsTest()
{
    // ========== Initialize SDL window ==========

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::printf("Failed (SDL_INIT)");
        return;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Physics Test",
        1280,
        720,
        SDL_WINDOW_RESIZABLE
    );

    if (!window)
    {
        std::printf("Failed to create window: %s\n", SDL_GetError());
        return;
    }

    // ========== Initialize Jolt ==========

    JPH::RegisterDefaultAllocator();
    JPH::Trace = TraceImpl;
#ifdef JPH_DEBUG
    JPH::JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)
#endif
    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();

    // ========== Initialize game ==========
    
    ComponentRegistry::RegisterComponent("PhysicsComponent", []() { return std::make_unique<PhysicsComponent>(); });

    Droplet::ThreadPool::GetInstance().Initialize();
    Droplet::ResourceManager &rm = Droplet::ResourceManager::GetInstance();
    rm.Initialize(ROOT_PATH "/Assets");
    rm.RegisterResourceType<Droplet::MeshResource>();
    rm.RegisterAsset("Seafloor.fbx");
    const Droplet::ResourceHandle<Droplet::MeshResource> rh = rm.LoadResource<Droplet::MeshResource>(12751091778080588892);

    Droplet::Time &time = Droplet::Time::Get();
    float timePassed = 0.0f;
    while (true)
    {
        time.Update();
        timePassed += time.GetDeltaTime();

        rm.Update();

        if (timePassed >= 1.0f)
        {
            break;
        }
    }
    
    SceneManager sm;
    sm.LoadScene("Game");
    
    const std::shared_ptr<Scene> s = sm.GetScene("Game");
    if (!s)
    {
        std::cerr << "Failed to load Game scene\n";
        return;
    }

    s->InitializePhysics();
    s->GetPhysics()->SetGravity(glm::vec3(0.0f, -9.82f, 0.0f));

    sm.ActivateScene("Game");

    auto seafloor = s->AddNode("Seafloor");
    auto player = s->AddNode("Player");
    player->GetTransform().SetPosition(glm::vec3(0.0f, 60.0f, 0.0f));

    std::unique_ptr<Component> seafloorComp = ComponentFactory::CreateComponent("PhysicsComponent");
    seafloor->AddComponent(seafloorComp);
	auto seafloorComponets = seafloor->GetComponents<PhysicsComponent>();
    seafloorComponets[0]->Initialize(s->GetPhysics(), &seafloor->GetTransform());
    seafloorComponets[0]->SetMeshCollider(rh.GetGUID());

    std::unique_ptr<Component> playerComp = ComponentFactory::CreateComponent("PhysicsComponent");
    player->AddComponent(playerComp);
	auto playerComponets = player->GetComponents<PhysicsComponent>();
    playerComponets[0]->Initialize(s->GetPhysics(), &player->GetTransform());
    playerComponets[0]->SetCapsuleCollider(0.5f, 0.5f);
    playerComponets[0]->SetMotionType(JPH::EMotionType::Dynamic);
    playerComponets[0]->SetObjectLayer(Droplet::Layers::MOVING);

    std::cout << "Initialization finished\n";

    // ========== Main loop ==========

    Droplet::GameInput &gi = Droplet::GameInput::Get();
    while (true)
    {
        time.Update();
        gi.Update();

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            gi.ProcessEvent(event);
        }

        if (gi.KeyPressed(Droplet::Key::KeyEscape))
        {
            break;
        }

        float dt = time.GetDeltaTime();
        if (gi.KeyHeld(Droplet::Key::KeyW))
        {
            player->GetTransform().Move(glm::vec3(0.0f, 0.0f, 1.0f * dt), Transform::Space::World);
        }
        if (gi.KeyHeld(Droplet::Key::KeyA))
        {
            player->GetTransform().Move(glm::vec3(-1.0f * dt, 0.0f, 0.0f), Transform::Space::World);
        }
        if (gi.KeyHeld(Droplet::Key::KeyS))
        {
            player->GetTransform().Move(glm::vec3(0.0f, 0.0f, -1.0f * dt), Transform::Space::World);
        }
        if (gi.KeyHeld(Droplet::Key::KeyD))
        {
            player->GetTransform().Move(glm::vec3(1.0f * dt, 0.0f, 0.0f), Transform::Space::World);
        }
        if (gi.KeyPressed(Droplet::Key::KeySpace))
        {
            player->GetTransform().SetPosition(glm::vec3(0.0f, 100.0f, 0.0f), Transform::Space::World);
        }

        sm.Update(time.GetDeltaTime());

        glm::vec3 p = player->GetTransform().GetPosition();
        glm::vec3 f = seafloor->GetTransform().GetPosition();
        std::print(std::cout, "Floor({}, {}, {}), Player({}, {}, {})\n", f.x, f.y, f.z, p.x, p.y, p.z);
    }
}

// --------------------------------------------------
// Example Component
// --------------------------------------------------

class PlayerComponent : public Component
{
public:
    std::string_view GetTypeName() const override { return "PlayerComponent"; }

    void StartImpl() override
    {
        std::cout << "PlayerComponent started\n";
    }

    void UpdateImpl() override
    {
        std::cout
            << "PlayerComponent updating: "
            << Droplet::Time::Get().GetDeltaTime()
            << " seconds\n";
    }

    nlohmann::json SerializeImpl() override
    {
        nlohmann::json json;
        json["type"] = "PlayerComponent";
        return json;
    }

    void DeserializeImpl([[maybe_unused]] const nlohmann::json p_compJson) override
    {

    }

};

// --------------------------------------------------
// Another Example Component
// --------------------------------------------------

class EnemyComponent :public Component
{
public:
    std::string_view GetTypeName() const override { return "EnemyComponent"; }

    void StartImpl() override
    {
        std::cout << "EnemyComponent started\n";
    }

    void UpdateImpl() override
    {
        std::cout
            << "EnemyComponent updating: "
            << Droplet::Time::Get().GetDeltaTime()
            << " seconds\n";
    }

    nlohmann::json SerializeImpl() override
    {
        nlohmann::json json;
        
        json["reference"]["nodeid"] = GetTarget()->GetOwner()->GetID();
        json["reference"]["componentid"] = GetTarget()->GetID();
        
        
        return json;
    }

    void DeserializeImpl([[maybe_unused]] const nlohmann::json p_compJson) override
    {
        SceneObjectID targetID;
        if (!p_compJson.contains("reference"))
        {
            throw std::runtime_error("Component missing field 'reference'");
        }
        nlohmann::json refJson = p_compJson["reference"];

        if (!refJson.contains("nodeid"))
        {
            throw std::runtime_error("Reference missing field 'nodeid'");
        }
        if (!refJson.contains("componentid"))
        {
            throw std::runtime_error("Reference missing field 'componentid'");
        }

        targetID.nodeID = refJson["nodeid"];
        targetID.componentID = refJson["componentid"];

        m_target.SetSerializedID(targetID);
        
    }

    void ResolveReferences([[maybe_unused]]SerializationContext &p_context) override
    {
        auto comp = p_context.FindComponent(m_target.serializedID.nodeID, m_target.serializedID.componentID);
        SetTarget(comp);
    }

    void SetTarget(std::shared_ptr<Component> p_target)
    {
        m_target.SetTarget(p_target);
    }
    std::shared_ptr<Component>GetTarget() const
    {
        return m_target.GetTarget();
    }

    SceneObjectID GetTargetID()
    {
        return m_target.serializedID;
    }

private:
    SceneReference<Component> m_target;
};

// --------------------------------------------------
// Main
// --------------------------------------------------

int main()
{
    //OctreeTest();
    PhysicsTest();

    SceneManager sceneManager;

    // ==================================================
    // Create and load a scene
    // ==================================================

    sceneManager.LoadScene("Game");

    auto scene = sceneManager.GetScene("Game");

    if (!scene)
    {
        std::cerr << "Failed to load Game scene\n";
        return 1;
    }

    // ==================================================
    // Create the root node
    // ==================================================

    auto root = scene->AddNode("Root");

    // ==================================================
    // Build the scene hierarchy
    // ==================================================

    auto player =
        root->AddChild(
            scene->AddNode("Player"));

    auto camera =
        player->AddChild(
            scene->AddNode("Camera"));

    auto weapon =
        player->AddChild(
            scene->AddNode("Weapon"));

    auto enemy =
        root->AddChild(
            scene->AddNode("Enemy"));

    // ==================================================
    // Configure transforms
    // ==================================================

    player->GetTransform().SetPosition(
        glm::vec3(10.0f, 0.0f, 0.0f));

    player->GetTransform().SetScale(glm::vec3(2.0f, 2.0f, 2.0f));

    camera->GetTransform().SetPosition(
        glm::vec3(0.0f, 2.0f, -5.0f));

    weapon->GetTransform().SetPosition(
        glm::vec3(0.0f, 0.0f, 2.0f));

    enemy->GetTransform().SetPosition(
        glm::vec3(-5.0f, 0.0f, 10.0f));
    // ==================================================
    // Register Component types
    // ==================================================

    ComponentRegistry::RegisterComponent("ScriptComponent", []() { return std::make_unique<ScriptComponent>(); });
    ComponentRegistry::RegisterComponent("PlayerComponent", []() { return std::make_unique<PlayerComponent>(); });
    ComponentRegistry::RegisterComponent("EnemyComponent", []() { return std::make_unique<EnemyComponent>(); });

    // ==================================================
    // Add Components
    // ==================================================
	auto scriptComp = ComponentFactory::CreateComponent("ScriptComponent");
	auto playerComp = ComponentFactory::CreateComponent("PlayerComponent");   
	auto enemyComp = ComponentFactory::CreateComponent("EnemyComponent");    

    player->AttachComponent(scriptComp);
    player->AttachComponent(playerComp);
    enemy->AttachComponent(enemyComp);

	auto playerComponets = player->GetComponents<PlayerComponent>();
    auto enemyComponents = enemy->GetComponents<EnemyComponent>();
    enemyComponents[0]->SetTarget(playerComponets[0]);

    

    // ==================================================
    // Activate the scene
    // ==================================================

    sceneManager.ActivateScene("Game");

    // ==================================================
    // Game loop
    // ==================================================

    constexpr float deltaTime = 0.016f;

    for (int frame = 0; frame < 5; ++frame)
    {
        std::cout
            << "\n--- Frame "
            << frame
            << " ---\n";

        //sceneManager.Update(deltaTime);
    }

    //==================================================
    // Serialize
    //==================================================


    // Remember that we need to create Resource manager BEFORE we deserialize any meshComponents
    sceneManager.SerializeToFile(scene, "sceneJson.json");
    sceneManager.UnloadScene("Game");
    sceneManager.LoadFromFile("sceneJson.json");
    auto loadedScene = sceneManager.GetScene("Game");
    sceneManager.SerializeToFile(loadedScene, "sceneJson2.json");

    // ==================================================
    // Deactivate / unload
    // ==================================================


    return 0;
}
