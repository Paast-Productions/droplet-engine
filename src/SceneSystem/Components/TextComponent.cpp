#include "TextComponent.hpp"

#include <Resource/ResourceManager.hpp>
#include <Engine/EngineFlags.hpp>
#include <SceneSystem/Node.hpp>
#include <SceneSystem/Scene.hpp>
#include <Editor/ResourceSlot.hpp>
#include <ImGui/imgui_stdlib.h>
#include <glm/gtc/type_ptr.hpp>

Droplet::Scene::TextComponent::TextComponent()
{
	if (EngineFlagsOwner::IsUpdateFlagSet(UpdateFlags::EditorFlags))
	{
		ResourceManager &rsMan = ResourceManager::GetInstance();
		MetaEntry entry;
		
		if (rsMan.GetResourceMetaEntry("JetBrainsMono-Regular", entry))
		{
			SetFontGUID(entry.guid);
		}
	}
}

void Droplet::Scene::TextComponent::SetFontGUID(GUID p_guid)
{
	m_font = ResourceManager::GetInstance().LoadResource<FontResource>(p_guid);
	if (MetaEntry entry; ResourceManager::GetInstance().GetResourceMetaEntry(p_guid, entry))
	{
		m_fontDisplayName = entry.displayName;
	}
}

void Droplet::Scene::TextComponent::StartImpl()
{
	SetRenderable();
}

void Droplet::Scene::TextComponent::UpdateImpl()
{
	// TODO: MeshComponent has a todo here - I don't know
}

void Droplet::Scene::TextComponent::QueueRenderImpl()
{
	GetOwner()->GetScene()->QueueRenderable(shared_from_this());
}

void Droplet::Scene::TextComponent::RenderUIImpl()
{
#ifdef _EDITOR
	using namespace ImGui;
	
	PushID(this);
	
	if (BeginTable("TextProps", 2, ImGuiTableFlags_SizingStretchProp))
	{
		TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, 80.0f);
		TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);
		
		GUID newGUID = C_INVALID_GUID;
		std::string newName;
		
		if (ResourceSlot(
			"Font",
			m_font.GetGUID(),
			m_fontDisplayName,
			newGUID,
			newName,
			StaticResourceType::Font))
		{
			SetFontGUID(newGUID);
			m_fontDisplayName = newName;
		}
		
		PushID("Font size");
		TableNextRow();
		TableNextColumn();
		AlignTextToFramePadding();
		TextUnformatted("Font size");
		TableNextColumn();
		if (InputFloat("##emSize", &m_emSize, 0.1f, 1.0f, "%.1f"))
		{
			m_textIsDirty = true;
		}
		SameLine();
		
		ImGui::TextDisabled("(?)");
		if (ImGui::BeginItemTooltip())
		{
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::Text("em-to-world conversion - 1 em = %.1f units", m_emSize);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
		PopID();
		
		PushID("Color");
		TableNextRow();
		TableNextColumn();
		AlignTextToFramePadding();
		TextUnformatted("Color");
		TableNextColumn();
		ColorEdit4("##textColor", glm::value_ptr(m_textColor), ImGuiColorEditFlags_Float);
		PopID();
		
		Separator();
		
		PushID("Text");
		TableNextRow();
		TableNextColumn();
		AlignTextToFramePadding();
		TextUnformatted("Text");
		TableNextColumn();
		if (InputTextMultiline("##input", &m_text, ImVec2(-FLT_MIN, GetTextLineHeight() * 16)))
		{
			m_textIsDirty = true;
		}
		PopID();
		
		EndTable();
	}
	
	PopID();
#endif
}

nlohmann::json Droplet::Scene::TextComponent::SerializeImpl()
{
	nlohmann::json json = {
		{"fontGUID", GetFontGUID()},
		{"textColor", {
			m_textColor.r,
			m_textColor.g,
			m_textColor.b,
			m_textColor.a
		}},
		{"emSize", m_emSize},
		{"displayText", m_text}
	};
	
	return json;
}

void Droplet::Scene::TextComponent::DeserializeImpl(const nlohmann::json p_compJson)
{
	if (p_compJson.contains("fontGUID"))
	{
		SetFontGUID(p_compJson["fontGUID"].get<GUID>());
	}
	
	if (p_compJson.contains("textColor"))
	{
		const auto& color = p_compJson["textColor"];
		
		m_textColor = glm::vec4(
			color[0].get<float>(),
			color[1].get<float>(),
			color[2].get<float>(),
			color[3].get<float>()
		);
	}
	
	if (p_compJson.contains("emSize"))
	{
		m_emSize = p_compJson["emSize"].get<float>();
	}
	
	if (p_compJson.contains("displayText"))
	{
		m_text = p_compJson["displayText"].get<std::string>();
	}
}

