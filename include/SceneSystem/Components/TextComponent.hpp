#pragma once

#include <SceneSystem/Component.hpp>
#include <resource/StaticResourceHandle.hpp>
#include <resource/types/FontResource.hpp>
#include <Common/RenderData.hpp>

#include <string_view>

#include "TextLayout.hpp"
#include "SceneSystem/Bounds/DefaultNodeBounds.hpp"

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
		
		
		/// @brief Determines whether the component provides custom bounds.
		/// @return True if the component provides custom bounds, otherwise false.
		[[nodiscard]] bool HasBoundsOverride() const override { return true; }

		/// @brief Gets the custom bounds provided by the component.
		/// @return The bounding box provided by the component.
		[[nodiscard]] const NodeBounds *GetBounds() const override { return &m_bounds; }
		
	protected:
		void StartImpl() override;
		void UpdateImpl() override;
		void EditorUpdateImpl() override;
		void QueueRenderImpl() override;
		void RenderUIImpl() override;
		[[nodiscard]] Common::RenderData GetRenderDataImpl() override;
		
		nlohmann::json SerializeImpl() override;
		void DeserializeImpl(const nlohmann::json p_compJson) override;
		
	private:
		void UpdateBounds(bool p_hasGlyphs, const glm::vec2& p_min, const glm::vec2& p_max);
		
		/// @brief Checks if the text is in need of being updated, and rebuilds if that is the case.
		void RebuildText();
		
		std::string m_fontDisplayName;
		
		StaticResourceHandle<FontResource> m_font{};
		bool m_textIsDirty = true; // Used to decide whether the text layout should be updated.
		
		glm::vec4 m_textColor = {1, 1, 1, 1};
		float m_emSize = 0.5f;
		std::string m_text = "Text";
		
		std::shared_ptr<const std::vector<Graphics::Text::GlyphInstance>> m_glyphInstances;
		DefaultNodeBounds m_bounds{glm::vec3(0.0f), glm::vec3(0.5f) };
	};
}
