#include <iostream>
#include <memory>

#include "SceneSystem/SceneManager.hpp"
#include "SceneSystem/Scene.hpp"
#include "SceneSystem/Node.hpp"
#include "SceneSystem/Component.hpp"

// --------------------------------------------------
// Test component
// --------------------------------------------------

class TestComponent : public Component
{
public:
    void Initialize() override
    {
        std::cout << "TestComponent initialized.\n";
        m_initialized = true;
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
    int m_updateCount = 0;
    float m_lastDeltaTime = 0.0f;
};


int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    SceneManager sceneManager;

    // --------------------------------------------------
    // Scene
    // --------------------------------------------------

    std::cout << "Loading Game scene...\n";

    if (!sceneManager.LoadScene("Game"))
    {
        std::cout << "Failed to load Game scene!\n";
        return 1;
    }

    std::cout << "Game scene loaded successfully.\n";

    if (!sceneManager.ActivateScene("Game"))
    {
        std::cout << "Failed to activate Game scene!\n";
        return 1;
    }

    std::cout << "Game scene activated.\n";


    // --------------------------------------------------
    // Get root
    // --------------------------------------------------

    std::shared_ptr<Scene> gameScene =
        sceneManager.GetScene("Game");

    std::shared_ptr<Node> root =
        gameScene->GetRoot();

    std::cout << "Root name: "
        << root->GetName()
        << "\n";


    // --------------------------------------------------
    // Create nodes
    // --------------------------------------------------

    auto player =
        std::make_shared<Node>("Player");

    auto weapon =
        std::make_shared<Node>("Weapon");

    auto enemy =
        std::make_shared<Node>("Enemy");


    // --------------------------------------------------
    // Build hierarchy
    // --------------------------------------------------

    root->AddChild(player);
    player->AddChild(weapon);
    root->AddChild(enemy);


    // --------------------------------------------------
    // Set transforms
    // --------------------------------------------------

    player->SetPosition(glm::vec3(10.0f, 0.0f, 0.0f));

    weapon->SetPosition(glm::vec3(0.0f, 0.0f, 2.0f));

    enemy->SetPosition(glm::vec3(-5.0f, 0.0f, 0.0f));


    // --------------------------------------------------
    // Component tests
    // --------------------------------------------------

    std::cout << "\n--- Component Tests ---\n";


    // --------------------------------------------------
    // Add component
    // --------------------------------------------------

    std::cout << "\nAdding TestComponent to Player...\n";

    auto testComponent =
        player->AddComponent<TestComponent>();

    if (testComponent)
    {
        std::cout << "TestComponent added successfully.\n";
    }
    else
    {
        std::cout << "Failed to add TestComponent.\n";
    }


    // --------------------------------------------------
    // Test initialization
    // --------------------------------------------------

    std::cout << "\nTesting initialization...\n";

    std::cout << "Initialized: "
        << testComponent->IsInitialized()
        << "\n";


    // --------------------------------------------------
    // Test component owner
    // --------------------------------------------------

    std::cout << "\nTesting component owner...\n";

    auto componentOwner =
        testComponent->GetOwner();

    if (componentOwner)
    {
        std::cout << "Component owner: "
            << componentOwner->GetName()
            << "\n";
    }
    else
    {
        std::cout << "Component has no owner!\n";
    }


    // --------------------------------------------------
    // Test GetComponent
    // --------------------------------------------------

    std::cout << "\nTesting GetComponent...\n";

    auto retrievedComponent =
        player->GetComponent<TestComponent>();

    if (retrievedComponent)
    {
        std::cout << "Component retrieved successfully.\n";
    }
    else
    {
        std::cout << "Failed to retrieve component.\n";
    }


    // --------------------------------------------------
    // Test component update
    // --------------------------------------------------

    std::cout << "\nTesting component update...\n";

    sceneManager.Update(0.016f);

    std::cout << "Update count: "
        << testComponent->GetUpdateCount()
        << "\n";

    std::cout << "Last delta time: "
        << testComponent->GetLastDeltaTime()
        << "\n";


    // --------------------------------------------------
    // Test component removal
    // --------------------------------------------------

    std::cout << "\nRemoving TestComponent...\n";

    bool removed =
        player->RemoveComponent<TestComponent>();

    std::cout << "Component removed: "
        << removed
        << "\n";


    // --------------------------------------------------
    // Verify removal
    // --------------------------------------------------

    auto componentAfterRemoval =
        player->GetComponent<TestComponent>();

    if (!componentAfterRemoval)
    {
        std::cout << "Component successfully removed.\n";
    }
    else
    {
        std::cout << "Component still exists!\n";
    }


    // --------------------------------------------------
    // Update
    // --------------------------------------------------

    sceneManager.Update(0.016f);


    // --------------------------------------------------
    // Print positions
    // --------------------------------------------------

    glm::vec3 playerWorld =
        player->GetPosition();

    glm::vec3 weaponWorld =
        weapon->GetPosition();

    glm::vec3 enemyWorld =
        enemy->GetPosition();

    std::cout << "\nPlayer local position:\n";
    std::cout << playerWorld.x << ", "
        << playerWorld.y << ", "
        << playerWorld.z << "\n";

    std::cout << "\nWeapon local position:\n";
    std::cout << weaponWorld.x << ", "
        << weaponWorld.y << ", "
        << weaponWorld.z << "\n";

    std::cout << "\nEnemy local position:\n";
    std::cout << enemyWorld.x << ", "
        << enemyWorld.y << ", "
        << enemyWorld.z << "\n";


    // --------------------------------------------------
    // Test removing a node
    // --------------------------------------------------

    std::cout << "\nRemoving Enemy...\n";

    root->RemoveChild(enemy);

    std::cout << "Children after removing Enemy: "
        << root->GetChildren().size()
        << "\n";


    // --------------------------------------------------
    // Test deactivation
    // --------------------------------------------------

    std::cout << "\nDeactivating Game scene...\n";

    sceneManager.DeactivateScene("Game");

    std::cout << "Game active: "
        << gameScene->IsActive()
        << "\n";


    // --------------------------------------------------
    // Test activation again
    // --------------------------------------------------

    std::cout << "\nActivating Game scene again...\n";

    sceneManager.ActivateScene("Game");

    std::cout << "Game active: "
        << gameScene->IsActive()
        << "\n";


    // --------------------------------------------------
    // Test unloading
    // --------------------------------------------------

    std::cout << "\nUnloading Game scene...\n";

    sceneManager.UnloadScene("Game");

    if (sceneManager.GetScene("Game") == nullptr)
    {
        std::cout << "Game scene successfully unloaded.\n";
    }

    return 0;
}