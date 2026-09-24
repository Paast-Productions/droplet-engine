#include <iostream>
#include <memory>

#include "ScriptSystem/ScriptSystem.hpp"
#include "SceneSystem/SceneManager.hpp"
#include "SceneSystem/Scene.hpp"
#include "SceneSystem/Node.hpp"
#include "SceneSystem/Component.hpp"
#include "SceneSystem/Components/MeshComponent.hpp"
#include "SceneSystem/Components/ScriptComponent.hpp"


// --------------------------------------------------
// Example Component
// --------------------------------------------------

class PlayerComponent : public Component
{
public:
    void Start() override
    {
        std::cout << "PlayerComponent started\n";
    }

    void Update(float p_deltaTime) override
    {
        std::cout
            << "PlayerComponent updating: "
            << p_deltaTime
            << " seconds\n";
    }
};


// --------------------------------------------------
// Main
// --------------------------------------------------

int main()
{
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

    auto root =
        scene->AddRoot(
            std::make_shared<Node>("Root"));

    // ==================================================
    // Build the scene hierarchy
    // ==================================================

    auto player =
        root->AddChild(
            std::make_shared<Node>("Player"));

    auto camera =
        player->AddChild(
            std::make_shared<Node>("Camera"));

    auto weapon =
        player->AddChild(
            std::make_shared<Node>("Weapon"));

    auto enemy =
        root->AddChild(
            std::make_shared<Node>("Enemy"));     

    // ==================================================
    // Configure transforms
    // ==================================================

    player->SetPosition(
        glm::vec3(10.0f, 0.0f, 0.0f));

    camera->SetPosition(
        glm::vec3(0.0f, 2.0f, -5.0f));

    weapon->SetPosition(
        glm::vec3(0.0f, 0.0f, 2.0f));

    enemy->SetPosition(
        glm::vec3(-5.0f, 0.0f, 10.0f));

    // ==================================================
    // Add Components
    // ==================================================

    player->AddComponent<PlayerComponent>();

    player->AddComponent<MeshComponent>(
        "Meshes/Player.obj");

    enemy->AddComponent<MeshComponent>(
        "Meshes/Enemy.obj");

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

        sceneManager.Update(deltaTime);
    }

    // ==================================================
    // Deactivate / unload
    // ==================================================
    ScriptSystem scriptsystem;

    player->AddComponent<ScriptComponent>("testScript.lua");

    sceneManager.DeactivateScene("Game");

    sceneManager.UnloadScene("Game");

    return 0;
}