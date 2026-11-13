#include "Core/Layer.hpp"
#include "json/json.hpp"
#include "Core/IoManager.hpp"
#include "ImGui/imgui.h"
#include "ImGui/imgui_stdlib.h"
#include <IconFontCppHeaders/IconsLucide.h>

using namespace Droplet::Core;
using namespace Droplet::Core::JsonIO;
using namespace nlohmann;

void LayerUtility::InternalSerializeLayerNames(const std::string &p_path) const
{
	json doc = json::object();

	for (const auto &[layer, name] : m_layerNames)
	{
		doc[std::to_string(static_cast<uint16_t>(layer))] = name;
	}

	Write(p_path, doc);
}

void LayerUtility::InternalDeserializeLayerNames(const std::string &p_path)
{
	std::ifstream fileRead(p_path);
	if (!fileRead.is_open())
	{
		return;
	}

	json doc = Read(p_path);

	if (doc.is_discarded())
	{
		return;
	}

	for (const auto &[key, value] : doc.items())
	{
		uint16_t layerValue = static_cast<uint16_t>(std::stoi(key));
		Layer layer = static_cast<Layer>(layerValue);
		std::string name = value.get<std::string>();
		m_layerNames[layer] = name;
	}
}

bool LayerUtility::EditLayerNamesPopup(bool p_doOpen)
{
	using namespace ImGui;

	if (p_doOpen)
	{
		OpenPopup("Edit Layers");
	}

	bool changed = false;

	if (BeginPopup("Edit Layers", ImGuiWindowFlags_AlwaysAutoResize))
	{
		for (auto &[layer, name] : m_layerNames)
		{
			std::string inputLabel = "##" + std::to_string(static_cast<uint16_t>(layer));

			if (InputText(inputLabel.c_str(), &name))
			{
				changed = true;
			}
		}

		if (Button("Reset"))
		{
			for (auto &[layer, name] : m_layerNames)
			{
				name = "";
			}
			changed = true;
		}

		if (IsKeyPressed(ImGuiKey_Escape) || IsKeyPressed(ImGuiKey_Enter))
		{
			CloseCurrentPopup();
		}

		EndPopup();
	}

	return changed;
}

bool LayerUtility::SetLayerMaskUI(const char *label, Layer &p_layerMask)
{
	using namespace ImGui;

	PushID(label);
	BeginGroup();

	Text("%s", label);

	std::string comboDisplay = "";
	bool isAllSelected = (p_layerMask == Layer::All);
	bool isNoneSelected = (p_layerMask == Layer::None);

	if (isAllSelected)
	{
		comboDisplay = "All Layers";
	}
	else if (isNoneSelected)
	{
		comboDisplay = "No Layers";
	}
	else if (IsSingleLayer(p_layerMask))
	{
		comboDisplay = GetLayerName(p_layerMask);
	}
	else // Multiple layers selected
	{
		comboDisplay = "Multiple Layers";
	}

	SameLine();

	bool changed = false;

	// Set the max width of the combo box to avail region minus an icon button width

	float availWidth = GetContentRegionAvail().x;
	float iconButtonWidth = GetFrameHeight(); // Assuming square button

	SetNextItemWidth(availWidth - iconButtonWidth - GetStyle().ItemSpacing.x);

	// Dropdown for selecting layers
	if (BeginCombo("##LayerMaskCombo", comboDisplay.c_str(), ImGuiComboFlags_HeightLarge))
	{
		// None option
		{
			if (Selectable("None", isNoneSelected, ImGuiSelectableFlags_NoAutoClosePopups))
			{
				p_layerMask = Layer::None;
				changed = true;
			}
		}

		// All option
		{
			if (Selectable("All", isAllSelected, ImGuiSelectableFlags_NoAutoClosePopups))
			{
				p_layerMask = Layer::All;
				changed = true;
			}
		}

		Separator();

		for (const auto &[layer, name] : m_layerNames)
		{
			if (name.empty())
			{
				continue; // Skip empty names
			}

			bool isSelected = IsLayerEnabled(layer, p_layerMask);
			if (Selectable(name.c_str(), isSelected, ImGuiSelectableFlags_NoAutoClosePopups))
			{
				ToggleLayer(layer, p_layerMask);
				changed = true;
			}
		}
		EndCombo();
	}

	SameLine();
	EditLayerNamesPopup(Button(ICON_LC_SETTINGS));

	EndGroup();
	PopID();

	return changed;
}

bool LayerUtility::SetLayerUI(const char *label, Layer &p_layer)
{
	using namespace ImGui;

	// Ensure that p_layer is a single layer flag
	assert(
		IsSingleLayer(p_layer) &&
		"p_layer must be a single layer flag, not a layer mask."
	);

	PushID(label);
	BeginGroup();

	Text("%s", label);

	SameLine();

	bool changed = false;

	// Set the max width of the combo box to avail region minus an icon button width

	float availWidth = GetContentRegionAvail().x;
	float iconButtonWidth = GetFrameHeight(); // Assuming square button

	SetNextItemWidth(availWidth - iconButtonWidth - GetStyle().ItemSpacing.x);

	// Dropdown for selecting layers
	if (BeginCombo("##LayerCombo", GetLayerName(p_layer).c_str(), ImGuiComboFlags_HeightLarge))
	{
		for (const auto &[layer, name] : m_layerNames)
		{
			if (name.empty())
			{
				continue; // Skip empty names
			}

			if (Selectable(name.c_str(), p_layer == layer))
			{
				p_layer = layer;
				changed = true;
			}
		}
		EndCombo();
	}

	SameLine();
	EditLayerNamesPopup(Button(ICON_LC_SETTINGS));

	EndGroup();
	PopID();

	return changed;
}
