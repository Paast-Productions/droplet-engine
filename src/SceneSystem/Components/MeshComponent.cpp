#include "MeshComponent.hpp"

#include <ImGui/imgui.h>

#include <Editor/DragDropFields.hpp>
#include <Resource/ResourceManager.hpp>
#include <Graphics/RenderData.hpp>
#include <Engine/Engine.hpp>
#include <Engine/EngineFlags.hpp>
#include <string>
#include <Editor/ResourceSlot.hpp>

// imgui helper function

using namespace Droplet;
using namespace Droplet::Scene;

MeshComponent::MeshComponent() : m_bounds(this)
{
    if (EngineFlagsOwner::IsUpdateFlagSet(UpdateFlags::EditorFlags))
    {
        // Set default resources

		ResourceManager &rsMan = ResourceManager::GetInstance();
        MetaEntry entry; 

        // Mesh
        if (rsMan.GetResourceMetaEntry("Cube", entry))
        {
			SetMeshGUID(entry.guid);
        }

		// Texture
        if (rsMan.GetResourceMetaEntry("White", entry))
        {
			SetTextureGUID(entry.guid);
        }

		// Vertex Shader
        if (rsMan.GetResourceMetaEntry("Base.VS", entry))
        {
			SetVSGUID(entry.guid);
        }

		// Fragment Shader
        if (rsMan.GetResourceMetaEntry("Base.FS", entry))
        {
            SetFSGUID(entry.guid);
        }
    }
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

        newGUID = C_INVALID_GUID;
        newName.clear();

        if (ResourceSlot(
            "Vertex Shader",
            m_shaderData.vs.GetGUID(),
            m_vsDisplayName,
            newGUID,
            newName,
            Droplet::ResourceType::Shader,
            Droplet::ShaderResource::ShaderType::Vertex))
        {
            SetVSGUID(newGUID);
            m_vsDisplayName = newName;
        }
        newGUID = C_INVALID_GUID;
        newName.clear();

        if (ResourceSlot(
            "Tes Control Shader",
            m_shaderData.tcs.GetGUID(),
            m_tcsDisplayName,
            newGUID,
            newName,
            Droplet::ResourceType::Shader,
            Droplet::ShaderResource::ShaderType::TessellationControl))
        {
            SetTCSGUID(newGUID);
            m_tcsDisplayName = newName;
        }
        newGUID = C_INVALID_GUID;
        newName.clear();

        if (ResourceSlot(
            "Tes Evaluation Shader",
            m_shaderData.tes.GetGUID(),
            m_tesDisplayName,
            newGUID,
            newName,
            Droplet::ResourceType::Shader,
            Droplet::ShaderResource::ShaderType::TessellationEvaluation))
        {
            SetTESGUID(newGUID);
            m_tesDisplayName = newName;
        }
        newGUID = C_INVALID_GUID;
        newName.clear();

        if (ResourceSlot(
            "Geometry Shader",
            m_shaderData.gs.GetGUID(),
            m_gsDisplayName,
            newGUID,
            newName,
            Droplet::ResourceType::Shader,
            Droplet::ShaderResource::ShaderType::Geometry))
        {
            SetGSGUID(newGUID);
            m_gsDisplayName = newName;
        }
        newGUID = C_INVALID_GUID;
        newName.clear();

        if (ResourceSlot(
            "Fragment Shader",
            m_shaderData.fs.GetGUID(),
            m_fsDisplayName,
            newGUID,
            newName,
            Droplet::ResourceType::Shader,
            Droplet::ShaderResource::ShaderType::Fragment))
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

    // Shaders GUIDS
    json["vs"] = GetVSGUID();
    json["tcs"] = GetTCSGUID();
    json["tes"] = GetTESGUID();
    json["gs"] = GetGSGUID();
    json["fs"] = GetFSGUID();

	// Names

	json["meshDisplayName"] = m_meshDisplayName;
	json["textureDisplayName"] = m_textureDisplayName;

    // Shader Names 

    json["vsName"] = m_vsDisplayName;
    json["tcsName"] = m_tcsDisplayName;
    json["tesName"] = m_tesDisplayName;
    json["gsName"] = m_gsDisplayName;
    json["fsName"] = m_fsDisplayName;


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

    // Shader GUIDS

    if (!p_compJson.contains("vs"))
    {
        throw std::runtime_error("Meshcomponent missing required field 'vs'");
    }
    if (!p_compJson.contains("tcs"))
    {
        throw std::runtime_error("Meshcomponent missing required field 'tcs'");
    }
    if (!p_compJson.contains("tes"))
    {
        throw std::runtime_error("Meshcomponent missing required field 'tes'");
    }
    if (!p_compJson.contains("gs"))
    {
        throw std::runtime_error("Meshcomponent missing required field 'gs'");
    }
    if (!p_compJson.contains("fs"))
    {
        throw std::runtime_error("Meshcomponent missing required field 'fs'");
    }

    SetVSGUID(p_compJson["vs"]);
    SetTCSGUID(p_compJson["tcs"]);
    SetTESGUID(p_compJson["tes"]);
    SetGSGUID(p_compJson["gs"]);
    SetFSGUID(p_compJson["fs"]);

	// Names

	if (!p_compJson.contains("meshDisplayName"))
	{
        throw std::runtime_error("Meshcomponent missing required field 'meshDisplayName'");
	}
    m_meshDisplayName = p_compJson["meshDisplayName"].get<std::string>();

	if (!p_compJson.contains("textureDisplayName"))
	{
        throw std::runtime_error("Meshcomponent missing required field 'textureDisplayName'");
	}
    m_textureDisplayName = p_compJson["textureDisplayName"].get<std::string>();


    // Shader Names

    if (!p_compJson.contains("vsName"))
    {
        throw std::runtime_error("Meshcomponent missing required field 'textureDisplayName'");
    }
    if (!p_compJson.contains("tcsName"))
    {
        throw std::runtime_error("Meshcomponent missing required field 'textureDisplayName'");
    }
    if (!p_compJson.contains("tesName"))
    {
        throw std::runtime_error("Meshcomponent missing required field 'textureDisplayName'");
    }
    if (!p_compJson.contains("gsName"))
    {
        throw std::runtime_error("Meshcomponent missing required field 'textureDisplayName'");
    }
    if (!p_compJson.contains("fsName"))
    {
        throw std::runtime_error("Meshcomponent missing required field 'textureDisplayName'");
    }

    m_vsDisplayName = p_compJson["vsName"].get<std::string>();
    m_tcsDisplayName = p_compJson["tcsName"].get<std::string>();
    m_tesDisplayName = p_compJson["tesName"].get<std::string>();
    m_gsDisplayName = p_compJson["gsName"].get<std::string>();
    m_fsDisplayName = p_compJson["fsName"].get<std::string>();
}
