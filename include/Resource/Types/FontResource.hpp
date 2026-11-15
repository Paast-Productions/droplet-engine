#pragma once
#include "resource/IStaticResource.hpp"

namespace Droplet
{
	class FontResource : public IStaticResource
	{
	public:
		#pragma pack(push, 1)
				/// @brief Font metadata
				/// 
				/// Contains information that is common for all glyphs within the font.
				/// 
				/// @note Serialized directly into the Slug binary file. Changes require regeneration of .slug files.
				struct SlugFontHeader
				{
					uint16_t unitsPerEm = 0;
				
					uint32_t ascent = 0; 
					uint32_t descent = 0;
					uint32_t lineGap = 0;
				};
			
				/// @brief Metadata required to render a processed Slug glyph.
				///
				/// Describes the glyph's dimensions, band configuration, and
				/// location of its band headers within the shared band texture.
				///
				/// @note Serialized directly into the Slug binary file. Changes require regeneration of .slug files.
				struct SlugCodePoint
				{
					/// @brief Unicode code point represented by the glyph.
					uint32_t codePoint;
				
					/// @brief Width of the glyph's bounding box in font units.
					uint32_t width;
				
					/// @brief Height of the glyph's bounding box in font units.
					uint32_t height;
				
					/// @brief Spacing to the next character.
					int32_t advanceWidth;
				
					/// @brief Minimum x-value of the glyph bounding box.
					int32_t xMin;
				
					/// @brief Minimum y-value of the glyph bounding box.
					int32_t yMin;
				
					/// @brief Number of horizontal and vertical bands per glyph.
					uint32_t bandCount;
				
					/// @brief Width of each vertical band in glyph-local units.
					uint32_t bandDimX;
				
					/// @brief Height of each horizontal band in glyph-local units.
					uint32_t bandDimY;
				
					/// @brief X texel coordinate of the glyph's first band header.
					uint16_t bandsTexCoordX;
				
					/// @brief Y texel coordinate of the glyph's first band header.
					uint16_t bandsTexCoordY;
				};
		#pragma pack(pop)
		
		const SlugCodePoint* GetGlyph(uint32_t p_codePoint) const;
		
		const SlugFontHeader& GetFontHeader() const;
		
	private:
		std::string m_name{}; // Name of the font
		
		std::vector<SlugCodePoint> m_codePoints; // Metadata of decoded glyphs
		std::vector<glm::vec4> m_curvesTextureData; // RGBA texture containing information about all glyph curves in the font
		std::vector<glm::u16vec2> m_bandsTextureData; // RG uint texture containing information....
		SlugFontHeader m_fontHeader; // Metadata about the font
		
		uint16_t m_curveTextureWidth = 0;
		uint16_t m_curveTextureHeight = 0;
		
		uint16_t m_bandsTextureWidth = 0;
		uint16_t m_bandsTextureHeight = 0;
	};
}
