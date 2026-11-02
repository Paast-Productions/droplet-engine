#include "DrawLayer.hpp"
#include "json/json.hpp"
#include "Core/IoManager.hpp"
#include "ImGui/imgui.h"
#include "ImGui/imgui_stdlib.h"
#include <IconFontCppHeaders/IconsLucide.h>

using namespace Droplet::Graphics;
using namespace Droplet::Core::JsonIO;
using namespace nlohmann;

void DrawLayerUtility::InternalSerializeLayerNames(const std::string &p_path) const
{
	json doc = json::object();

	for (const auto &[layer, name] : m_layerNames)
	{
		doc[std::to_string(static_cast<uint16_t>(layer))] = name;
	}

	Write(p_path, doc);
}

void DrawLayerUtility::InternalDeserializeLayerNames(const std::string &p_path)
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
		DrawLayer layer = static_cast<DrawLayer>(layerValue);
		std::string name = value.get<std::string>();
		m_layerNames[layer] = name;
	}
}

bool DrawLayerUtility::EditLayerNamesPopup(bool p_doOpen)
{
	using namespace ImGui;
	auto &instance = Get();

	if (p_doOpen)
	{
		OpenPopup("Edit Draw Layers");
	}

	bool changed = false;

	if (BeginPopup("Edit Draw Layers", ImGuiWindowFlags_AlwaysAutoResize))
	{
		for (auto &[layer, name] : instance.m_layerNames)
		{
			std::string inputLabel = "##" + std::to_string(static_cast<uint16_t>(layer));

			if (InputText(inputLabel.c_str(), &name))
			{
				changed = true;
			}
		}

		if (Button("Reset"))
		{
			for (auto &[layer, name] : instance.m_layerNames)
			{
				int layerIndex = std::log2(static_cast<uint16_t>(layer)) + 1; // Calculate the layer index based on the bit position
				name = "Layer " + std::to_string(layerIndex);
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

bool DrawLayerUtility::SetLayerMaskUI(const char *label, DrawLayer &p_layerMask)
{
	using namespace ImGui;
	auto &instance = Get();

	PushID(label);
	BeginGroup();

	Text("%s", label);

	std::string comboDisplay = "";
	bool isAllSelected = (p_layerMask == DrawLayer::All);
	bool isNoneSelected = (p_layerMask == DrawLayer::None);

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
				p_layerMask = DrawLayer::None;
				changed = true;
			}
		}

		// All option
		{
			if (Selectable("All", isAllSelected, ImGuiSelectableFlags_NoAutoClosePopups))
			{
				p_layerMask = DrawLayer::All;
				changed = true;
			}
		}

		Separator();

		for (const auto &[layer, name] : instance.m_layerNames)
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

bool DrawLayerUtility::SetLayerUI(const char *label, DrawLayer &p_layer)
{
	using namespace ImGui;
	auto &instance = Get();

	// Ensure that p_layer is a single layer flag
	assert(
		IsSingleLayer(p_layer),
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
		for (const auto &[layer, name] : instance.m_layerNames)
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
