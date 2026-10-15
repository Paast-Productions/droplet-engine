#include "ImGui/UIStyle.hpp"
#include <ImGui/imgui.h>
#include <Core/IoManager.hpp>
#include <EditorConfig.hpp>
#include <filesystem>
#include <fstream>

using namespace Droplet::Editor;
using namespace Droplet::Core::JsonIO;
using namespace nlohmann;

constexpr const char *STYLES_PATH = ROOT_PATH "/Editor/Styles";

static json ToArr(ImVec2 &vec)
{
	return json::array({ vec.x, vec.y });
}

static json ToArr(ImVec4 &vec)
{
	return json::array({ vec.x, vec.y, vec.z, vec.w });
}

static ImVec2 ToVec2(const json &arr)
{
	if (!arr.is_array() || arr.size() != 2)
		throw std::runtime_error("Invalid array size for ImVec2");
	return ImVec2(arr[0].get<float>(), arr[1].get<float>());
}

static ImVec4 ToVec4(const json &arr)
{
	if (!arr.is_array() || arr.size() != 4)
		throw std::runtime_error("Invalid array size for ImVec4");
	return ImVec4(arr[0].get<float>(), arr[1].get<float>(), arr[2].get<float>(), arr[3].get<float>());
}


void UIStyle::GetStyleNames(std::vector<std::string> &styles)
{
	styles.clear();

	// Search for all .json files in style directory
	for (const auto &entry : std::filesystem::directory_iterator(STYLES_PATH))
	{
		const auto &path = entry.path();
		std::string filename = path.filename().string();
		std::string ext = filename.c_str() + filename.find_last_of('.') + 1;

		if (ext != "json")
			continue; // Skip non-style files

		filename = filename.substr(0, filename.find_last_of('.'));
		styles.emplace_back(filename);
	}
}

void UIStyle::SaveStyle(const std::string &name)
{
	// Create JSON document
	json doc = json::object();

	{
		ImGuiStyle &style = ImGui::GetStyle();

		// Save current style to JSON

		doc = {
			{ "WindowMenuButtonPosition",					style.WindowMenuButtonPosition },
			{ "ColorButtonPosition",						style.ColorButtonPosition },
			{ "HoverFlagsForTooltipMouse",					style.HoverFlagsForTooltipMouse },
			{ "HoverFlagsForTooltipNav",					style.HoverFlagsForTooltipNav },
			{ "TreeLinesFlags",								style.TreeLinesFlags },

			{ "DockingNodeHasCloseButton",					style.DockingNodeHasCloseButton },
			{ "AntiAliasedLines",							style.AntiAliasedLines },
			{ "AntiAliasedLinesUseTex",						style.AntiAliasedLinesUseTex },
			{ "AntiAliasedFill",							style.AntiAliasedFill },

			{ "FontSizeBase",								style.FontSizeBase },
			{ "FontScaleMain",								style.FontScaleMain },
			{ "FontScaleDpi",								style.FontScaleDpi },
			{ "Alpha",										style.Alpha },
			{ "DisabledAlpha",								style.DisabledAlpha},
			{ "WindowRounding",								style.WindowRounding },
			{ "WindowBorderSize",							style.WindowBorderSize },
			{ "WindowBorderHoverPadding",					style.WindowBorderHoverPadding },
			{ "ChildRounding",								style.ChildRounding },
			{ "ChildBorderSize",							style.ChildBorderSize },
			{ "PopupRounding",								style.PopupRounding },
			{ "PopupBorderSize",							style.PopupBorderSize },
			{ "FrameRounding",								style.FrameRounding },
			{ "FrameBorderSize",							style.FrameBorderSize },
			{ "IndentSpacing",								style.IndentSpacing },
			{ "ColumnsMinSpacing",							style.ColumnsMinSpacing },
			{ "ScrollbarSize",								style.ScrollbarSize },
			{ "ScrollbarRounding",							style.ScrollbarRounding },
			{ "ScrollbarPadding",							style.ScrollbarPadding },
			{ "GrabMinSize",								style.GrabMinSize },
			{ "GrabRounding",								style.GrabRounding },
			{ "LogSliderDeadzone",							style.LogSliderDeadzone },
			{ "ImageRounding",								style.ImageRounding },
			{ "ImageBorderSize",							style.ImageBorderSize },
			{ "TabRounding",								style.TabRounding },
			{ "TabBorderSize",								style.TabBorderSize },
			{ "TabMinWidthBase",							style.TabMinWidthBase },
			{ "TabMinWidthShrink",							style.TabMinWidthShrink },
			{ "TabCloseButtonMinWidthSelected",				style.TabCloseButtonMinWidthSelected },
			{ "TabCloseButtonMinWidthUnselected",			style.TabCloseButtonMinWidthUnselected },
			{ "TabBarBorderSize",							style.TabBarBorderSize },
			{ "TabBarOverlineSize",							style.TabBarOverlineSize },
			{ "TableAngledHeadersAngle",					style.TableAngledHeadersAngle },
			{ "TreeLinesSize",								style.TreeLinesSize },
			{ "TreeLinesRounding",							style.TreeLinesRounding },
			{ "MenuItemRounding",							style.MenuItemRounding },
			{ "SelectableRounding",							style.SelectableRounding },
			{ "DragDropTargetRounding",						style.DragDropTargetRounding },
			{ "DragDropTargetBorderSize",					style.DragDropTargetBorderSize },
			{ "DragDropTargetPadding",						style.DragDropTargetPadding },
			{ "ColorMarkerSize",							style.ColorMarkerSize },
			{ "InputTextCursorSize",						style.InputTextCursorSize },
			{ "SeparatorSize",								style.SeparatorSize },
			{ "SeparatorTextBorderSize",					style.SeparatorTextBorderSize },
			{ "DockingSeparatorSize",						style.DockingSeparatorSize },
			{ "MouseCursorScale",							style.MouseCursorScale },
			{ "CurveTessellationTol",						style.CurveTessellationTol },
			{ "CircleTessellationMaxError",					style.CircleTessellationMaxError },
			{ "HoverStationaryDelay",						style.HoverStationaryDelay },
			{ "HoverDelayShort",							style.HoverDelayShort },
			{ "HoverDelayNormal",							style.HoverDelayNormal },

			{ "WindowPadding",								ToArr(style.WindowPadding) },
			{ "WindowMinSize",								ToArr(style.WindowMinSize) },
			{ "WindowTitleAlign",							ToArr(style.WindowTitleAlign) },
			{ "FramePadding",								ToArr(style.FramePadding) },
			{ "ItemSpacing",								ToArr(style.ItemSpacing) },
			{ "ItemInnerSpacing",							ToArr(style.ItemInnerSpacing) },
			{ "CellPadding",								ToArr(style.CellPadding) },
			{ "TouchExtraPadding",							ToArr(style.TouchExtraPadding) },
			{ "TableAngledHeadersTextAlign",				ToArr(style.TableAngledHeadersTextAlign) },
			{ "ButtonTextAlign",							ToArr(style.ButtonTextAlign) },
			{ "SelectableTextAlign",						ToArr(style.SelectableTextAlign) },
			{ "SeparatorTextAlign",							ToArr(style.SeparatorTextAlign) },
			{ "SeparatorTextPadding",						ToArr(style.SeparatorTextPadding) },
			{ "DisplayWindowPadding",						ToArr(style.DisplayWindowPadding) },
			{ "DisplaySafeAreaPadding",						ToArr(style.DisplaySafeAreaPadding) }
		};

		json colorsJson = json::array();
		for (int i = 0; i < ImGuiCol_COUNT; ++i)
		{
			ImVec4 color = style.Colors[i];
			colorsJson.push_back(ToArr(color));
		}

		doc["Colors"] = colorsJson;

	}

	// Write doc to file
	std::string filePath = std::string(STYLES_PATH) + "/" + name + ".json";

	try
	{
		Write(filePath, doc);
	}
	catch (const std::exception &e)
	{
		// TODO
	}

	EditorConfig::StoreString("Editor.Style", name);
}

