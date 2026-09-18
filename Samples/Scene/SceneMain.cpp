#include <iostream>
#include <memory>

#include "SceneSystem/SceneManager.hpp"
#include "SceneSystem/Scene.hpp"
#include "SceneSystem/Node.hpp"
#include "SceneSystem/Component.hpp"
#include "SceneSystem/MeshComponent.hpp"


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
    // MeshComponent tests
    // --------------------------------------------------

    std::cout << "\n--- MeshComponent Tests ---\n";


    // --------------------------------------------------
    // Add MeshComponent
    // --------------------------------------------------

    std::cout << "\nAdding MeshComponent to Player...\n";

    auto meshComponent =
        player->AddComponent<MeshComponent>(
            "Assets/Models/Player.mesh");

    if (meshComponent)
    {
        std::cout << "MeshComponent added successfully.\n";
    }
    else
    {
        std::cout << "Failed to add MeshComponent.\n";
    }


    // --------------------------------------------------
    // Test mesh path
    // --------------------------------------------------

    std::cout << "\nTesting mesh path...\n";

    std::cout << "Mesh path: "
        << meshComponent->GetMeshPath()
        << "\n";


    // --------------------------------------------------
    // Test owner
    // --------------------------------------------------

    std::cout << "\nTesting MeshComponent owner...\n";

    auto meshOwner =
        meshComponent->GetOwner();

    if (meshOwner)
    {
        std::cout << "Mesh owner: "
            << meshOwner->GetName()
            << "\n";
    }
    else
    {
        std::cout << "MeshComponent has no owner!\n";
    }


    // --------------------------------------------------
    // Test GetComponent
    // --------------------------------------------------

    std::cout << "\nTesting GetComponent<MeshComponent>...\n";

    auto retrievedMesh =
        player->GetComponents<MeshComponent>();

    if (!retrievedMesh.empty())
    {
        std::cout << "MeshComponent retrieved successfully.\n";
    }
    else
    {
        std::cout << "Failed to retrieve MeshComponent.\n";
    }


    // --------------------------------------------------
    // Test component removal
    // --------------------------------------------------

    std::cout << "\nRemoving MeshComponent...\n";

    bool meshRemoved =
        player->RemoveComponent<MeshComponent>();

    std::cout << "MeshComponent removed: "
        << meshRemoved
        << "\n";


    // --------------------------------------------------
    // Verify removal
    // --------------------------------------------------

    auto meshAfterRemoval =
        player->GetComponents<MeshComponent>();

    if (meshAfterRemoval.empty())
    {
        std::cout << "MeshComponent successfully removed.\n";
    }
    else
    {
        std::cout << "MeshComponent still exists!\n";
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