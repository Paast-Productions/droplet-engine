#pragma once
#ifdef _EDITOR
#include <ImGui/imgui.h>
#include <string>
#include <string_view>
#include <Resource/GuidUtils.hpp>
#include <Resource/IStaticResource.hpp>

namespace Droplet::Scene
{
	class Component;
	class Node;
}

namespace Droplet::Editor
{
	/// @brief Implements functionality to create ImGui drag and drop fields for various engine types, such as Components, Nodes and Assets.
	
	constexpr std::string_view PAYLOAD_NODE			= "DROPLET_NODE";
	constexpr std::string_view PAYLOAD_COMPONENT	= "DROPLET_COMPONENT";
	constexpr std::string_view PAYLOAD_RESOURCE		= "DROPLET_RESOURCE";
    constexpr std::string_view PAYLOAD_ASSET		= "DROPLET_ASSET";
    // Add more...

	struct ComponentDragDropData
	{
		Droplet::Scene::Node *sourceNode;
		Droplet::Scene::Component *component;
	};
    
    struct ResourcePayload
    {
        GUID guid;
        StaticResourceType type;
        char name[256];
    };
}
#endif
