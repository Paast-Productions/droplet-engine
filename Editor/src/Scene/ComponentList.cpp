#include "ComponentList.hpp"
#include <ComponentRegistry.hpp>
#include <ImGui/imgui.h>

using namespace Droplet::Editor::Scene;
using namespace Droplet::Scene;

bool Droplet::Editor::Scene::OpenComponentList(std::string *p_outName)
{
	using namespace ImGui;

	const auto &registry = ComponentRegistry::GetRegistry();

	// TODO: Open a popup window at the cursor with a list of registered components and allow the user to select one as well as search through them.
}
