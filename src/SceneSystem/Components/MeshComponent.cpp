#include "MeshComponent.hpp"
#include <ImGui/imgui.h>
#include <Editor/DragDropFields.hpp>

using namespace Droplet::Scene;

MeshComponent::MeshComponent() : m_bounds(this)
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

	Text("Mesh GUID: ");
	SameLine();

	BeginGroup();
	Text("%llu", m_meshGUID);
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
				m_meshGUID = *static_cast<Droplet::GUID *>(payload->Data);
			}
		}

		EndDragDropTarget();
	}


	Text("Material GUID: ");
	SameLine();

	BeginGroup();
	Text("%llu", m_materialGUID);
	EndGroup();

	if (BeginDragDropTarget())
	{
		// Verify that the payload is a material resource
		bool isValidResource = false;

		{
			const ImGuiPayload *payload = GetDragDropPayload();

			bool isResource = payload->IsDataType(PAYLOAD_RESOURCE.data());

			if (isResource)
			{
				// TODO: Check with ResourceManager if the resource is a material resource

				isValidResource = true;
			}
		}

		if (isValidResource)
		{
			if (const ImGuiPayload *payload = AcceptDragDropPayload(PAYLOAD_RESOURCE.data()))
			{
				m_materialGUID = *static_cast<Droplet::GUID *>(payload->Data);
			}
		}

		EndDragDropTarget();
	}
}

void MeshComponent::SetMeshGUID(GUID p_meshGUID)
{
	// TODO: Update some state probably
	m_meshGUID = p_meshGUID;
}

void MeshComponent::SetMaterialGUID(GUID p_materialGUID)
{
	// TODO: Update some state probably
	m_materialGUID = p_materialGUID;
}

const std::weak_ptr<Droplet::MeshResource> MeshComponent::GetMeshResource() const
{
    // TODO
    return std::weak_ptr<MeshResource>();
}

const std::weak_ptr<Droplet::MaterialResource> Droplet::Scene::MeshComponent::GetMaterialResource() const
{
	// TODO
	return std::weak_ptr<MaterialResource>();
}

nlohmann::json MeshComponent::SerializeImpl()
{
    nlohmann::json json;

    json["meshGUID"] = m_meshGUID;
    json["materialGUID"] = m_materialGUID;

    return json;
}

void MeshComponent::DeserializeImpl([[maybe_unused]] const nlohmann::json p_compJson)
{
	if (p_compJson.contains("meshGUID"))
	{
		m_meshGUID = p_compJson["meshGUID"].get<Droplet::GUID>();
	}

	if (p_compJson.contains("materialGUID"))
	{
		m_materialGUID = p_compJson["materialGUID"].get<Droplet::GUID>();
	}
}
