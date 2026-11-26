#include <Engine/Engine.hpp>
#include <cstdio>
#include <ScriptSystem/ScriptSystem.hpp>
#include <tracy/public/tracy/Tracy.hpp>
#include <GameInput.hpp>
#include <chrono>

const glm::vec2 C_INITSIZE = { 1280, 720 };

int main([[maybe_unused]] int argc, [[maybe_unused]] char **argv)
{
    // TODO: (DEBUG) Check if Tracy is enabled and if so, sleep for a few seconds to allow the profiler to connect before starting the engine
    ZoneScopedN("Game"); // NOTE: Scoped Tracy calls should almost always be on the first line of their scope

    glm::vec2 windowSize = C_INITSIZE;

    Droplet::EngineConfig config{
        .WindowConfig = {
            .title = "Drone Boy Journey: Deep Sea Discoveries",
            .width = static_cast<int>(windowSize.x),
            .height = static_cast<int>(windowSize.y)
        },
        .UpdateFlags = Droplet::UpdateFlags::EditorFlags
    };
    Droplet::Engine engine(config);

    SDL_Window *wnd = engine.GetWindow();
    if (!wnd)
    {
        return 1;
    }

    bool run = true;

    // Set the script path for where the engine will look for scripts.
    Droplet::Script::ScriptSystem::Get().SetScriptPath(ROOT_PATH "/src/Scripts");

    engine.GetSceneManager().LoadFromFileAsync(ROOT_PATH "/GameData/Scenes/BoatScene.json");
    engine.GetSceneManager().LoadFromFileAsync(ROOT_PATH "/GameData/Scenes/InsideBoatScene.json");
    engine.GetSceneManager().LoadFromFileAsync(ROOT_PATH "/GameData/Scenes/PodScene.json");

    while (!engine.GetSceneManager().IsLoadingComplete())
    {
        engine.Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    engine.GetSceneManager().ActivateScene("BoatScene");

    //Do one update without game flags set to prevent physics components landing at (0,0,0)
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

    Droplet::EngineFlagsOwner::SetUpdateFlags(Droplet::UpdateFlags::GameFlags | Droplet::UpdateFlags::PlayModeFlags);

    bool mouseLock = true;
    bool fullscreen = true;

    Droplet::GameInput::Get().LockCursor(mouseLock, windowSize.x / 2, windowSize.y / 2, engine.GetWindow());

    FrameMark;

    // Run main loop
    while (run)
    {
        ZoneScopedN("Main Loop"); // NOTE: Scoped Tracy calls should almost always be on the first line of their scope

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

        std::vector<Droplet::Common::CameraRenderData> cameras = engine.GetSceneManager().GetRenderData();

        switch (engine.Render(cameras))
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

        if (Droplet::GameInput::Get().KeyPressed(Droplet::Key::KeyEscape))
        {
            SDL_SetWindowFullscreen(wnd, fullscreen);
            fullscreen = !fullscreen;
        }

        if (Droplet::GameInput::Get().KeyPressed(Droplet::Key::KeyTab))
        {
            Droplet::GameInput::Get().LockCursor(mouseLock, windowSize.x / 2, windowSize.y / 2, engine.GetWindow());
            mouseLock = !mouseLock;
        }
    }

    return 0;
}
