#include "MeshComponent.hpp"
#include <ImGui/imgui.h>
#include <Editor/DragDropFields.hpp>
#include <resource/ResourceManager.hpp>
#include <Engine/EngineProvider.hpp>
#include <Engine/Engine.hpp>

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

	Droplet::Engine &engine = Droplet::EngineProvider::Get();

    // Mesh GUID


    if (InputScalar("Mesh GUID", ImGuiDataType_U64, &m_meshGUID, nullptr, nullptr, "%llu", ImGuiInputTextFlags_CharsHexadecimal))
    {
        SetMeshGUID(m_meshGUID, engine.GetResourceManager());
    }

    if (BeginDragDropTarget())
    {
        if (const ImGuiPayload *payload =
            AcceptDragDropPayload(PAYLOAD_RESOURCE.data()))
        {
            if (payload->DataSize == sizeof(std::uint64_t))
            {
                SetMeshGUID(*static_cast<const std::uint64_t *>(payload->Data), engine.GetResourceManager());
            }
        }

        EndDragDropTarget();
    }

    // Texture GUID

    if (InputScalar("Texture GUID", ImGuiDataType_U64, &m_textureGUID, nullptr, nullptr, "%llu", ImGuiInputTextFlags_CharsHexadecimal))
    {
        SetTextureGUID(m_textureGUID, engine.GetResourceManager());
    }

    if (BeginDragDropTarget())
    {
        if (const ImGuiPayload *payload =
            AcceptDragDropPayload(PAYLOAD_RESOURCE.data()))
        {
            if (payload->DataSize == sizeof(std::uint64_t))
            {
                SetTextureGUID(*static_cast<const std::uint64_t *>(payload->Data), engine.GetResourceManager());
            }
        }

        EndDragDropTarget();
    }
}

void MeshComponent::SetMeshGUID(
    Droplet::GUID p_guid,
    Droplet::ResourceManager &p_resourceManager)
{
    m_mesh = p_resourceManager.LoadResource<Droplet::MeshResource>(p_guid);
}

void MeshComponent::SetTextureGUID(
    Droplet::GUID p_guid,
    Droplet::ResourceManager &p_resourceManager)
{
    m_texture = p_resourceManager.LoadResource<Droplet::TextureResource>(p_guid);
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
