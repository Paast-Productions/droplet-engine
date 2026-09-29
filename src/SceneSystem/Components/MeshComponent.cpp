#include "MeshComponent.hpp"
#include <ImGui/imgui.h>
#include <Editor/DragDropFields.hpp>

using namespace Droplet::Scene;

MeshComponent::MeshComponent(const std::string &p_meshPath)
    : m_meshPath((p_meshPath))
{
}

void MeshComponent::Update([[maybe_unused]] float p_deltaTime)
{
	//TODO: will be inplemted later when the mesh system is implemented
}

void MeshComponent::RenderInternalUI()
{
	using namespace Droplet::Editor;
	using namespace ImGui;

	Text("Mesh Path: ");
	SameLine();

	BeginGroup();
	Text("%s", m_meshPath.c_str());
	EndGroup();

	if (BeginDragDropTarget())
	{
		// Verify that the payload is a mesh resource
		bool isValidResource = false;

		{
			const ImGuiPayload *payload = GetDragDropPayload();

			bool isResource = payload->IsDataType(PAYLOAD_RESOURCE.data());

			if (isResource)
			{
				// TODO: Check with ResourceManager if the resource is a mesh resource

				isValidResource = true;
			}
		}

		if (isValidResource)
		{
			if (const ImGuiPayload *payload = AcceptDragDropPayload(PAYLOAD_RESOURCE.data()))
			{
				// TODO: Replace with resource handle
				const char *payloadPath = static_cast<const char *>(payload->Data);
				m_meshPath = std::string(payloadPath);
			}
		}

		EndDragDropTarget();
	}

}

const std::string &MeshComponent::GetMeshPath() const
{
    return m_meshPath;
}
