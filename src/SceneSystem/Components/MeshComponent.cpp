#include "MeshComponent.hpp"

#include <string>

#include <ImGui/imgui.h>

#include <Editor/DragDropFields.hpp>
#include <Resource/ResourceManager.hpp>
#include <Graphics/RenderData.hpp>
#include <Engine/Engine.hpp>

// imgui helper function

namespace
{
    bool ResourceSlot(
        const char *p_label,
        Droplet::GUID p_current,
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
            OpenPopup("pick");
        }

        // Drag and drop
        if (BeginDragDropTarget())
        {
            // TODO: ENSURE PAYLOAD_RESOURCE IS NULL TERMINATED 
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

void MeshComponent::GetRenderData(Graphics::RenderData &p_renderData)
{
    p_renderData.fsData.GUID = m_shaderData.fs.GetGUID();
    p_renderData.tcsData.GUID = m_shaderData.tcs.GetGUID();
    p_renderData.tesData.GUID = m_shaderData.tes.GetGUID();
    p_renderData.gsData.GUID = m_shaderData.gs.GetGUID();
    p_renderData.vsData.GUID = m_shaderData.vs.GetGUID();

    p_renderData.meshData.GUID = m_mesh.GetGUID();
}

void MeshComponent::RenderUIImpl()
{
    using namespace ImGui;

    PushID(this);
    if (BeginTable("MeshProps", 2, ImGuiTableFlags_SizingStretchProp))
    {
        TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

        Droplet::GUID newGUID = C_INVALID_GUID;

        if(ResourceSlot(
            "Mesh",
            m_mesh.GetGUID(),
            newGUID,
            Droplet::ResourceType::Mesh))
        {
            SetMeshGUID(newGUID);
        }
    
        if (ResourceSlot(
            "Texture",
            m_2dTexture.GetGUID(),
            newGUID,
            Droplet::ResourceType::Texture2D))
        {
            SetTextureGUID(newGUID);
        }
        
        Separator();
        
        if(ResourceSlot(
            "Vertex Shader",
            m_shaderData.vs.GetGUID(),
            newGUID,
            Droplet::ResourceType::Shader))
        {
            SetVSGUID(newGUID);
        }
        
        if(ResourceSlot(
            "Tes Control Shader",
            m_shaderData.tcs.GetGUID(),
            newGUID,
            Droplet::ResourceType::Shader))
        {
            SetTCSGUID(newGUID);
        }
        
        if(ResourceSlot(
            "Tes Evaluation Shader",
            m_shaderData.tes.GetGUID(),
            newGUID,
            Droplet::ResourceType::Shader))
        {
            SetTESGUID(newGUID);
        }
        
        if(ResourceSlot(
            "Geometry Shader",
            m_shaderData.gs.GetGUID(),
            newGUID,
            Droplet::ResourceType::Shader))
        {
            SetGSGUID(newGUID);
        }
        
        if(ResourceSlot(
            "Fragment Shader",
            m_shaderData.fs.GetGUID(),
            newGUID,
            Droplet::ResourceType::Shader))
        {
            SetFSGUID(newGUID);
        }
        
        EndTable();
    }
    PopID();
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
		SetMeshGUID(p_compJson["meshGUID"].get<Droplet::GUID>());
	}

	if (p_compJson.contains("textureGUID"))
	{
		SetTextureGUID(p_compJson["textureGUID"].get<Droplet::GUID>());
	}
}
