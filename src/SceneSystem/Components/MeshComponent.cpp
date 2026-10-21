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
        const std::string &p_currentDisplayName,
        Droplet::GUID &p_outGUID,
        std::string &p_outDisplayName,
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
            ? p_currentDisplayName
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
                        p_outDisplayName = entryPtr->displayName;
                        changed = true;
                    }
                    else
                    {

                    }
                }
            }
            if (const ImGuiPayload *pl =
                AcceptDragDropPayload(PAYLOAD_ASSET.data()))
            {
                if (pl->DataSize % sizeof(const Droplet::MetaEntry *) == 0)
                {
                    const auto *entries =
                        static_cast<const Droplet::MetaEntry *const *>(pl->Data);

                    const std::size_t count =
                        pl->DataSize / sizeof(const Droplet::MetaEntry *);

                    for (std::size_t i = 0; i < count; ++i)
                    {
                        const Droplet::MetaEntry *entry = entries[i];

                        if (entry && entry->type == p_expectedType)
                        {
                            p_outGUID = entry->guid;
                            p_outDisplayName = entry->displayName;
                            changed = true;
                            break;
                        }
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
            p_outDisplayName.clear();
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

void MeshComponent::StartImpl()
{

}

void MeshComponent::UpdateImpl([[maybe_unused]] float p_deltaTime)
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
            m_meshDisplayName,
            newGUID,
            newName,
            Droplet::ResourceType::Mesh))
        {
            SetMeshGUID(newGUID, resourceManager);
			m_meshDisplayName = newName;
        }

        newGUID = C_INVALID_GUID;
        newName.clear();

        if (ResourceSlot(
            "Texture",
            m_2dTexture.GetGUID(),
            m_textureDisplayName,
            newGUID,
            newName,
            Droplet::ResourceType::Texture2D))
        {
            SetTextureGUID(newGUID, resourceManager);
			m_textureDisplayName = newName;
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

	json["meshDisplayName"] = m_meshDisplayName;
	json["textureDisplayName"] = m_textureDisplayName;

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

	if (p_compJson.contains("meshDisplayName"))
	{
		m_meshDisplayName = p_compJson["meshDisplayName"].get<std::string>();
	}

	if (p_compJson.contains("textureDisplayName"))
	{
		m_textureDisplayName = p_compJson["textureDisplayName"].get<std::string>();
	}
}
