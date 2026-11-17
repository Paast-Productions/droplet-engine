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

	if (!p_outName)
	{
		throw std::invalid_argument("p_outName cannot be null");
	}

	bool selected = false;

	if (BeginPopup("Component List"))
	{
		static std::string filter = "";
		SearchBar(filter, ImVec2(100.0f, 0.0f), true);

		Separator();

		const auto &registry = ComponentRegistry::GetRegistry();

		for (const auto &[name, _] : registry)
		{
			if (!filter.empty())
			{
				std::string lowerName = name;
				std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

				if (lowerName.find(filter) == std::string::npos)
				{
					continue;
				}
			}

			if (Selectable(name.c_str()))
			{
				if (p_outName)
				{
					*p_outName = name;
				}

				CloseCurrentPopup();
				selected = true;
			}
		}
		EndPopup();
	}

	return selected;
}
