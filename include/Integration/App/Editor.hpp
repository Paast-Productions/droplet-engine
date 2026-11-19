/**
 * @file Editor.hpp
 * @brief Editor: the editor's windows and its one-off viewport camera. Editor builds only (_EDITOR).
 */
#pragma once
#ifdef _EDITOR

#include <cstdint>
#include <functional>
#include <span>
#include <string>
#include <vector>

#include "Bridge/Extract.hpp"
#include "Game/Scene.hpp"
#include "Render/PreparedFrame.hpp"

/// @brief What an editor window may use while it draws; the engine fills it each frame
struct EditorContext
{
    Game::Scene& scene;
    /// an ImGui texture id that shows render target or texture `image` (pass it to ImGui::Image); stable across frames
    std::function<std::uint64_t(Guid image)> targetTexture;
    const Render::PreparedFrame& lastFrame; ///< for stats: the views drawn last frame
};

/// @brief The runtime side of the editor: its windows, drawn between ImGuiFrame::Begin and End, and its own camera.
/// @details The editor camera is not part of the scene: it is handed to Bridge::Extract next to the scene and draws
///          into a target that follows its viewport window. The game's cameras keep their sizes; their windows show
///          them fitted.
class Editor
{
public:
    /// @brief Places the editor camera where the scene's presenting camera is
    void Init(const Game::Scene& scene);

    /// @brief The editor's windows. Size changes reach the renderer through the next frame's world
    void Draw(EditorContext& context);

    /// @brief The cameras the editor renders, for Bridge::Extract
    [[nodiscard]] std::span<const Bridge::ExternalCamera> Cameras() const noexcept { return m_cameras; }

    /// @brief The presented view fills the window behind the editor; false: the composite only clears
    [[nodiscard]] bool SceneBehindEditor() const noexcept { return m_sceneBehindEditor; }

private:
    void DrawStats(const EditorContext& context);
    /// @brief The editor camera's window: its target follows the window's size
    void DrawViewport(const EditorContext& context);
    /// @brief A game camera's output, fitted to the window at the target's own size; the camera is picked from a list
    void DrawGameView(const EditorContext& context);

    std::vector<Bridge::ExternalCamera> m_cameras; ///< [0]: the viewport camera
    bool m_sceneBehindEditor{ false };
    std::string m_gameCamera; ///< name of the scene camera the Game window shows
};

#endif