void UIStyle::LoadStyle(const std::string &name)
{
	json doc;
	{
		std::string filePath = std::string(STYLES_PATH) + "/" + name + ".json";

		// Read JSON document from file
		try
		{
			doc = Read(filePath);
		}
		catch (const std::exception &e)
		{
			// TODO
		}
	}

	{
		ImGuiStyle &style = ImGui::GetStyle();
		
		doc.at("WindowMenuButtonPosition").get_to(style.WindowMenuButtonPosition);
		doc.at("ColorButtonPosition").get_to(style.ColorButtonPosition);
		doc.at("HoverFlagsForTooltipMouse").get_to(style.HoverFlagsForTooltipMouse);
		doc.at("HoverFlagsForTooltipNav").get_to(style.HoverFlagsForTooltipNav);
		doc.at("TreeLinesFlags").get_to(style.TreeLinesFlags);
		doc.at("DockingNodeHasCloseButton").get_to(style.DockingNodeHasCloseButton);
		doc.at("AntiAliasedLines").get_to(style.AntiAliasedLines);
		doc.at("AntiAliasedLinesUseTex").get_to(style.AntiAliasedLinesUseTex);
		doc.at("AntiAliasedFill").get_to(style.AntiAliasedFill);
		doc.at("FontSizeBase").get_to(style.FontSizeBase);
		doc.at("FontScaleMain").get_to(style.FontScaleMain);
		doc.at("FontScaleDpi").get_to(style.FontScaleDpi);
		doc.at("Alpha").get_to(style.Alpha);
		doc.at("DisabledAlpha").get_to(style.DisabledAlpha);
		doc.at("WindowRounding").get_to(style.WindowRounding);
		doc.at("WindowBorderSize").get_to(style.WindowBorderSize);
		doc.at("WindowBorderHoverPadding").get_to(style.WindowBorderHoverPadding);
		doc.at("ChildRounding").get_to(style.ChildRounding);
		doc.at("ChildBorderSize").get_to(style.ChildBorderSize);
		doc.at("PopupRounding").get_to(style.PopupRounding);
		doc.at("PopupBorderSize").get_to(style.PopupBorderSize);
		doc.at("FrameRounding").get_to(style.FrameRounding);
		doc.at("FrameBorderSize").get_to(style.FrameBorderSize);
		doc.at("IndentSpacing").get_to(style.IndentSpacing);
		doc.at("ColumnsMinSpacing").get_to(style.ColumnsMinSpacing);
		doc.at("ScrollbarSize").get_to(style.ScrollbarSize);
		doc.at("ScrollbarRounding").get_to(style.ScrollbarRounding);
		doc.at("ScrollbarPadding").get_to(style.ScrollbarPadding);
		doc.at("GrabMinSize").get_to(style.GrabMinSize);
		doc.at("GrabRounding").get_to(style.GrabRounding);
		doc.at("LogSliderDeadzone").get_to(style.LogSliderDeadzone);
		doc.at("ImageRounding").get_to(style.ImageRounding);
		doc.at("ImageBorderSize").get_to(style.ImageBorderSize);
		doc.at("TabRounding").get_to(style.TabRounding);
		doc.at("TabBorderSize").get_to(style.TabBorderSize);
		doc.at("TabMinWidthBase").get_to(style.TabMinWidthBase);
		doc.at("TabMinWidthShrink").get_to(style.TabMinWidthShrink);
		doc.at("TabCloseButtonMinWidthSelected").get_to(style.TabCloseButtonMinWidthSelected);
		doc.at("TabCloseButtonMinWidthUnselected").get_to(style.TabCloseButtonMinWidthUnselected);
		doc.at("TabBarBorderSize").get_to(style.TabBarBorderSize);
		doc.at("TabBarOverlineSize").get_to(style.TabBarOverlineSize);
		doc.at("TableAngledHeadersAngle").get_to(style.TableAngledHeadersAngle);
		doc.at("TreeLinesSize").get_to(style.TreeLinesSize);
		doc.at("TreeLinesRounding").get_to(style.TreeLinesRounding);
		doc.at("MenuItemRounding").get_to(style.MenuItemRounding);
		doc.at("SelectableRounding").get_to(style.SelectableRounding);
		doc.at("DragDropTargetRounding").get_to(style.DragDropTargetRounding);
		doc.at("DragDropTargetBorderSize").get_to(style.DragDropTargetBorderSize);
		doc.at("DragDropTargetPadding").get_to(style.DragDropTargetPadding);
		doc.at("ColorMarkerSize").get_to(style.ColorMarkerSize);
		doc.at("InputTextCursorSize").get_to(style.InputTextCursorSize);
		doc.at("SeparatorSize").get_to(style.SeparatorSize);
		doc.at("SeparatorTextBorderSize").get_to(style.SeparatorTextBorderSize);
		doc.at("DockingSeparatorSize").get_to(style.DockingSeparatorSize);
		doc.at("MouseCursorScale").get_to(style.MouseCursorScale);
		doc.at("CurveTessellationTol").get_to(style.CurveTessellationTol);
		doc.at("CircleTessellationMaxError").get_to(style.CircleTessellationMaxError);
		doc.at("HoverStationaryDelay").get_to(style.HoverStationaryDelay);
		doc.at("HoverDelayShort").get_to(style.HoverDelayShort);
		doc.at("HoverDelayNormal").get_to(style.HoverDelayNormal);

		style.WindowPadding = ToVec2(doc.at("WindowPadding"));
		style.WindowMinSize = ToVec2(doc.at("WindowMinSize"));
		style.WindowTitleAlign = ToVec2(doc.at("WindowTitleAlign"));
		style.FramePadding = ToVec2(doc.at("FramePadding"));
		style.ItemSpacing = ToVec2(doc.at("ItemSpacing"));
		style.ItemInnerSpacing = ToVec2(doc.at("ItemInnerSpacing"));
		style.CellPadding = ToVec2(doc.at("CellPadding"));
		style.TouchExtraPadding = ToVec2(doc.at("TouchExtraPadding"));
		style.TableAngledHeadersTextAlign = ToVec2(doc.at("TableAngledHeadersTextAlign"));
		style.ButtonTextAlign = ToVec2(doc.at("ButtonTextAlign"));
		style.SelectableTextAlign = ToVec2(doc.at("SelectableTextAlign"));
		style.SeparatorTextAlign = ToVec2(doc.at("SeparatorTextAlign"));
		style.SeparatorTextPadding = ToVec2(doc.at("SeparatorTextPadding"));
		style.DisplayWindowPadding = ToVec2(doc.at("DisplayWindowPadding"));
		style.DisplaySafeAreaPadding = ToVec2(doc.at("DisplaySafeAreaPadding"));

		for (int i = 0; i < ImGuiCol_COUNT; ++i)
		{
			style.Colors[i] = ToVec4(doc.at("Colors").at(i));
		}
	}

	EditorConfig::StoreString("Editor.Style", name);
}
