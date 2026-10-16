#pragma once
#include <ImGui/imgui.h>
#include <string>
#include "resource/GuidUtils.hpp"

namespace Droplet::Editor
{
	/// @brief Implements functionality to create ImGui drag and drop fields for various engine types, such as Components, Nodes and Assets.
	
	constexpr std::string_view PAYLOAD_NODE			= "DROPLET_NODE\0";
	constexpr std::string_view PAYLOAD_COMPONENT	= "DROPLET_COMPONENT\0";
	constexpr std::string_view PAYLOAD_RESOURCE		= "DROPLET_RESOURCE\0";
    constexpr std::string_view PAYLOAD_ASSET		= "DROPLET_ASSET\0";
    
    struct ResourcePayload
    {
        GUID guid;
        ResourceType type;
        char name[256];
    };
}