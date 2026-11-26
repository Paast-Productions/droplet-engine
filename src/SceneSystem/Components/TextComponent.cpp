#include "TextComponent.hpp"

#include <Resource/ResourceManager.hpp>
#include <Engine/EngineFlags.hpp>
#include <SceneSystem/Node.hpp>
#include <SceneSystem/Scene.hpp>
#include <Editor/ResourceSlot.hpp>
#include <ImGui/imgui_stdlib.h>
#include <glm/gtc/type_ptr.hpp>

#include "Text/TextLayout.hpp"

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
	
	m_glyphInstances.reset();
	m_textIsDirty = true;
}

void Droplet::Scene::TextComponent::StartImpl()
{
	SetRenderable();
}

void Droplet::Scene::TextComponent::UpdateImpl()
{
	if (!EngineFlagsOwner::IsUpdateFlagSet(UpdateFlags::InPlayMode))
		return;
	
	if (!IsActive())
		return;
	
	RebuildText();
}

void Droplet::Scene::TextComponent::EditorUpdateImpl()
{
	if (!EngineFlagsOwner::IsUpdateFlagSet(UpdateFlags::InEditor))
		return;
	
	if (!IsActive())
		return;
	
	RebuildText();
}

void Droplet::Scene::TextComponent::QueueRenderImpl()
{
	if (!m_font.IsReady() || m_textIsDirty || !m_glyphInstances || m_glyphInstances->empty())
		return;
	
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

Droplet::Graphics::RenderData Droplet::Scene::TextComponent::GetRenderDataImpl()
{
	Graphics::RenderData data{};
	
	data.textData = Graphics::TextData{
		.font = m_font,
		.color = m_textColor,
		.glyphInstances = m_glyphInstances
	};
	
	data.transparent = true;
	
	return data;
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
	
	m_textIsDirty = true;
}

void Droplet::Scene::TextComponent::UpdateBounds(bool p_hasGlyphs, const glm::vec2& p_min, const glm::vec2& p_max)
{
	if (!p_hasGlyphs)
	{
		constexpr float thickness = 0.001f;

		const glm::vec2 center =
			(p_min + p_max) * 0.5f;

		const glm::vec2 extents =
			(p_max - p_min) * 0.5f;

		m_bounds = DefaultNodeBounds{
			glm::vec3(center, 0.0f),
			glm::vec3(extents, thickness)
		};
	}
	else
	{
		// No drawable text.
		m_bounds = DefaultNodeBounds{
			glm::vec3(0.0f),
			glm::vec3(0.001f)
		};
	}
}

void Droplet::Scene::TextComponent::RebuildText()
{
	if (!m_textIsDirty || !m_font.IsReady())
		return;
	
	const FontResource* font = m_font.Get();
	
	if (!font)
		return;
	
	auto layout = Graphics::Text::BuildTextLayout(*font, m_text, m_emSize);
	
	bool hasGlyphs = !layout.instances.empty();
	UpdateBounds(hasGlyphs, layout.minBounds, layout.maxBounds);
	
	m_glyphInstances = std::make_shared<const std::vector<Graphics::Text::GlyphInstance>>(std::move(layout.instances));
	
	GetOwner()->GetTransform().MakeDirty();
	m_textIsDirty = false;
}

