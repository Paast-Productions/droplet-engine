#include "MeshComponent.hpp"
#include <ImGui/imgui.h>
#include <Editor/DragDropFields.hpp>
#include <resource/ResourceManager.hpp>
#include <Engine/Engine.hpp>
#include <string>
#include "resource/meta/MetaUtils.hpp">

// imgui helper function

namespace
{
    bool ResourceSlot(
        const char *p_label,
        Droplet::GUID p_current,
        const std::string &p_currentName,
        Droplet::GUID &p_outGUID,
        std::string &p_outName,
        Droplet::ResourceType p_expectedType)
    {
        using namespace ImGui;
        using namespace Droplet::Editor;

        bool changed = false;

        PushID(p_label);
        TableNextRow();
        TableNextColumn();
        AlignTextToFramePadding();
        TextUnformatted(p_label);
        TableNextColumn();

        const float clearW = GetFrameHeight();
        const float gap = GetStyle().ItemSpacing.x;

        const std::string text = p_current != Droplet::C_INVALID_GUID
            ? p_currentName
            : "None (drop resource here)";

        if (Button(text.c_str(), ImVec2(-(clearW + gap), 0.0f)))
        {
            OpenPopup("pick");
        }

        if (BeginDragDropTarget())
        {
            if (const ImGuiPayload *pl =
                AcceptDragDropPayload(PAYLOAD_RESOURCE.data()))
            {
                if (pl->DataSize == sizeof(const Droplet::MetaEntry *))
                {
                    const auto *entryPtr =
                        *static_cast<const Droplet::MetaEntry *const *>(pl->Data);

                    if (entryPtr->type == p_expectedType)
                    {
                        // Access the complete entry.
                        p_outGUID = entryPtr->guid;
                        p_outName = entryPtr->displayName;
                        changed = true;
                    }
                    else
                    {

                    }
                }
            }

            EndDragDropTarget();
        }

        if (p_current != Droplet::C_INVALID_GUID && IsItemHovered())
        {
            SetTooltip(
                "GUID: %llu",
                static_cast<unsigned long long>(p_current));
        }

        SameLine();
        BeginDisabled(p_current == Droplet::C_INVALID_GUID);

        if (Button("X", ImVec2(clearW, clearW)))
        {
            p_outGUID = Droplet::C_INVALID_GUID;
            p_outName.clear();
            changed = true;
        }

        EndDisabled();

        PopID();
        return changed;
    }
}

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
    using namespace ImGui;

    auto &resourceManager = Droplet::ResourceManager::Get();

    PushID(this);

    if (BeginTable("MeshProps", 2, ImGuiTableFlags_SizingStretchProp))
    {
        TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

        Droplet::GUID newGUID = C_INVALID_GUID;
        std::string newName;

        // Retrieve the existing resource names from ResourceManager here.
        // Replace these examples with your actual lookup API.

        if (ResourceSlot(
            "Mesh",
            m_mesh.GetGUID(),
            m_meshName,
            newGUID,
            newName,
            Droplet::ResourceType::Mesh))
        {
            SetMeshGUID(newGUID, resourceManager);
			m_meshName = newName;
        }

        newGUID = C_INVALID_GUID;
        newName.clear();

        if (ResourceSlot(
            "Texture",
            m_2dTexture.GetGUID(),
            m_textureName,
            newGUID,
            newName,
            Droplet::ResourceType::Texture2D))
        {
            SetTextureGUID(newGUID, resourceManager);
			m_textureName = newName;
        }

        EndTable();
    }

    PopID();
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

    // GUIDS

    json["meshGUID"] = GetMeshGUID();
    json["textureGUID"] = GetTextureGUID();

	// Names

	json["meshName"] = m_meshName;
	json["textureName"] = m_textureName;

    return json;
}

void MeshComponent::DeserializeImpl([[maybe_unused]] const nlohmann::json p_compJson)
{

    // GUIDS

	if (p_compJson.contains("meshGUID"))
	{
        m_mesh = ResourceManager::Get().LoadResource<Droplet::MeshResource>(p_compJson["meshGUID"].get<Droplet::GUID>());
	}

	if (p_compJson.contains("textureGUID"))
	{
        m_2dTexture = ResourceManager::Get().LoadResource<Droplet::Texture2DResource>(p_compJson["textureGUID"].get<Droplet::GUID>());
	}

	// Names

	if (p_compJson.contains("meshName"))
	{
		m_meshName = p_compJson["meshName"].get<std::string>();
	}

	if (p_compJson.contains("textureName"))
	{
		m_textureName = p_compJson["textureName"].get<std::string>();
	}
}
