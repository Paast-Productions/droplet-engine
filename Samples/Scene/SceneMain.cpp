#include <iostream>
#include <memory>
#include <random>

#include "SceneSystem/SceneManager.hpp"
#include "SceneSystem/Scene.hpp"
#include "SceneSystem/Node.hpp"
#include "SceneSystem/Component.hpp"
#include "SceneSystem/Components/MeshComponent.hpp"
#include "SceneSystem/Components/ScriptComponent.hpp"
#include "SceneSystem/SceneSerializer.hpp"
#include "Core/IoManager.hpp"
#include "GameInput.hpp"
#include <SDL3/SDL_init.h>

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

    std::shared_ptr<Scene> s = sm.GetScene("Game");
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
    while (true)
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
            const std::vector<std::shared_ptr<Node>> roots = s->GetRoots();
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

// --------------------------------------------------
// Example Component
// --------------------------------------------------

class PlayerComponent : public Component
{
public:
    std::string_view GetTypeName() override { return "PlayerComponent"; }

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

    nlohmann::json SerializeImpl() override
    {
        nlohmann::json json;
        json["type"] = "PlayerComponent";
        return json;
    }
};


// --------------------------------------------------
// Main
// --------------------------------------------------

int main()
{
    OctreeTest();

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

    //==================================================
    // Serialize
    //==================================================

    SceneSerializer seri;
    nlohmann::json json = seri.SerializeScene(sceneManager.GetScene("Game"));
    Droplet::Core::JsonIO::Write("testJson.json", json);

    // ==================================================
    // Deactivate / unload
    // ==================================================


    return 0;
}