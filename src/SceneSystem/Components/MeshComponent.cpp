#include "MeshComponent.hpp"

#include <ImGui/imgui.h>
#include <Editor/DragDropFields.hpp>
#include <resource/ResourceManager.hpp>
#include <Engine/Engine.hpp>
#include <string>

// imgui helper function

namespace
{
    bool ResourceSlot(
        const char *p_label,
        Droplet::GUID p_current,
        Droplet::GUID &p_pickBuffer,
        Droplet::GUID &p_outGUID,
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
            ? "GUID: " + std::to_string(p_current)
            : "None (drop resource here)";

        if (Button(text.c_str(), ImVec2(-(clearW + gap), 0.0f)))
        {
            p_pickBuffer = p_current;
            OpenPopup("pick");
        }

        // Drag and drop
        if (BeginDragDropTarget())
        {
            if (const ImGuiPayload *pl =
                AcceptDragDropPayload(PAYLOAD_RESOURCE.data()))
            {
                if (pl->DataSize == sizeof(ResourcePayload))
                {
                    const auto *resourcePayload =
                        static_cast<const ResourcePayload *>(pl->Data);

                    if (resourcePayload->type == p_expectedType)
                    {
                        p_outGUID = resourcePayload->guid;
                        changed = true;
                    }
                    else
                    {
                        // Optional: report incompatible resource type.
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

        // Clear button
        SameLine();
        BeginDisabled(p_current == Droplet::C_INVALID_GUID);

        if (Button("X", ImVec2(clearW, clearW)))
        {
            p_outGUID = Droplet::C_INVALID_GUID;
            changed = true;
        }

        EndDisabled();

        // Manual GUID popup
        if (BeginPopup("pick"))
        {
            InputScalar("GUID", ImGuiDataType_U64, &p_pickBuffer);

            if (Button("Assign"))
            {
                p_outGUID = p_pickBuffer;
                changed = true;
                CloseCurrentPopup();
            }

            SameLine();

            if (Button("Cancel"))
                CloseCurrentPopup();

            EndPopup();
        }

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

        if(ResourceSlot(
            "Mesh",
            m_mesh.GetGUID(),
            m_meshPickGUID,
            newGUID,
            Droplet::ResourceType::Mesh))
        {
            SetMeshGUID(newGUID, resourceManager);
        }

        if (ResourceSlot(
            "Texture",
            m_2dTexture.GetGUID(),
            m_texturePickGUID,
            newGUID,
            Droplet::ResourceType::Texture2D))
        {
            SetTextureGUID(newGUID, resourceManager);
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
