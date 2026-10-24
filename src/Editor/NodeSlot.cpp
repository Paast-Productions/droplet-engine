#include "Editor/NodeSlot.hpp"
#include <ImGui/imgui.h>

using namespace Droplet::Editor;
using namespace Droplet::Scene;

bool Droplet::Editor::NodeSlot(
    const char *p_label,
    std::weak_ptr<Node> &p_current,
    const std::function<bool(std::shared_ptr<Node>)> &p_acceptFunc,
    ImVec2 p_size)
{
    using namespace ImGui;

	// TODO: Steal and modify Alexander's ResourceSlot

    return false;
}
