#include <Scene/ResourceDropTarget.hpp>

#include <algorithm>
#include <cstddef>
#include <ImGui/imgui.h>
#include <glm/glm.hpp>
#include <Editor/DragDropFields.hpp>
#include <Graphics/Debug/DebugDraw.hpp>
#include <resource/meta/MetaData.hpp>
#include <SceneSystem/Node.hpp>

namespace Droplet::Editor
{
	ResourceDropEntries AcceptResourceDrop(bool &p_outIsDelivery)
	{
		ResourceDropEntries entries;
		p_outIsDelivery = false;

		if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(PAYLOAD_RESOURCE.data(), ImGuiDragDropFlags_AcceptBeforeDelivery))
		{
			if (payload->DataSize == sizeof(const MetaEntry *))
			{
				if (const MetaEntry *entry = *static_cast<const MetaEntry *const *>(payload->Data))
				{
					entries.push_back(entry);
				}
			}
			p_outIsDelivery |= payload->IsDelivery();
		}

		if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(PAYLOAD_ASSET.data(), ImGuiDragDropFlags_AcceptBeforeDelivery))
		{
			if (payload->DataSize % sizeof(const MetaEntry *) == 0)
			{
				const auto *items = static_cast<const MetaEntry *const *>(payload->Data);
				const std::size_t count = payload->DataSize / sizeof(const MetaEntry *);

				for (std::size_t i = 0; i < count; ++i)
				{
					if (items[i])
					{
						entries.push_back(items[i]);
					}
				}
			}
			p_outIsDelivery |= payload->IsDelivery();
		}

		return entries;
	}

	bool CanApplyResources(const Droplet::Scene::Node &p_node, const ResourceDropEntries &p_entries)
	{
		return std::any_of(p_entries.begin(), p_entries.end(),
			[&p_node](const MetaEntry *p_entry) { return p_node.CanApplyResource(*p_entry); });
	}

	void ApplyResources(Droplet::Scene::Node &p_node, const ResourceDropEntries &p_entries)
	{
		for (const MetaEntry *entry : p_entries)
		{
			p_node.OnResourceDropped(*entry);
		}
	}

	void DrawResourceDropHighlight(Droplet::Scene::Node &p_node, bool p_accepts)
	{
		const auto *bounds = p_node.GetBounds();
		if (!bounds)
		{
			return;
		}

		const glm::mat4 nodeMatrix = p_node.GetTransform().GetMatrix(Droplet::Scene::Transform::Space::World);
		const Droplet::Math::OBB obb = bounds->GetTransformedOBB(nodeMatrix);

		const glm::vec3 color = p_accepts ? glm::vec3(0.2f, 1.0f, 0.3f) : glm::vec3(1.0f, 0.25f, 0.25f);

		Droplet::Graphics::Debug::DebugDraw::DrawBox(obb, glm::vec4(color, 0.25f), false, true);
		Droplet::Graphics::Debug::DebugDraw::DrawBox(obb, glm::vec4(color, 1.0f), true, true);
	}
}
