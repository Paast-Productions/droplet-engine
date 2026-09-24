#include <print>

#include "ScriptSystem/ScriptSystem.hpp"
#include "SceneSystem/SceneManager.hpp"
#include "SceneSystem/Scene.hpp"
#include "SceneSystem/Node.hpp"
#include "SceneSystem/Component.hpp"
#include "SceneSystem/Components/MeshComponent.hpp"
#include "SceneSystem/Components/ScriptComponent.hpp"

using namespace Droplet::Scene;

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

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
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

    camera->GetTransform().SetPosition(
        glm::vec3(0.0f, 2.0f, -5.0f));

    weapon->GetTransform().SetPosition(
        glm::vec3(0.0f, 0.0f, 2.0f));

    enemy->GetTransform().SetPosition(
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


    ScriptSystem::Get().SetScriptPath("../../../src/TestScripts");
    player->AddComponent<ScriptComponent>("testScript.lua");


    ScriptSystem::Get().Start();

    while (true)
    {
        ScriptSystem::Get().Update(1.0);
    }
    

    return 0;
}  