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

    auto &engine = Droplet::EngineProvider::Get();
    auto &resourceManager = engine.GetResourceManager();

    // Mesh resource
    Droplet::GUID meshGUID = m_mesh.GetGUID();

    if (InputScalar(
        "Mesh GUID",
        ImGuiDataType_U64,
        &meshGUID,
        nullptr,
        nullptr,
        "%llu"))
    {
        SetMeshGUID(meshGUID, resourceManager);
    }

    if (BeginDragDropTarget())
    {
        if (const ImGuiPayload *payload =
            AcceptDragDropPayload(PAYLOAD_RESOURCE.data()))
        {
            if (payload->DataSize == sizeof(Droplet::GUID))
            {
                const auto guid =
                    *static_cast<const Droplet::GUID *>(payload->Data);

                SetMeshGUID(guid, resourceManager);
            }
        }

        EndDragDropTarget();
    }

    // Texture resource
    Droplet::GUID textureGUID = m_2dTexture.GetGUID();

    if (InputScalar(
        "Texture GUID",
        ImGuiDataType_U64,
        &textureGUID,
        nullptr,
        nullptr,
        "%llu"))
    {
        SetTextureGUID(textureGUID, resourceManager);
    }

    if (BeginDragDropTarget())
    {
        if (const ImGuiPayload *payload =
            AcceptDragDropPayload(PAYLOAD_RESOURCE.data()))
        {
            if (payload->DataSize == sizeof(Droplet::GUID))
            {
                const auto guid =
                    *static_cast<const Droplet::GUID *>(payload->Data);

                SetTextureGUID(guid, resourceManager);
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
    m_2dTexture = p_resourceManager.LoadResource<Droplet::Texture2DResource>(p_guid);
}

const Droplet::ResourceHandle<Droplet::MeshResource> &Droplet::Scene::MeshComponent::GetMeshResource() const
{
    // TODO: insert return statement here
    return m_mesh;
}

const Droplet::ResourceHandle<Droplet::Texture2DResource> &Droplet::Scene::MeshComponent::Get2dTextureResource() const
{
    // TODO: insert return statement here
    return m_2dTexture;
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
