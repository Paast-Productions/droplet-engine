#include <iostream>
#include <memory>
#include <string>

#include "SceneSystem/SceneManager.hpp"
#include "SceneSystem/Scene.hpp"
#include "SceneSystem/Node.hpp"
#include "SceneSystem/Component.hpp"
#include "SceneSystem/Components/MeshComponent.hpp"


// --------------------------------------------------
// Test Component
// --------------------------------------------------

class TestComponent : public Component
{
public:

    void Initialize() override
    {
        m_initialized = true;
    }

    void OnStart() override
    {
        m_started = true;
        ++m_startCount;
    }

    void Update(float p_deltaTime) override
    {
        m_updateCount++;
        m_lastDeltaTime = p_deltaTime;
    }

    bool IsInitialized() const
    {
        return m_initialized;
    }

    bool HasStarted() const
    {
        return m_started;
    }

    int GetStartCount() const
    {
        return m_startCount;
    }

    int GetUpdateCount() const
    {
        return m_updateCount;
    }

    float GetLastDeltaTime() const
    {
        return m_lastDeltaTime;
    }

private:

    bool m_initialized = false;
    bool m_started = false;

    int m_startCount = 0;
    int m_updateCount = 0;

    float m_lastDeltaTime = 0.0f;
};


// --------------------------------------------------
// Test Helpers
// --------------------------------------------------

int g_testsPassed = 0;
int g_testsFailed = 0;

void Check(bool p_condition, const std::string& p_testName)
{
    if (p_condition)
    {
        std::cout << "[PASS] " << p_testName << '\n';
        ++g_testsPassed;
    }
    else
    {
        std::cout << "[FAIL] " << p_testName << '\n';
        ++g_testsFailed;
    }
}


