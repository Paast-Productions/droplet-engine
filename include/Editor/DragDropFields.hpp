#pragma once
#include <ImGui/imgui.h>
#include <string>

namespace Droplet::Editor
{
	/// @brief Implements functionality to create ImGui drag and drop fields for various engine types, such as Components, Nodes and Assets.
	
	constexpr std::string_view PAYLOAD_NODE			= "DROPLET_NODE";
	constexpr std::string_view PAYLOAD_COMPONENT	= "DROPLET_COMPONENT";
	constexpr std::string_view PAYLOAD_RESOURCE		= "DROPLET_RESOURCE";
}