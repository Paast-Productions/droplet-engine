#include "resource/types/FontResource.hpp"

namespace Droplet
{
	FontResource::FontResource(LoadedFontData &&p_data)
		:	m_name(std::move(p_data.name)),
			m_fontHeader(p_data.fontHeader),
			m_glyphs(std::move(p_data.glyphs)),
			m_curvesTextureData(std::move(p_data.curvesTextureData)),
			m_bandsTextureData(std::move(p_data.bandsTextureData)),
			m_curveTextureWidth(p_data.curveTextureWidth),
			m_curveTextureHeight(p_data.curveTextureHeight),
			m_bandsTextureWidth(p_data.bandsTextureWidth),
			m_bandsTextureHeight(p_data.bandsTextureHeight)
	{
	}

	std::unique_ptr<FontResource> FontResource::CreateFallback()
	{
		return nullptr;
	}

	const FontResource::SlugGlyph *FontResource::GetGlyph(uint32_t p_codePoint) const
	{
		const auto it = std::ranges::find(
			m_glyphs,
			p_codePoint,
			&SlugGlyph::codePoint
		);
	
		// Return the pointer to the glyph metadata
		return it != m_glyphs.end() ? &(*it) : nullptr;
	}

	const FontResource::SlugFontHeader &FontResource::GetFontHeader() const
	{
		return m_fontHeader;
	}

	const std::vector<glm::vec4> &FontResource::GetCurvesTextureData() const
	{
		return m_curvesTextureData;
	}

	const std::vector<glm::u16vec2> &FontResource::GetBandsTextureData() const
	{
		return m_bandsTextureData;
	}

	uint16_t FontResource::GetCurveTextureWidth() const
	{
		return m_curveTextureWidth;
	}

	uint16_t FontResource::GetCurveTextureHeight() const
	{
		return m_curveTextureHeight;
	}

	uint16_t FontResource::GetBandsTextureWidth() const
	{
		return m_bandsTextureWidth;
	}

	uint16_t FontResource::GetBandsTextureHeight() const
	{
		return m_bandsTextureHeight;
	}

	const std::string &FontResource::GetName() const
	{
		return m_name;
	}
}
