#pragma once

#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include "FontResource.hpp"
#include "Common/RenderData.hpp"

namespace Droplet::Graphics::Text
{
	struct TextLayoutResult
	{
		std::vector<Common::GlyphInstance> instances;
		glm::vec2 minBounds{};
		glm::vec2 maxBounds{};
	};
	
	[[nodiscard]] TextLayoutResult BuildTextLayout(
		const FontResource& p_font,
		const std::string_view &p_text,
		float p_emSize
	);
	
	[[nodiscard]] std::vector<std::uint32_t> DecodeUTF8(const std::string_view &p_text);
}
