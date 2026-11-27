#include "Text/TextLayout.hpp"

Droplet::Graphics::Text::TextLayoutResult Droplet::Graphics::Text::BuildTextLayout(const FontResource &p_font,
	const std::string_view &p_text, float p_emSize)
{
	ZoneScopedN("Text / Layout")
	
	glm::vec2 minBounds{std::numeric_limits<float>::max()};
	glm::vec2 maxBounds{std::numeric_limits<float>::lowest()};
	
	const FontResource::SlugFontHeader& fontHeader = p_font.GetFontHeader();
	float fontScale = p_emSize / static_cast<float>(fontHeader.unitsPerEm);
	const float lineHeight = static_cast<float>(fontHeader.ascent - fontHeader.descent + fontHeader.lineGap) * fontScale;
	
	TextLayoutResult result{};
	glm::vec2 pen{0.0f};
	result.instances.reserve((p_text.size()));
	for (uint32_t codePoint : DecodeUTF8(p_text))
	{
		if (codePoint == U'\n')
		{
			pen.x = 0.0f;
			pen.y -= lineHeight;
			continue;
		}
		
		const FontResource::SlugGlyph* glyph = p_font.GetGlyph(codePoint);
		
		float advance = 0.0f;
		bool shouldRender = false;
		Common::GlyphInstance instance{};
		
		if (!glyph)
		{
			advance = p_emSize * 0.6f;
			const float width = p_emSize * 0.5f;
			const float height = p_emSize * 0.7f;
			
			instance = {
				.scaleBias = {
					width,
					height,
					pen.x + advance * 0.5f,
					pen.y + height * 0.5f,
				},
				.glyphBandScale = glm::vec4(0.0f),
				.bandMaxTexCoords = glm::uvec4(0)
			};
			shouldRender = true; // We want to render a "missing char" glyph
		}
		else if (glyph->bandCount > 0 && glyph->bandDimX && glyph->bandDimY)
		{
			const float width = static_cast<float>(glyph->width);
			const float height = static_cast<float>(glyph->height);
			
			const float quadWidth = width * fontScale;
			const float quadHeight = height * fontScale;
			
			const float glyphX = pen.x + static_cast<float>(glyph->xMin) * fontScale;
			const float glyphY = pen.y + static_cast<float>(glyph->yMin) * fontScale;
			
			instance = {
				.scaleBias = {
					quadWidth,
					quadHeight,
					glyphX + quadWidth * 0.5f,
					glyphY + quadHeight * 0.5f
				},
				.glyphBandScale = {
					width,
					height,
					width / static_cast<float>(glyph->bandDimX),
					height / static_cast<float>(glyph->bandDimY)
				},
				.bandMaxTexCoords = {
					glyph->bandCount - 1,
					glyph->bandCount - 1,
					glyph->bandsTexCoordX,
					glyph->bandsTexCoordY
				}
			};
			
			shouldRender = true;
		}
		
		if (glyph)
			advance = static_cast<float>(glyph->advanceWidth) * fontScale;
		
		pen.x += advance;
		
		if (shouldRender)
		{
			const glm::vec2 halfSize = glm::vec2(instance.scaleBias) * 0.5f;
			const glm::vec2 center = glm::vec2(instance.scaleBias.z, instance.scaleBias.w);
			
			minBounds = glm::min(minBounds, center - halfSize);
			maxBounds = glm::max(maxBounds, center + halfSize);
			
			result.instances.push_back(instance);
		}
	}
	
	if (!result.instances.empty())
	{
		result.minBounds = minBounds;		
		result.maxBounds = maxBounds;		
	}
	else
	{
		result.minBounds = glm::vec2(0.0f);
		result.maxBounds = glm::vec2(0.0f);
	}
	
	return result;
}

std::vector<std::uint32_t> Droplet::Graphics::Text::DecodeUTF8(const std::string_view &p_text)
{
	constexpr uint32_t REPLACEMENT_CHARACTER = 0xFFFD;
	
	std::vector<std::uint32_t> codePoints;

	for (std::size_t i = 0; i < p_text.size();)
	{
		const auto first = static_cast<std::uint8_t>(p_text[i]);

		std::uint32_t codePoint = 0;
		std::size_t length = 0;

		if (first < 0x80)
		{
			codePoint = first;
			length = 1;
		}
		else if ((first & 0xE0) == 0xC0)
		{
			codePoint = first & 0x1F;
			length = 2;
		}
		else if ((first & 0xF0) == 0xE0)
		{
			codePoint = first & 0x0F;
			length = 3;
		}
		else if ((first & 0xF8) == 0xF0)
		{
			codePoint = first & 0x07;
			length = 4;
		}
		else
		{
			codePoints.push_back(REPLACEMENT_CHARACTER);
			++i;
			continue;
		}

		if (length> p_text.size() - i)
		{
			codePoints.push_back(REPLACEMENT_CHARACTER);
			++i;
			continue;
		}

		bool valid = true;
		
		for (std::size_t j = 1; j < length; ++j)
		{
			const auto byte = static_cast<std::uint8_t>(p_text[i + j]);

			if ((byte & 0xC0) != 0x80)
			{
				valid = false;
				break;
			}

			codePoint = (codePoint << 6) | (byte & 0x3F);
		}

		if (!valid ||
			(length == 2 && codePoint < 0x80) ||
			(length == 3 && codePoint < 0x800) ||
			(length == 4 && codePoint < 0x10000) ||
			(codePoint >= 0xD800 && codePoint <= 0xDFFF) ||
			codePoint > 0x10FFFF)
		{
			codePoints.push_back(REPLACEMENT_CHARACTER);
			++i;
			continue;
		}

		codePoints.push_back(codePoint);
		i += length;
	}

	return codePoints;
}
