#include "SavedScenesWindow.hpp"

Droplet::Editor::SavedScenesWindow::SavedScenesWindow()
{
    RefreshScannedFiles(m_sceneFiles);
}

void Droplet::Editor::SavedScenesWindow::InitImpl()
{
    SetName("Saved Scenes");
    ToggleToolbar(true);
}

void Droplet::Editor::SavedScenesWindow::RenderImpl()
{
}

void Droplet::Editor::SavedScenesWindow::CloseImpl()
{
}

void Droplet::Editor::SavedScenesWindow::RenderToolbar()
{
}

void Droplet::Editor::SavedScenesWindow::RefreshScannedFiles(std::vector<std::string> &p_sceneFiles)
{

    static std::string selectedScene;
    const std::filesystem::path scenesDirectory = std::filesystem::current_path() / "Scenes";

    p_sceneFiles.clear();

    std::error_code error;

    if (std::filesystem::exists(scenesDirectory, error) && std::filesystem::is_directory(scenesDirectory, error))
    {
        for (const std::filesystem::directory_entry &entry : std::filesystem::directory_iterator(scenesDirectory, error))
        {
            if (entry.is_regular_file())
            {
                p_sceneFiles.push_back(
                    entry.path().filename().string()
                );
            }
        }
    }

    std::sort(p_sceneFiles.begin(), p_sceneFiles.end());


    if (std::find(
        p_sceneFiles.begin(),
        p_sceneFiles.end(),
        selectedScene) == p_sceneFiles.end())
    {
        selectedScene.clear();
    }
}
