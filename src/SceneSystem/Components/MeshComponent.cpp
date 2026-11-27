#include "MeshComponent.hpp"

#include <SceneSystem/Scene.hpp>
#include <Resource/ResourceManager.hpp>
#include <SceneSystem/SceneResourceLoad.hpp>
#include <Common/RenderData.hpp>
#include <Engine/Engine.hpp>
#include <Engine/EngineFlags.hpp>
#include <Editor/DragDropFields.hpp>
#include <Editor/ResourceSlot.hpp>
#include <string>
#include <SceneSystem/Components/CameraComponent.hpp>

#include <algorithm>
#include <chrono>
#include <vector>

#ifdef _EDITOR
#include <Editor/ShaderOverride.hpp>
#endif

// imgui helper function

using namespace Droplet;
using namespace Droplet::Scene;

#ifdef _EDITOR
namespace
{
    /// @brief Target of every camera in the scene as a texture choice
    std::vector<ResourceSlotChoice> CameraChoices(const std::vector<std::shared_ptr<Node>> &p_roots) 
    {
        std::vector<ResourceSlotChoice> choices;
        const auto addCameras = [&choices](const std::shared_ptr<Node> &p_node) 
        {
            for (const auto &camera : p_node->GetComponents<CameraComponent>())
            {
                if (camera->GetTarget().id != C_INVALID_GUID)
                {
                    choices.push_back( {"Camera: " + p_node->GetName(), camera->GetTarget().id});
                }
            }
        };

        for (const auto &root : p_roots) 
        {
            if (!root)
                continue;
            addCameras(root);
            for (const auto &child : root->GetChildren(true))
            {
                addCameras(child);
            }
        }
        return choices;
    }
}
#endif


MeshComponent::MeshComponent() : m_bounds(this)
{
	if (EngineFlagsOwner::IsUpdateFlagSet(UpdateFlags::InEditor) && !EngineFlagsOwner::IsUpdateFlagSet(UpdateFlags::InPlayMode))
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
	//TODO: (COMPONENT) will be inplemted later when the mesh system is implemented
}

void MeshComponent::QueueRenderImpl()
{
	GetOwner()->GetScene()->QueueRenderable(shared_from_this());
}

Droplet::Common::RenderData MeshComponent::GetRenderDataImpl()
{
	Droplet::Common::RenderData renderData{};
	renderData.fsData.guid = m_shaderData.fs.GetGUID();
	renderData.tcsData.guid = m_shaderData.tcs.GetGUID();
	renderData.tesData.guid = m_shaderData.tes.GetGUID();
	renderData.gsData.guid = m_shaderData.gs.GetGUID();
	renderData.vsData.guid = m_shaderData.vs.GetGUID();
	
#ifdef _EDITOR

	ResourceManager &rsMan = ResourceManager::GetInstance();
	MetaEntry entry;

	if (Droplet::Editor::Graphics::ShaderOverride::GetVertexFlag() == Droplet::Editor::Graphics::VertexFlag::Basic)
	{
		if (rsMan.GetResourceMetaEntry("Base.VS", entry))
		{
			renderData.vsData.guid = entry.guid;
		}
	}

	switch (Droplet::Editor::Graphics::ShaderOverride::GetFragmentFlag())
	{
	default:
		break;
	case Droplet::Editor::Graphics::FragmentFlag::Basic:
		if (rsMan.GetResourceMetaEntry("Base.FS", entry))
		{
			renderData.fsData.guid = entry.guid;
		}
		break;
	case Droplet::Editor::Graphics::FragmentFlag::Color:
		if (rsMan.GetResourceMetaEntry("Color.FS", entry))
		{
			renderData.fsData.guid = entry.guid;
		}
		break;
	case Droplet::Editor::Graphics::FragmentFlag::Normal:
		if (rsMan.GetResourceMetaEntry("Normal.FS", entry))
		{
			renderData.fsData.guid = entry.guid;
		}
		break;
	case Droplet::Editor::Graphics::FragmentFlag::UV:
		if (rsMan.GetResourceMetaEntry("UV.FS", entry))
		{
			renderData.fsData.guid = entry.guid;
		}
		break;
	}
#endif

	renderData.meshData.GUID = m_mesh.GetGUID();
	renderData.texData.GUID = GetTextureGUID();
	renderData.texData.isRenderTarget = m_textureIsTarget;

	renderData.transparent = m_transparent;

	return renderData;
}