// --------------------------------------------------
// Main
// --------------------------------------------------

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    SceneManager sceneManager;

    // ==================================================
    // Scene Manager Tests
    // ==================================================

    std::cout << "\n========================================\n";
    std::cout << "Scene Manager Tests\n";
    std::cout << "========================================\n";

    // --------------------------------------------------
    // Load Scene
    // --------------------------------------------------

    std::cout << "\nLoading Game scene...\n";

    Check(
        sceneManager.LoadScene("Game"),
        "Load Game scene"
    );

    // Attempt to load the same scene again.
    Check(
        !sceneManager.LoadScene("Game"),
        "Prevent loading duplicate scene"
    );

    // --------------------------------------------------
    // Get Scene
    // --------------------------------------------------

    auto scene = sceneManager.GetScene("Game");

    Check(
        scene != nullptr,
        "Retrieve loaded Game scene"
    );

    Check(
        scene->GetName() == "Game",
        "Scene has correct name"
    );

    Check(
        scene->IsLoaded(),
        "Scene is marked as loaded"
    );

    Check(
        !scene->IsActive(),
        "Scene is initially inactive"
    );

    // Try retrieving a scene that doesn't exist.
    Check(
        sceneManager.GetScene("DoesNotExist") == nullptr,
        "Return nullptr for unknown scene"
    );

    // --------------------------------------------------
    // Root
    // --------------------------------------------------

    auto root = scene->GetRoot();

    Check(
        root != nullptr,
        "Scene has a root Node"
    );

    Check(
        root->GetName() == "Root",
        "Root has correct name"
    );

    // ==================================================
    // Scene Reference Tests
    // ==================================================

    std::cout << "\n========================================\n";
    std::cout << "Scene Reference Tests\n";
    std::cout << "========================================\n";

    // The root should not have a Scene before activation
    // only if your implementation assigns the Scene during
    // Scene::Load(). With the implementation discussed earlier,
    // this should already be valid.

    Check(
        root->GetScene() == scene,
        "Root references its Scene"
    );

    // --------------------------------------------------
    // Create hierarchy
    // --------------------------------------------------

    auto player = root->AddChild(
        std::make_shared<Node>("Player")
    );

    auto weapon = player->AddChild(
        std::make_shared<Node>("Weapon")
    );

    auto camera = player->AddChild(
        std::make_shared<Node>("Camera")
    );

    auto enemy = root->AddChild(
        std::make_shared<Node>("Enemy")
    );

    // --------------------------------------------------
    // Check hierarchy
    // --------------------------------------------------

    Check(
        root->GetChildren().size() == 2,
        "Root contains Player and Enemy"
    );

    Check(
        player->GetChildren().size() == 2,
        "Player contains Weapon and Camera"
    );

    Check(
        player->GetParent() == root,
        "Player has Root as parent"
    );

    Check(
        weapon->GetParent() == player,
        "Weapon has Player as parent"
    );

    Check(
        camera->GetParent() == player,
        "Camera has Player as parent"
    );

    Check(
        enemy->GetParent() == root,
        "Enemy has Root as parent"
    );

    // --------------------------------------------------
    // Scene reference propagation
    // --------------------------------------------------

    Check(
        player->GetScene() == scene,
        "Player inherits Scene reference"
    );

    Check(
        weapon->GetScene() == scene,
        "Weapon inherits Scene reference"
    );

    Check(
        camera->GetScene() == scene,
        "Camera inherits Scene reference"
    );

    Check(
        enemy->GetScene() == scene,
        "Enemy inherits Scene reference"
    );

    // ==================================================
    // Transform Tests
    // ==================================================

    std::cout << "\n========================================\n";
    std::cout << "Transform Tests\n";
    std::cout << "========================================\n";

    player->SetPosition(glm::vec3(10.0f, 0.0f, 0.0f));
    weapon->SetPosition(glm::vec3(0.0f, 0.0f, 2.0f));

    Check(
        player->GetPosition() == glm::vec3(10.0f, 0.0f, 0.0f),
        "Player local position"
    );

    Check(
        weapon->GetPosition() == glm::vec3(0.0f, 0.0f, 2.0f),
        "Weapon local position"
    );  

    // ==================================================
    // Component Tests
    // ==================================================

    std::cout << "\n========================================\n";
    std::cout << "Component Tests\n";
    std::cout << "========================================\n";

    // --------------------------------------------------
    // Add Component
    // --------------------------------------------------

    std::cout << "\nAdding TestComponent to Player...\n";

    auto testComponent =
        player->AddComponent<TestComponent>();

    Check(
        testComponent != nullptr,
        "Component was created"
    );

    // --------------------------------------------------
    // Initialization
    // --------------------------------------------------

    Check(
        testComponent->IsInitialized(),
        "Component Initialize() was called"
    );

    // --------------------------------------------------
    // Owner
    // --------------------------------------------------

    Check(
        testComponent->GetOwner() == player,
        "Component has correct owner"
    );

    // --------------------------------------------------
    // GetComponents
    // --------------------------------------------------

    auto components =
        player->GetComponents<TestComponent>();

    Check(
        components.size() == 1,
        "GetComponents finds TestComponent"
    );

    if (!components.empty())
    {
        Check(
            components[0] == testComponent,
            "GetComponents returns correct component"
        );
    }

    // ==================================================
    // OnStart Tests
    // ==================================================

    std::cout << "\n========================================\n";
    std::cout << "OnStart Tests\n";
    std::cout << "========================================\n";

    // Scene is currently inactive.
    Check(
        !testComponent->HasStarted(),
        "Component has not started while Scene is inactive"
    );

    // Activate Scene.
    Check(
        sceneManager.ActivateScene("Game"),
        "Activate Game scene"
    );

    Check(
        scene->IsActive(),
        "Scene is active"
    );

    Check(
        testComponent->HasStarted(),
        "Component OnStart() was called"
    );

    Check(
        testComponent->GetStartCount() == 1,
        "Component OnStart() called exactly once"
    );

    // ==================================================
    // Update Tests
    // ==================================================

    std::cout << "\n========================================\n";
    std::cout << "Update Tests\n";
    std::cout << "========================================\n";

    constexpr float deltaTime = 0.016f;

    sceneManager.Update(deltaTime);

    Check(
        testComponent->GetUpdateCount() == 1,
        "Component Update() was called"
    );

    Check(
        testComponent->GetLastDeltaTime() == deltaTime,
        "Component received correct delta time"
    );

    // Update again.
    sceneManager.Update(deltaTime);

    Check(
        testComponent->GetUpdateCount() == 2,
        "Component updates every frame"
    );

    // ==================================================
    // Multiple Components Of Same Type
    // ==================================================

    std::cout << "\n========================================\n";
    std::cout << "Multiple Component Tests\n";
    std::cout << "========================================\n";

    auto secondTestComponent =
        player->AddComponent<TestComponent>();

    Check(
        secondTestComponent != nullptr,
        "Second TestComponent was created"
    );

    auto allTestComponents =
        player->GetComponents<TestComponent>();

    Check(
        allTestComponents.size() == 2,
        "Node supports multiple Components of the same type"
    );

    Check(
        secondTestComponent->IsInitialized(),
        "Second Component was initialized"
    );

    // Since Player has already started, the newly added
    // Component should start immediately.
    Check(
        secondTestComponent->HasStarted(),
        "Component added after Node start receives OnStart()"
    );

    Check(
        secondTestComponent->GetStartCount() == 1,
        "Late-added Component receives OnStart() exactly once"
    );

    // ==================================================
    // MeshComponent Tests
    // ==================================================

    std::cout << "\n========================================\n";
    std::cout << "MeshComponent Tests\n";
    std::cout << "========================================\n";

    auto meshComponent =
        player->AddComponent<MeshComponent>(
            "Meshes/Player.obj"
        );

    Check(
        meshComponent != nullptr,
        "MeshComponent was created"
    );

    Check(
        meshComponent->GetMeshPath() == "Meshes/Player.obj",
        "MeshComponent stores correct mesh path"
    );

    Check(
        meshComponent->GetOwner() == player,
        "MeshComponent has correct owner"
    );

    // ==================================================
    // Node Removal Tests
    // ==================================================

    std::cout << "\n========================================\n";
    std::cout << "Node Removal Tests\n";
    std::cout << "========================================\n";

    Check(
        root->GetChildren().size() == 2,
        "Root initially contains two children"
    );

    bool removed = root->RemoveChild(enemy);

    if (removed)
    {
        std::cout << "[PASS] Enemy was removed from Root\n";
    }

    Check(
        root->GetChildren().size() == 1,
        "Enemy was removed from Root"
    );

    Check(
        enemy->GetParent() == nullptr,
        "Removed Enemy no longer has a parent"
    );

    // ==================================================
    // Component Removal Tests
    // ==================================================

    std::cout << "\n========================================\n";
    std::cout << "Component Removal Tests\n";
    std::cout << "========================================\n";

    Check(
        player->RemoveComponent<TestComponent>(),
        "Remove first TestComponent"
    );

    allTestComponents =
        player->GetComponents<TestComponent>();

    Check(
        allTestComponents.size() == 1,
        "One TestComponent remains after removal"
    );

    player->RemoveComponent<TestComponent>();

    Check(
        player->GetComponents<TestComponent>().empty(),
        "All TestComponents were removed"
    );

    // There should now be no TestComponents.
    allTestComponents =
        player->GetComponents<TestComponent>();

    Check(
        allTestComponents.empty(),
        "All TestComponents were removed"
    );

    // ==================================================
    // Scene Activation Tests
    // ==================================================

    std::cout << "\n========================================\n";
    std::cout << "Scene Activation Tests\n";
    std::cout << "========================================\n";

    Check(
        sceneManager.DeactivateScene("Game"),
        "Deactivate Game scene"
    );

    Check(
        !scene->IsActive(),
        "Game scene is inactive"
    );

    // Store current update count.
    const int updateCountBeforeInactive =
        secondTestComponent->GetUpdateCount();

    sceneManager.Update(deltaTime);

    Check(
        secondTestComponent->GetUpdateCount()
        == updateCountBeforeInactive,
        "Inactive Scene does not update Components"
    );

    // Reactivate.
    Check(
        sceneManager.ActivateScene("Game"),
        "Reactivate Game scene"
    );

    Check(
        scene->IsActive(),
        "Game scene is active again"
    );

    // OnStart should NOT happen again.
    Check(
        secondTestComponent->GetStartCount() == 1,
        "Reactivating Scene does not call OnStart() twice"
    );

    // ==================================================
    // Node Active Tests
    // ==================================================

    std::cout << "\n========================================\n";
    std::cout << "Node Activation Tests\n";
    std::cout << "========================================\n";

    // Add a new component to Player.
    auto activeTestComponent =
        player->AddComponent<TestComponent>();

    Check(
        activeTestComponent->HasStarted(),
        "Active Node starts newly added Component"
    );

    player->SetActive(false);

    const int updateCountBeforeNodeInactive =
        activeTestComponent->GetUpdateCount();

    sceneManager.Update(deltaTime);

    Check(
        activeTestComponent->GetUpdateCount()
        == updateCountBeforeNodeInactive,
        "Inactive Node does not update Components"
    );

    player->SetActive(true);

    sceneManager.Update(deltaTime);

    Check(
        activeTestComponent->GetUpdateCount()
            > updateCountBeforeNodeInactive,
        "Reactivated Node updates Components"
    );

    // ==================================================
    // Dynamic Child OnStart Test
    // ==================================================

    std::cout << "\n========================================\n";
    std::cout << "Dynamic Child Tests\n";
    std::cout << "========================================\n";

    auto dynamicNode =
        player->AddChild(
            std::make_shared<Node>("DynamicNode")
        );

    Check(
        dynamicNode->GetScene() == scene,
        "Dynamically added Node receives Scene reference"
    );

    auto dynamicComponent =
        dynamicNode->AddComponent<TestComponent>();

    Check(
        dynamicComponent->IsInitialized(),
        "Dynamic Node Component is initialized"
    );

    Check(
        dynamicComponent->HasStarted(),
        "Dynamic Node Component receives OnStart()"
    );

    // ==================================================
    // Unload Tests
    // ==================================================

    std::cout << "\n========================================\n";
    std::cout << "Unload Tests\n";
    std::cout << "========================================\n";

    sceneManager.UnloadScene("Game");

    Check(
        sceneManager.GetScene("Game") == nullptr,
        "Game scene was removed from SceneManager"
    );

    // ==================================================
    // Results
    // ==================================================

    std::cout << "\n========================================\n";
    std::cout << "Test Results\n";
    std::cout << "========================================\n";

    std::cout << "Tests passed: " << g_testsPassed << '\n';
    std::cout << "Tests failed: " << g_testsFailed << '\n';

    if (g_testsFailed == 0)
    {
        std::cout << "\nAll tests passed!\n";
        return 0;
    }

    std::cout << "\nSome tests failed.\n";
    return 1;
}