#include "ComponentList.hpp"
#include <ComponentRegistry.hpp>
#include <Component.hpp>
#include <ImGui/imgui.h>

using namespace Droplet::Editor::Scene;
using namespace Droplet::Scene;

void Droplet::Editor::Scene::OpenComponentList()
{
	using namespace ImGui;

	OpenPopup("Component List");
}

bool Droplet::Editor::Scene::ShowComponentList(std::string *p_outName)
{
	using namespace ImGui;

	if (BeginPopup("Component List"))
	{
		const auto &registry = ComponentRegistry::GetRegistry();

		for (const auto &[name, _] : registry)
		{
			if (Selectable(name.c_str()))
			{
				if (p_outName)
				{
					*p_outName = name;
				}

				EndPopup();
				return true;
			}
		}
		EndPopup();
	}
}