void MeshComponent::RenderUIImpl()
{
#ifdef _EDITOR
	using namespace ImGui;

	Checkbox("Transparent", &m_transparent);

	PushID(this);

	if (BeginTable("MeshProps", 2, ImGuiTableFlags_SizingStretchProp))
	{
		TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, 80.0f);
		TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

		GUID newGUID = C_INVALID_GUID;
		std::string newName;

		// Retrieve the existing resource names from ResourceManager here.
		// Replace these examples with your actual lookup API.

        if (ResourceSlot(
            "Mesh",
            m_mesh.GetGUID(),
            m_meshDisplayName,
            newGUID,
            newName,
            Droplet::StaticResourceType::Mesh))
        {
            SetMeshGUID(newGUID);
			m_meshDisplayName = newName;
		}

		newGUID = C_INVALID_GUID;
		newName.clear();

		const auto scene = GetOwner()->GetScene();
		const std::vector<ResourceSlotChoice> cameraChoices = scene
			? CameraChoices(scene->GetRoots())
			: std::vector<ResourceSlotChoice>{};

        if (ResourceSlot(
            "Texture",
            GetTextureGUID(),
            m_textureDisplayName,
            newGUID,
            newName,
            Droplet::StaticResourceType::Texture2D,
            Droplet::ShaderResource::ShaderType::Vertex,
            cameraChoices))
        {
            const bool isCamera = newGUID != C_INVALID_GUID &&
                std::ranges::any_of(cameraChoices, [newGUID](const ResourceSlotChoice &p_choice)
                {
                    return p_choice.guid == newGUID;
                });
            if (isCamera)
            {
                SetTextureTarget(newGUID, newName);
            }
            else
            {
                SetTextureGUID(newGUID);
            }
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
            Droplet::StaticResourceType::Shader,
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
            Droplet::StaticResourceType::Shader,
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
            Droplet::StaticResourceType::Shader,
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
            Droplet::StaticResourceType::Shader,
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
            Droplet::StaticResourceType::Shader,
            Droplet::ShaderResource::ShaderType::Fragment))
        {
            SetFSGUID(newGUID);
            m_fsDisplayName = newName;
        }
        
        EndTable();
    }
	
	PopID();
#endif
}

nlohmann::json MeshComponent::SerializeImpl()
{
	nlohmann::json json;

	// GUIDS

	json["meshGUID"] = GetMeshGUID();
	json["textureGUID"] = GetTextureGUID();
	json["textureIsRenderTarget"] = m_textureIsTarget;

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

	// Settings
	json["transparent"] = m_transparent;

	return json;
}

