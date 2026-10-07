#include <print>

#include "SceneSystem/Behaviours/ScriptBehaviour.hpp"
#include "ScriptSystem/ScriptSystem.hpp"
#include "SceneSystem/SceneManager.hpp"
#include "SceneSystem/Scene.hpp"
#include "SceneSystem/Node.hpp"
#include "SceneSystem/Component.hpp"
#include "SceneSystem/Components/MeshComponent.hpp"
#include "SceneSystem/Components/ScriptComponent.hpp"
#include "GameInput.hpp"
#include "ImGui/imgui.h"

#include "Time.hpp"

using namespace Droplet::Scene;

class PlayerComponent : public Component
{
public:
    std::string_view GetTypeName() override { return "PlayerComponent"; }

    void Start() override
    {
        std::cout << "PlayerComponent started\n";
    }

    void Update([[maybe_unused]] float p_deltaTime) override
    {
        std::cout
            << "PlayerComponent updating: "
            << p_deltaTime
            << " seconds\n";
    }
};

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    Droplet::Time &time = time.Get();
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
    [[maybe_unused]] SDL_Window *window = SDL_CreateWindow(
        "Droplet",
        1280,
        720,
        SDL_WINDOW_VULKAN); 
    sceneManager.ActivateScene("Game");

    //ScriptBehaviour scriptBehaviour("testScript.lua");

    ScriptSystem::Get().SetScriptPath("../../../src/Scripts");
    player->AddComponent<ScriptComponent>("testScript.lua");
    
    ScriptSystem::Get().Start();


    player->RenderUI();


    while (true)
    {
        Droplet::GameInput::Get().Update();

		SDL_Event event;
        while (SDL_PollEvent(&event))
        {
			Droplet::GameInput::Get().ProcessEvent(event);
        }

        if (Droplet::GameInput::Get().KeyPressed(Droplet::Key::KeySpace))
        {
            std::print("Space key pressed\n");
        }
        //ScriptSystem::Get().Update(time.GetDeltaTime());
    }

    return 0;
}  