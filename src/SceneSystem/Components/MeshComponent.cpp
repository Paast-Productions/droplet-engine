#include "MeshComponent.hpp"
#include <ImGui/imgui.h>
#include <Editor/DragDropFields.hpp>
#include <resource/ResourceManager.hpp>
#include <Engine/Engine.hpp>
#include <string>

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

    auto &resourceManager = Droplet::ResourceManager::Get();

    // Mesh drop target
    const Droplet::GUID meshGUID = m_mesh.GetGUID();
    const std::string meshLabel =
        meshGUID != 0
        ? "Mesh: " + std::to_string(meshGUID)
        : "Drop Mesh Here";

    Button(meshLabel.c_str(), ImVec2(-1.0f, 40.0f));

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

    // Texture drop target
    const Droplet::GUID textureGUID = m_2dTexture.GetGUID();
    const std::string textureLabel =
        textureGUID != 0
        ? "Texture: " + std::to_string(textureGUID)
        : "Drop Texture Here";

    Button(textureLabel.c_str(), ImVec2(-1.0f, 40.0f));

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
	if (p_compJson.contains("meshGUID"))
	{
		m_meshGUID = p_compJson["meshGUID"].get<Droplet::GUID>();
	}

	if (p_compJson.contains("textureGUID"))
	{
		m_textureGUID = p_compJson["textureGUID"].get<Droplet::GUID>();
	}
}
