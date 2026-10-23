#include "MeshComponent.hpp"

#include <ImGui/imgui.h>

#include <Editor/DragDropFields.hpp>
#include <Resource/ResourceManager.hpp>
#include <Graphics/RenderData.hpp>
#include <Engine/Engine.hpp>
#include <string>
#include <Editor/ResourceSlot.hpp>

// imgui helper function

using namespace Droplet::Scene;

MeshComponent::MeshComponent() : m_bounds(this)
{
}

void MeshComponent::StartImpl()
{
    SetRenderable();
}

void MeshComponent::UpdateImpl()
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

    p_renderData.texData.GUID = m_2dTexture.GetGUID();
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
            SetMeshGUID(newGUID);
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
            SetTextureGUID(newGUID);
			m_textureDisplayName = newName;
        }
        
        Separator();
        
        if (ResourceSlot(
            "Vertex Shader",
            m_shaderData.vs.GetGUID(),
            m_vsDisplayName,
            newGUID,
            newName,
            Droplet::ResourceType::Shader))
        {
            SetVSGUID(newGUID);
            m_vsDisplayName = newName;
        }
        
        if (ResourceSlot(
            "Tes Control Shader",
            m_shaderData.tcs.GetGUID(),
            m_tcsDisplayName,
            newGUID,
            newName,
            Droplet::ResourceType::Shader))
        {
            SetTCSGUID(newGUID);
            m_tcsDisplayName = newName;
        }
        
        if (ResourceSlot(
            "Tes Evaluation Shader",
            m_shaderData.tes.GetGUID(),
            m_tesDisplayName,
            newGUID,
            newName,
            Droplet::ResourceType::Shader))
        {
            SetTESGUID(newGUID);
            m_tesDisplayName = newName;
        }
        
        if (ResourceSlot(
            "Geometry Shader",
            m_shaderData.gs.GetGUID(),
            m_gsDisplayName,
            newGUID,
            newName,
            Droplet::ResourceType::Shader))
        {
            SetGSGUID(newGUID);
            m_gsDisplayName = newName;
        }

        if (ResourceSlot(
            "Fragment Shader",
            m_shaderData.fs.GetGUID(),
            m_fsDisplayName,
            newGUID,
            newName,
            Droplet::ResourceType::Shader))
        {
            SetFSGUID(newGUID);
            m_fsDisplayName = newName;
        }
        
        EndTable();
    }

    PopID();
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
		SetMeshGUID(p_compJson["meshGUID"].get<Droplet::GUID>());
	}

	if (p_compJson.contains("textureGUID"))
	{
		SetTextureGUID(p_compJson["textureGUID"].get<Droplet::GUID>());
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
