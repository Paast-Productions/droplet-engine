#include "MeshComponent.hpp"
#include <ImGui/imgui.h>
#include <Editor/DragDropFields.hpp>

using namespace Droplet::Scene;

// TODO: Temp fix for bounds 
Droplet::Scene::MeshComponent::MeshComponent() : m_bounds(this)
{
}

void MeshComponent::Update([[maybe_unused]] float p_deltaTime)
{
	//TODO: will be inplemted later when the mesh system is implemented
}

void MeshComponent::RenderUIImpl()
{
	using namespace Droplet::Editor;
	using namespace ImGui;

	Text("Mesh Path: ");
	SameLine();

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
				if (payload->DataSize == sizeof(int))
				{
					const int resourceID =
						*static_cast<const int *>(payload->Data);

					SetMeshID(resourceID);
				}
			}
		}

		EndDragDropTarget();
	}

}

const std::weak_ptr<Droplet::MeshResource> MeshComponent::GetMeshResource() const
{
    // TODO
    return std::weak_ptr<MeshResource>();
}

void MeshComponent::SetMeshID(int p_meshID)
{
	m_meshID = p_meshID;
}

int MeshComponent::GetMeshID() const
{
	return m_meshID;
}

void MeshComponent::SetTextureID(int p_textureID)
{
	m_textureID = p_textureID;
}

int MeshComponent::GetTextureID() const
{
	return m_textureID;
}

nlohmann::json MeshComponent::SerializeImpl()
{
    nlohmann::json json;

    json["meshID"] = GetMeshID();
    json["textureID"] = GetTextureID();

    return json;
}

void MeshComponent::DeserializeImpl([[maybe_unused]] const nlohmann::json p_compJson)
{
	// TODO: Implement deserialization logic for the mesh component.
}
