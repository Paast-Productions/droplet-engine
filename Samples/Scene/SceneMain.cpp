#include <iostream>
#include <memory>

#include "SceneSystem/SceneManager.hpp"
#include "SceneSystem/Scene.hpp"
#include "SceneSystem/Node.hpp"
#include "SceneSystem/Component.hpp"
#include "SceneSystem/Components/MeshComponent.hpp"


int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    SceneManager sceneManager;

    std::cout << "Loading Game scene...\n";

    if (!sceneManager.LoadScene("Game"))
    {
        std::cout << "Failed to load Game scene.\n";
        return 1;
    }

    if (!sceneManager.ActivateScene("Game"))
    {
        std::cout << "Failed to activate Game scene.\n";
        return 1;
    }

    auto scene = sceneManager.GetScene("Game");

    if (!scene)
    {
        std::cout << "Failed to retrieve Game scene.\n";
        return 1;
    }

    auto root = scene->GetRoot();

    if (!root)
    {
        std::cout << "Root does not exist.\n";
        return 1;
    }

    std::cout << "\n--- Scene Reference Tests ---\n";

    // --------------------------------------------------
    // Test 1: Root knows its Scene
    // --------------------------------------------------

    std::cout << "\nTesting root Scene reference...\n";

    auto rootScene = root->GetScene();

    if (rootScene == scene)
    {
        std::cout << "Root correctly references Game scene.\n";
    }
    else
    {
        std::cout << "ERROR: Root does not reference Game scene.\n";
    }

    // --------------------------------------------------
    // Test 2: Child inherits Scene
    // --------------------------------------------------

    std::cout << "\nTesting child Scene reference...\n";

    auto player = root->AddChild(
        std::make_shared<Node>("Player")
    );

    auto playerScene = player->GetScene();

    if (playerScene == scene)
    {
        std::cout << "Player correctly references Game scene.\n";
    }
    else
    {
        std::cout << "ERROR: Player does not reference Game scene.\n";
    }

    // --------------------------------------------------
    // Test 3: Grandchild inherits Scene
    // --------------------------------------------------

    std::cout << "\nTesting grandchild Scene reference...\n";

    auto weapon = player->AddChild(
        std::make_shared<Node>("Weapon")
    );

    auto weaponScene = weapon->GetScene();

    if (weaponScene == scene)
    {
        std::cout << "Weapon correctly references Game scene.\n";
    }
    else
    {
        std::cout << "ERROR: Weapon does not reference Game scene.\n";
    }

    // --------------------------------------------------
    // Test 4: Child added after hierarchy exists
    // --------------------------------------------------

    std::cout << "\nTesting dynamically added child...\n";

    auto camera = player->AddChild(
        std::make_shared<Node>("Camera")
    );

    auto cameraScene = camera->GetScene();

    if (cameraScene == scene)
    {
        std::cout << "Camera correctly references Game scene.\n";
    }
    else
    {
        std::cout << "ERROR: Camera does not reference Game scene.\n";
    }

    // --------------------------------------------------
    // Test 5: Verify entire hierarchy
    // --------------------------------------------------

    std::cout << "\n--- Hierarchy Scene References ---\n";

    std::cout << root->GetName()
        << ": "
        << (root->GetScene() == scene ? "Game" : "ERROR")
        << '\n';

    std::cout << player->GetName()
        << ": "
        << (player->GetScene() == scene ? "Game" : "ERROR")
        << '\n';

    std::cout << weapon->GetName()
        << ": "
        << (weapon->GetScene() == scene ? "Game" : "ERROR")
        << '\n';

    std::cout << camera->GetName()
        << ": "
        << (camera->GetScene() == scene ? "Game" : "ERROR")
        << '\n';

    // --------------------------------------------------
    // Cleanup
    // --------------------------------------------------

    sceneManager.UnloadScene("Game");

    return 0;

}