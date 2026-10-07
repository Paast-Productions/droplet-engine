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

#include <Graphics/VK/Renderer.hpp>
#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_sdl3.h>
#include <ImGui/imgui_impl_vulkan.h>
#include <SDL3/SDL.h>

#include "Time.hpp"
#include "SceneSystem/SceneSerializer.hpp"

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
    Droplet::Time& time = Droplet::Time::Get();

    // ==================================================
    // Renderer and ImGui
    // ==================================================

    Droplet::Graphics::SDL::WindowConfig windowConfig =
    {
        .Width = 1280,
        .Height = 720,
        .Flags = 0
    };

    Droplet::Graphics::Renderer renderer(windowConfig);

    SDL_Window* window = renderer.GetWindow();

    if (window == nullptr)
    {
        std::cerr << "Failed to create renderer window\n";
        return 1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplSDL3_InitForVulkan(window);

    ImGui_ImplVulkan_InitInfo initInfo{};
    renderer.GetImGuiInitInfo(initInfo);
    ImGui_ImplVulkan_Init(&initInfo);

    // ==================================================
    // Scene
    // ==================================================

    SceneManager sceneManager;
    sceneManager.LoadScene("Game");

    auto scene = sceneManager.GetScene("Game");

    if (!scene)
    {
        std::cerr << "Failed to load Game scene\n";
        return 1;
    }

    auto root = scene->AddNode("Root");

    auto player = root->AddChild(
        scene->AddNode("Player"));

    auto camera = player->AddChild(
        scene->AddNode("Camera"));

    auto weapon = player->AddChild(
        scene->AddNode("Weapon"));

    auto enemy = root->AddChild(
        scene->AddNode("Enemy"));

    player->GetTransform().SetPosition(
        glm::vec3(10.0f, 0.0f, 0.0f));

    camera->GetTransform().SetPosition(
        glm::vec3(0.0f, 2.0f, -5.0f));

    weapon->GetTransform().SetPosition(
        glm::vec3(0.0f, 0.0f, 2.0f));

    enemy->GetTransform().SetPosition(
        glm::vec3(-5.0f, 0.0f, 10.0f));

    // ==================================================
    // Components and scripting
    // ==================================================

    player->AddComponent<PlayerComponent>();

    player->AddComponent<MeshComponent>(
        "Meshes/Player.obj");

    enemy->AddComponent<MeshComponent>(
        "Meshes/Enemy.obj");

    auto& scriptSystem = Droplet::Script::ScriptSystem::Get();

    scriptSystem.SetScriptPath("../../../src/Scripts");

    player->AddComponent<ScriptComponent>(
        "testScript.lua");

    /*
     * Activating the scene calls Node::Start().
     *
     * Node::Start()
     * -> ScriptComponent::Start()
     * -> CreateComponentScript()
     * -> ActivateComponentScript()
     */
    sceneManager.ActivateScene("Game");

    /*
     * Calls Lua OnStart() on all activated script instances.
     */
    scriptSystem.Start();

    // ==================================================
    // Main loop
    // ==================================================

    bool running = true;

    while (running)
    {
        Droplet::GameInput::Get().Update();

        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            Droplet::GameInput::Get().ProcessEvent(event);

            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }
        }

        scriptSystem.Update(time.GetDeltaTime());

        // Begin an ImGui frame.
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // The C++ side owns the window.
        ImGui::Begin("Lua ImGui Binding Test");

        /*
         * Node::RenderUI()
         * -> Component::RenderUI()
         * -> ScriptComponent::RenderInternalUI()
         * -> Call("RenderUI")
         * -> Lua RenderUI()
         */
        player->RenderUI();

        ImGui::End();

        /*
         * DrawFrame() calls:
         *
         * ImGui::EndFrame()
         * ImGui::Render()
         * ImGui_ImplVulkan_RenderDrawData(...)
         */
        renderer.DrawFrame();
    }

    // ==================================================
    // Shutdown
    // ==================================================

    renderer.WaitIdle();

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    return 0;
}
