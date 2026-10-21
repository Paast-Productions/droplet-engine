#include "ImGui/UIStyle.hpp"
#include <ImGui/imgui.h>
#include <Core/IoManager.hpp>
#include <EditorConfig.hpp>
#include <filesystem>
#include <fstream>

using namespace Droplet::Editor;
using namespace Droplet::Core::JsonIO;
using namespace nlohmann;

constexpr const char *LAYOUTS_PATH = ROOT_PATH "/Editor/Layouts";
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


void UIStyle::GetLayoutNames(std::vector<std::string> &layouts)
{
	layouts.clear();

	// Search for all .ini files in layout directory
	for (const auto &entry : std::filesystem::directory_iterator(LAYOUTS_PATH))
	{
		const auto &path = entry.path();
		std::string filename = path.filename().string();
		std::string ext = filename.c_str() + filename.find_last_of('.') + 1;

		// Skip non-layout files
		if (ext != "ini")
		{
			continue; 
		}

		// Skip files without a filename
		if (filename.find_last_of('.') == std::string::npos)
		{
			continue; // Skip files without a filename
		}

		filename = filename.substr(0, filename.find_last_of('.'));
		layouts.emplace_back(filename);
	}
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

		// Skip non-json files
		if (ext != "json")
		{
			continue;
		}

		// Skip files without a filename
		if (filename.find_last_of('.') == std::string::npos)
		{
			continue;
		}

		filename = filename.substr(0, filename.find_last_of('.'));
		styles.emplace_back(filename);
	}
}


void UIStyle::SaveLayout(const std::string &name)
{
	// Save current ImGui ini settings to file
	std::string path = std::string(LAYOUTS_PATH) + "/" + name + ".ini";

	ImGui::SaveIniSettingsToDisk(path.c_str());

	// Set the current layout name in the editor config
	EditorConfig::StoreString("Editor.Layout", name);
}

