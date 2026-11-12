#include "EffectList.hpp"
#include <EffectRegistry.hpp>
#include <Effect.hpp>
#include <ImGui/imgui.h>

using namespace Droplet::Editor;
using namespace Droplet::Audio;

void Droplet::Editor::OpenEffectList()
{
	ImGui::OpenPopup("Effect List");
}

bool Droplet::Editor::ShowEffectList(std::string *p_outputName)
{

	if (!p_outputName)
	{
		throw std::invalid_argument(" p_outputname cannot be null");
	}

	bool selected = false;

	if (ImGui::BeginPopup("Effect List"))
	{
		const auto &registry = EffectRegistry::GetEffectRegistry();

		for (const auto &[name, _] : registry)
		{
			if (ImGui::Selectable(name.c_str()))
			{
				if (p_outputName)
				{
					*p_outputName = name;
				}
				ImGui::CloseCurrentPopup();
				selected = true;
			}
		}
		ImGui::EndPopup();
	}

	return selected;
}