void MeshComponent::DeserializeImpl([[maybe_unused]] const nlohmann::json p_compJson)
{
	// GUIDS

	if (p_compJson.contains("meshGUID"))
	{
		SetMeshGUID(p_compJson["meshGUID"].get<GUID>());
	}

	if (p_compJson.contains("textureGUID"))
	{
		const GUID textureGUID = p_compJson["textureGUID"].get<GUID>();
		if (p_compJson.value("textureIsRenderTarget", false))
		{
			SetTextureTarget(textureGUID, p_compJson.value("textureDisplayName", std::string{}));
		}
		else
		{
			SetTextureGUID(textureGUID);
		}
	}

	// Shader GUIDS

    if (p_compJson.contains("vs"))
    {
        SetVSGUID(p_compJson["vs"]);
    }
    if (p_compJson.contains("tcs"))
    {
        SetTCSGUID(p_compJson["tcs"]);
    }
    if (p_compJson.contains("tes"))
    {
        SetTESGUID(p_compJson["tes"]);
    }
    if (p_compJson.contains("gs"))
    {
        SetGSGUID(p_compJson["gs"]);
    }
    if (p_compJson.contains("fs"))
    {
        SetFSGUID(p_compJson["fs"]);

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

	// Settings
	if (p_compJson.contains("transparent"))
	{
		m_transparent = p_compJson["transparent"].get<bool>();
	}
}

namespace
{
	/// @brief The MeshComponent slot that a dropped resource can be applied to.
	enum class MeshSlotTarget
	{
		None,
		Mesh,
		Texture,
		VS,
		TCS,
		TES,
		GS,
		FS
	};

	MeshSlotTarget GetMeshSlotTarget(const MetaEntry &p_entry)
	{
		switch (p_entry.type)
		{
		case StaticResourceType::Mesh:
			return MeshSlotTarget::Mesh;
		case StaticResourceType::Texture2D:
			return MeshSlotTarget::Texture;
		case StaticResourceType::Shader:
		{
			const auto shaderType = p_entry.loadSettings.value(MetaLoadSettings::C_SHADER_TYPE.key, MetaLoadSettings::C_SHADER_TYPE.defaultValue);
			switch (shaderType)
			{
			case ShaderResource::ShaderType::Vertex:                 return MeshSlotTarget::VS;
			case ShaderResource::ShaderType::TessellationControl:    return MeshSlotTarget::TCS;
			case ShaderResource::ShaderType::TessellationEvaluation: return MeshSlotTarget::TES;
			case ShaderResource::ShaderType::Geometry:               return MeshSlotTarget::GS;
			case ShaderResource::ShaderType::Fragment:               return MeshSlotTarget::FS;
			default:                                                 return MeshSlotTarget::None;
			}
		}
		default:
			return MeshSlotTarget::None;
		}
	}
}

bool MeshComponent::CanApplyResource(const MetaEntry &p_entry) const
{
	return GetMeshSlotTarget(p_entry) != MeshSlotTarget::None;
}

bool MeshComponent::TryApplyResource(const MetaEntry &p_entry)
{
	switch (GetMeshSlotTarget(p_entry))
	{
	case MeshSlotTarget::Mesh:    SetMeshGUID(p_entry.guid); return true;
	case MeshSlotTarget::Texture: SetTextureGUID(p_entry.guid); return true;
	case MeshSlotTarget::VS:      SetVSGUID(p_entry.guid); return true;
	case MeshSlotTarget::TCS:     SetTCSGUID(p_entry.guid); return true;
	case MeshSlotTarget::TES:     SetTESGUID(p_entry.guid); return true;
	case MeshSlotTarget::GS:      SetGSGUID(p_entry.guid); return true;
	case MeshSlotTarget::FS:      SetFSGUID(p_entry.guid); return true;
	default:                      return false;
	}
}

void MeshComponent::SetMeshGUID(GUID p_guid)
{
	m_mesh = LoadTrackedResource<MeshResource>(TryGetScene(), p_guid);
	if (MetaEntry entry; ResourceManager::GetInstance().GetResourceMetaEntry(p_guid, entry))
	{
		m_meshDisplayName = entry.displayName;
	}
}

void MeshComponent::SetTextureTarget(GUID p_targetId, const std::string& p_displayName)
{
    m_2dTexture = {};
    m_textureTarget = p_targetId;
    m_textureIsTarget = p_targetId != C_INVALID_GUID;
    m_textureDisplayName = p_displayName;
}


void MeshComponent::SetTextureGUID(GUID p_guid)
{
	m_textureIsTarget = false;
	m_textureTarget = C_INVALID_GUID;
	m_2dTexture = LoadTrackedResource<Texture2DResource>(TryGetScene(), p_guid);
	if (MetaEntry entry; ResourceManager::GetInstance().GetResourceMetaEntry(p_guid, entry))
	{
		m_textureDisplayName = entry.displayName;
	}
}

void MeshComponent::SetVSGUID(GUID p_guid)
{ 
	m_shaderData.vs = LoadTrackedResource<ShaderResource>(TryGetScene(), p_guid); 
	if (MetaEntry entry; ResourceManager::GetInstance().GetResourceMetaEntry(p_guid, entry))
	{
		m_vsDisplayName = entry.displayName;
	}
}

void MeshComponent::SetTCSGUID(GUID p_guid)
{
	m_shaderData.tcs = LoadTrackedResource<ShaderResource>(TryGetScene(), p_guid);
	if (MetaEntry entry; ResourceManager::GetInstance().GetResourceMetaEntry(p_guid, entry))
	{
		m_tcsDisplayName = entry.displayName;
	}
}

void MeshComponent::SetTESGUID(GUID p_guid)
{
	m_shaderData.tes = LoadTrackedResource<ShaderResource>(TryGetScene(), p_guid);
	if (MetaEntry entry; ResourceManager::GetInstance().GetResourceMetaEntry(p_guid, entry))
	{
		m_tesDisplayName = entry.displayName;
	}
}

void MeshComponent::SetGSGUID(GUID p_guid)
{
	m_shaderData.gs = LoadTrackedResource<ShaderResource>(TryGetScene(), p_guid);
	if (MetaEntry entry; ResourceManager::GetInstance().GetResourceMetaEntry(p_guid, entry))
	{
		m_gsDisplayName = entry.displayName;
	}
}

void MeshComponent::SetFSGUID(GUID p_guid)
{ 
	m_shaderData.fs = LoadTrackedResource<ShaderResource>(TryGetScene(), p_guid); 
	if (MetaEntry entry; ResourceManager::GetInstance().GetResourceMetaEntry(p_guid, entry))
	{
		m_fsDisplayName = entry.displayName;
	}
}