void UIStyle::SaveStyle(const std::string &name)
{
	// Create JSON document
	json doc = json::object();

	{
		ImGuiStyle &style = ImGui::GetStyle();

		ImFont *font = ImGui::GetFont();
		std::string fontName = font->GetDebugName();

		float fontGlobalScale = ImGui::GetIO().FontGlobalScale;

		// Save current style to JSON

		doc = {
			{ "FontName",									fontName },
			{ "FontGlobalScale",							fontGlobalScale },
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


void UIStyle::LoadLayout(const std::string &name)
{
	// Load ImGui ini settings from file
	std::string path = std::string(LAYOUTS_PATH) + "/" + name + ".ini";

	// Check if the layout file exists
	if (!std::filesystem::exists(path))
	{
		throw std::runtime_error("Layout file does not exist: " + path);
	}

	ImGui::LoadIniSettingsFromDisk(path.c_str());

	// Set the current layout name in the editor config
	EditorConfig::StoreString("Editor.Layout", name);
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

		if (doc.contains("WindowMenuButtonPosition"))
		doc.at("WindowMenuButtonPosition").get_to(style.WindowMenuButtonPosition);

		if (doc.contains("ColorButtonPosition"))
		doc.at("ColorButtonPosition").get_to(style.ColorButtonPosition);

		if (doc.contains("HoverFlagsForTooltipMouse"))
		doc.at("HoverFlagsForTooltipMouse").get_to(style.HoverFlagsForTooltipMouse);

		if (doc.contains("HoverFlagsForTooltipNav"))
		doc.at("HoverFlagsForTooltipNav").get_to(style.HoverFlagsForTooltipNav);

		if (doc.contains("TreeLinesFlags"))
		doc.at("TreeLinesFlags").get_to(style.TreeLinesFlags);

		if (doc.contains("DockingNodeHasCloseButton"))
		doc.at("DockingNodeHasCloseButton").get_to(style.DockingNodeHasCloseButton);

		if (doc.contains("AntiAliasedLines"))
		doc.at("AntiAliasedLines").get_to(style.AntiAliasedLines);

		if (doc.contains("AntiAliasedLinesUseTex"))
		doc.at("AntiAliasedLinesUseTex").get_to(style.AntiAliasedLinesUseTex);

		if (doc.contains("AntiAliasedFill"))
		doc.at("AntiAliasedFill").get_to(style.AntiAliasedFill);

		if (doc.contains("FontSizeBase"))
		doc.at("FontSizeBase").get_to(style.FontSizeBase);

		if (doc.contains("FontScaleMain"))
		doc.at("FontScaleMain").get_to(style.FontScaleMain);

		if (doc.contains("FontScaleDpi"))
		doc.at("FontScaleDpi").get_to(style.FontScaleDpi);

		if (doc.contains("Alpha"))
		doc.at("Alpha").get_to(style.Alpha);

		if (doc.contains("DisabledAlpha"))
		doc.at("DisabledAlpha").get_to(style.DisabledAlpha);

		if (doc.contains("WindowRounding"))
		doc.at("WindowRounding").get_to(style.WindowRounding);

		if (doc.contains("WindowBorderSize"))
		doc.at("WindowBorderSize").get_to(style.WindowBorderSize);

		if (doc.contains("WindowBorderHoverPadding"))
		doc.at("WindowBorderHoverPadding").get_to(style.WindowBorderHoverPadding);

		if (doc.contains("ChildRounding"))
		doc.at("ChildRounding").get_to(style.ChildRounding);

		if (doc.contains("ChildBorderSize"))
		doc.at("ChildBorderSize").get_to(style.ChildBorderSize);

		if (doc.contains("PopupRounding"))
		doc.at("PopupRounding").get_to(style.PopupRounding);

		if (doc.contains("PopupBorderSize"))
		doc.at("PopupBorderSize").get_to(style.PopupBorderSize);

		if (doc.contains("FrameRounding"))
		doc.at("FrameRounding").get_to(style.FrameRounding);

		if (doc.contains("FrameBorderSize"))
		doc.at("FrameBorderSize").get_to(style.FrameBorderSize);

		if (doc.contains("IndentSpacing"))
		doc.at("IndentSpacing").get_to(style.IndentSpacing);

		if (doc.contains("ColumnsMinSpacing"))
		doc.at("ColumnsMinSpacing").get_to(style.ColumnsMinSpacing);

		if (doc.contains("ScrollbarSize"))
		doc.at("ScrollbarSize").get_to(style.ScrollbarSize);

		if (doc.contains("ScrollbarRounding"))
		doc.at("ScrollbarRounding").get_to(style.ScrollbarRounding);

		if (doc.contains("ScrollbarPadding"))
		doc.at("ScrollbarPadding").get_to(style.ScrollbarPadding);

		if (doc.contains("GrabMinSize"))
		doc.at("GrabMinSize").get_to(style.GrabMinSize);

		if (doc.contains("GrabRounding"))
		doc.at("GrabRounding").get_to(style.GrabRounding);

		if (doc.contains("LogSliderDeadzone"))
		doc.at("LogSliderDeadzone").get_to(style.LogSliderDeadzone);

		if (doc.contains("ImageRounding"))
		doc.at("ImageRounding").get_to(style.ImageRounding);

		if (doc.contains("ImageBorderSize"))
		doc.at("ImageBorderSize").get_to(style.ImageBorderSize);

		if (doc.contains("TabRounding"))
		doc.at("TabRounding").get_to(style.TabRounding);

		if (doc.contains("TabBorderSize"))
		doc.at("TabBorderSize").get_to(style.TabBorderSize);

		if (doc.contains("TabMinWidthBase"))
		doc.at("TabMinWidthBase").get_to(style.TabMinWidthBase);

		if (doc.contains("TabMinWidthShrink"))
		doc.at("TabMinWidthShrink").get_to(style.TabMinWidthShrink);

		if (doc.contains("TabCloseButtonMinWidthSelected"))
		doc.at("TabCloseButtonMinWidthSelected").get_to(style.TabCloseButtonMinWidthSelected);

		if (doc.contains("TabCloseButtonMinWidthUnselected"))
		doc.at("TabCloseButtonMinWidthUnselected").get_to(style.TabCloseButtonMinWidthUnselected);

		if (doc.contains("TabBarBorderSize"))
		doc.at("TabBarBorderSize").get_to(style.TabBarBorderSize);

		if (doc.contains("TabBarOverlineSize"))
		doc.at("TabBarOverlineSize").get_to(style.TabBarOverlineSize);

		if (doc.contains("TableAngledHeadersAngle"))
		doc.at("TableAngledHeadersAngle").get_to(style.TableAngledHeadersAngle);

		if (doc.contains("TreeLinesSize"))
		doc.at("TreeLinesSize").get_to(style.TreeLinesSize);

		if (doc.contains("TreeLinesRounding"))
		doc.at("TreeLinesRounding").get_to(style.TreeLinesRounding);

		if (doc.contains("MenuItemRounding"))
		doc.at("MenuItemRounding").get_to(style.MenuItemRounding);

		if (doc.contains("SelectableRounding"))
		doc.at("SelectableRounding").get_to(style.SelectableRounding);

		if (doc.contains("DragDropTargetRounding"))
		doc.at("DragDropTargetRounding").get_to(style.DragDropTargetRounding);

		if (doc.contains("DragDropTargetBorderSize"))
		doc.at("DragDropTargetBorderSize").get_to(style.DragDropTargetBorderSize);

		if (doc.contains("DragDropTargetPadding"))
		doc.at("DragDropTargetPadding").get_to(style.DragDropTargetPadding);

		if (doc.contains("ColorMarkerSize"))
		doc.at("ColorMarkerSize").get_to(style.ColorMarkerSize);

		if (doc.contains("InputTextCursorSize"))
		doc.at("InputTextCursorSize").get_to(style.InputTextCursorSize);

		if (doc.contains("SeparatorSize"))
		doc.at("SeparatorSize").get_to(style.SeparatorSize);

		if (doc.contains("SeparatorTextBorderSize"))
		doc.at("SeparatorTextBorderSize").get_to(style.SeparatorTextBorderSize);

		if (doc.contains("DockingSeparatorSize"))
		doc.at("DockingSeparatorSize").get_to(style.DockingSeparatorSize);

		if (doc.contains("MouseCursorScale"))
		doc.at("MouseCursorScale").get_to(style.MouseCursorScale);

		if (doc.contains("CurveTessellationTol"))
		doc.at("CurveTessellationTol").get_to(style.CurveTessellationTol);

		if (doc.contains("CircleTessellationMaxError"))
		doc.at("CircleTessellationMaxError").get_to(style.CircleTessellationMaxError);

		if (doc.contains("HoverStationaryDelay"))
		doc.at("HoverStationaryDelay").get_to(style.HoverStationaryDelay);

		if (doc.contains("HoverDelayShort"))
		doc.at("HoverDelayShort").get_to(style.HoverDelayShort);

		if (doc.contains("HoverDelayNormal"))
		doc.at("HoverDelayNormal").get_to(style.HoverDelayNormal);

		if (doc.contains("WindowPadding"))
		style.WindowPadding = ToVec2(doc.at("WindowPadding"));

		if (doc.contains("WindowMinSize"))
		style.WindowMinSize = ToVec2(doc.at("WindowMinSize"));

		if (doc.contains("WindowTitleAlign"))
		style.WindowTitleAlign = ToVec2(doc.at("WindowTitleAlign"));

		if (doc.contains("FramePadding"))
		style.FramePadding = ToVec2(doc.at("FramePadding"));

		if (doc.contains("ItemSpacing"))
		style.ItemSpacing = ToVec2(doc.at("ItemSpacing"));

		if (doc.contains("ItemInnerSpacing"))
		style.ItemInnerSpacing = ToVec2(doc.at("ItemInnerSpacing"));

		if (doc.contains("CellPadding"))
		style.CellPadding = ToVec2(doc.at("CellPadding"));

		if (doc.contains("TouchExtraPadding"))
		style.TouchExtraPadding = ToVec2(doc.at("TouchExtraPadding"));

		if (doc.contains("TableAngledHeadersTextAlign"))
		style.TableAngledHeadersTextAlign = ToVec2(doc.at("TableAngledHeadersTextAlign"));

		if (doc.contains("ButtonTextAlign"))
		style.ButtonTextAlign = ToVec2(doc.at("ButtonTextAlign"));

		if (doc.contains("SelectableTextAlign"))
		style.SelectableTextAlign = ToVec2(doc.at("SelectableTextAlign"));

		if (doc.contains("SeparatorTextAlign"))
		style.SeparatorTextAlign = ToVec2(doc.at("SeparatorTextAlign"));

		if (doc.contains("SeparatorTextPadding"))
		style.SeparatorTextPadding = ToVec2(doc.at("SeparatorTextPadding"));

		if (doc.contains("DisplayWindowPadding"))
		style.DisplayWindowPadding = ToVec2(doc.at("DisplayWindowPadding"));

		if (doc.contains("DisplaySafeAreaPadding"))
			style.DisplaySafeAreaPadding = ToVec2(doc.at("DisplaySafeAreaPadding"));

		for (int i = 0; i < ImGuiCol_COUNT; ++i)
		{
			style.Colors[i] = ToVec4(doc.at("Colors").at(i));
		}

		// TODO: Load font
		if (doc.contains("FontName"))
			std::string fontName = doc.at("FontName").get<std::string>();

		// Set font global scale
		if (doc.contains("FontGlobalScale"))
			ImGui::GetIO().FontGlobalScale = doc.at("FontGlobalScale").get<float>();
	}

	EditorConfig::StoreString("Editor.Style", name);
}
