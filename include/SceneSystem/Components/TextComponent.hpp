#pragma once

#include <SceneSystem/Component.hpp>
#include <resource/StaticResourceHandle.hpp>
#include <resource/types/FontResource.hpp>

#include <string_view>

#include "TextLayout.hpp"

namespace Droplet::Scene
{
	/// @brief Component that attaches a text to a Node.
	class TextComponent : public Component
	{
	public:
		TextComponent();
		
		/// @brief Gets the type name of the component.
		/// @return The type name of the component.
		[[nodiscard]] std::string_view GetTypeName() const override { return "TextComponent"; }
		
		/// @brief Getter-function for Font Resource.
		/// @return Resource Handle to a Font Resource
		[[nodiscard]] const StaticResourceHandle<FontResource> &GetFontResource() { return m_font; }
		
		/// @brief Sets the font resource using its GUID
		/// @param p_guid GUID identifying the font resource to load.
		void SetFontGUID(GUID p_guid);
		
		[[nodiscard]] GUID GetFontGUID() const { return m_font.GetGUID(); }
		
		/// @brief Gets the local-space bounds of the rendered text.
		/// @return The text bounds, or std::nullopt if no drawable glyphs exist.
		[[nodiscard]] std::optional<Math::AABB> GetLocalRenderBounds() const;
		
	protected:
		void StartImpl() override;
		void UpdateImpl() override;
		void EditorUpdateImpl() override;
		void QueueRenderImpl() override;
		void RenderUIImpl() override;
		[[nodiscard]] Graphics::RenderData GetRenderDataImpl() override;
		
		nlohmann::json SerializeImpl() override;
		void DeserializeImpl(const nlohmann::json p_compJson) override;
		
	private:
		std::string m_fontDisplayName;
		
		StaticResourceHandle<FontResource> m_font{};
		bool m_textIsDirty = true; // Used to decide whether the text layout should be updated.
		
		glm::vec4 m_textColor = {1, 1, 1, 1};
		float m_emSize = 0.5f;
		std::string m_text = "Text";
		
		std::shared_ptr<const std::vector<Graphics::Text::GlyphInstance>> m_glyphInstances;
		std::optional<Math::AABB> m_localRenderBounds;
	};
}
