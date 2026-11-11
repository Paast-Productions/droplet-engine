#include <Engine/Engine.hpp>
#include <cstdio>
#include <ScriptSystem/ScriptSystem.hpp>
#include <Graphics/VK/TestData.hpp>
#include <tracy/public/tracy/Tracy.hpp>
#include <GameInput.hpp>

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
    // TODO: Check if Tracy is enabled and if so, sleep for a few seconds to allow the profiler to connect before starting the engine
    ZoneScopedN("Game"); // NOTE: Scoped Tracy calls should almost always be on the first line of their scope

    glm::vec2 windowSize = glm::vec2(1280, 720);

    Droplet::EngineConfig config{
        Droplet::Graphics::SDL::WindowConfig{
            static_cast<int>(windowSize.x),
            static_cast<int>(windowSize.y),
            {SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE},
            "Drone Boy Adventures: THALASSA"
        },
        Droplet::UpdateFlags::GameFlags
    };
    Droplet::Engine engine(config);

    SDL_Window *wnd = engine.GetWindow();
    if (!wnd)
    {
        return 1;
    }

    // Set the script path for where the engine will look for scripts.
    Droplet::Script::ScriptSystem::Get().SetScriptPath(ROOT_PATH "/src/Scripts");

    engine.GetSceneManager().LoadFromFile(ROOT_PATH "/GameData/Scenes/BoatScene.json");

    engine.GetSceneManager().ActivateScene("BoatScene");

    FrameMark;

    // Run main loop
    bool run = true;
    while (run)
    {
        ZoneScopedN("Main Loop"); // NOTE: Scoped Tracy calls should almost always be on the first line of their scope

        //Get game camera?
        switch (engine.Update())
        {
        case Droplet::DROPLET_RETURNTYPE::OK:
            break;
        
        case Droplet::DROPLET_RETURNTYPE::EXIT:
            run = false;
            break;

        default:
        case Droplet::DROPLET_RETURNTYPE::EXIT_ERROR:
            run = false;
            break;
        }

        //Get game camera?
        switch (engine.Render())
        {
        case Droplet::DROPLET_RETURNTYPE::OK:
            break;
        
        case Droplet::DROPLET_RETURNTYPE::EXIT:
            run = false;
            break;

        default:
        case Droplet::DROPLET_RETURNTYPE::EXIT_ERROR:
            run = false;
            break;
        }
    }

    return 0;
}
