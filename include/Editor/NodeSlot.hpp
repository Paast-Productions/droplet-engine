#pragma once
#ifndef _GAME
#include <string>
#include <memory>
#include <optional>
#include <functional>
#include <ImGui/imgui.h>

namespace Droplet::Scene
{
    class Node;
}

namespace Droplet::Editor
{
    bool NodeSlot(
        const char *p_label,
        std::weak_ptr<Droplet::Scene::Node> &p_node,
		ImVec2 p_buttonSize = ImVec2(0, 0),
        std::optional<std::function<bool(Droplet::Scene::Node*)>> p_acceptCallback = std::nullopt,
        std::optional<std::function<void()>> p_onClickCallback = std::nullopt,
        const char *p_noAcceptMsg = "Cannot accept this node");
}
#endif
