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

    // Mesh GUID
    std::uint64_t meshGUID = GetMeshGUID();

    if (InputScalar("Mesh GUID", ImGuiDataType_U64, &meshGUID, nullptr, nullptr, "%llu", ImGuiInputTextFlags_CharsHexadecimal))
    {
        SetMeshGUID(meshGUID);
    }

    if (BeginDragDropTarget())
    {
        if (const ImGuiPayload *payload =
            AcceptDragDropPayload(PAYLOAD_RESOURCE.data()))
        {
            if (payload->DataSize == sizeof(std::uint64_t))
            {
                SetMeshGUID(*static_cast<const std::uint64_t *>(payload->Data));
            }
        }

        EndDragDropTarget();
    }

    // Texture GUID
    std::uint64_t textureGUID = GetTextureGUID();

    if (InputScalar("Texture GUID", ImGuiDataType_U64, &textureGUID, nullptr, nullptr, "%llu", ImGuiInputTextFlags_CharsHexadecimal))
    {
        SetTextureGUID(textureGUID);
    }

    if (BeginDragDropTarget())
    {
        if (const ImGuiPayload *payload =
            AcceptDragDropPayload(PAYLOAD_RESOURCE.data()))
        {
            if (payload->DataSize == sizeof(std::uint64_t))
            {
                SetTextureGUID(*static_cast<const std::uint64_t *>(payload->Data));
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

void MeshComponent::SetMeshGUID(std::uint64_t p_meshGUID)
{
	m_meshGUID = p_meshGUID;
}

std::uint64_t MeshComponent::GetMeshGUID() const
{
	return m_meshGUID;
}

void MeshComponent::SetTextureGUID(std::uint64_t p_textureGUID)
{
	m_textureGUID = p_textureGUID;
}

std::uint64_t MeshComponent::GetTextureGUID() const
{
	return m_textureGUID;
}

nlohmann::json MeshComponent::SerializeImpl()
{
    nlohmann::json json;

    json["meshGUID"] = GetMeshGUID();
    json["textureGUID"] = GetTextureGUID();

    return json;
}

void MeshComponent::DeserializeImpl([[maybe_unused]] const nlohmann::json p_compJson)
{
	// TODO: Implement deserialization logic for the mesh component.
}
