#pragma once
#include "resource/IStaticResource.hpp"
#include <glm/gtc/type_precision.hpp>

namespace Droplet
{
	class FontResource : public IStaticResource
	{
	public:
		/// @brief Width, in texels, of the generated curve and band textures.
		static constexpr uint32_t TEXTURE_WIDTH = 4096;
	
		/// @brief Magic identifier used to recognize Slug binary files.
		static constexpr char SLUG_HEADER_DATA[] = "SLUG";
	
		/// @brief Number of bytes written for the Slug file identifier.
		static constexpr size_t SLUG_HEADER_LEN = 4;
	
		static constexpr char SLUG_FILE_VERSION[] = "2";
		static constexpr size_t SLUG_FILE_VERSION_LEN = 1;
		
		// Ensure that we're packing tightly
		#pragma pack(push, 1)
			/// @brief Font metadata
			/// 
			/// Contains information that is common for all glyphs within the font.
			/// 
			/// @note Serialized directly into the Slug binary file. Changes require regeneration of .slug files.
			struct SlugFontHeader
			{
				uint16_t unitsPerEm = 0;
			
				int32_t ascent = 0; 
				int32_t descent = 0;
				int32_t lineGap = 0;
			};
		
			/// @brief Metadata required to render a processed Slug glyph.
			///
			/// Describes the glyph's dimensions, band configuration, and
			/// location of its band headers within the shared band texture.
			///
			/// @note Serialized directly into the Slug binary file. Changes require regeneration of .slug files.
			struct SlugGlyph
			{
				/// @brief Unicode code point represented by the glyph.
				uint32_t codePoint = 0;
			
				/// @brief Width of the glyph's bounding box in font units.
				uint32_t width = 0;
			
				/// @brief Height of the glyph's bounding box in font units.
				uint32_t height = 0;
			
				/// @brief Spacing to the next character.
				int32_t advanceWidth = 0;
			
				/// @brief Minimum x-value of the glyph bounding box.
				int32_t xMin = 0;
			
				/// @brief Minimum y-value of the glyph bounding box.
				int32_t yMin = 0;
			
				/// @brief Number of horizontal and vertical bands per glyph.
				uint32_t bandCount = 0;
			
				/// @brief Width of each vertical band in glyph-local units.
				uint32_t bandDimX = 0;
			
				/// @brief Height of each horizontal band in glyph-local units.
				uint32_t bandDimY = 0;
			
				/// @brief X texel coordinate of the glyph's first band header.
				uint16_t bandsTexCoordX = 0;
			
				/// @brief Y texel coordinate of the glyph's first band header.
				uint16_t bandsTexCoordY = 0;
			};
		#pragma pack(pop)

		struct LoadedFontData
		{
			std::string name;
			
			SlugFontHeader fontHeader{}; // Metadata about the font
			std::vector<SlugGlyph> glyphs; // Metadata of decoded glyphs
			
			std::vector<glm::vec4> curvesTextureData; // RGBA texture containing information about all glyph curves in the font
			std::vector<glm::u16vec2> bandsTextureData; // RG uint texture containing information....
		
			uint16_t curveTextureWidth = 0;
			uint16_t curveTextureHeight = 0;
		
			uint16_t bandsTextureWidth = 0;
			uint16_t bandsTextureHeight = 0;
		};

		explicit FontResource(LoadedFontData&& p_data);
		
		static std::unique_ptr<FontResource> CreateFallback();
		
		/// @brief Get metadata for given code point
		/// @return Pointer to metadata header
		const SlugGlyph* GetGlyph(uint32_t p_codePoint) const;
		
		/// @brief Get the font metadata
		/// @return Reference to the metadata header associated with the font
		const SlugFontHeader &GetFontHeader() const;
		
		/// @brief Get curves texture
		/// @return Reference to vec4 vector containing the curve texture data
		const std::vector<glm::vec4> &GetCurvesTextureData() const;
		
		/// @brief Get bands texture
		/// @return Reference to uint16 vec2 vector containing the band texture data
		const std::vector<glm::u16vec2> &GetBandsTextureData() const;
		
		const std::string &GetName() const;
		
	private:
		std::string m_name{}; // Name of the font
		
		SlugFontHeader m_fontHeader{}; // Metadata about the font
		std::vector<SlugGlyph> m_glyphs; // Metadata of decoded glyphs
		
		std::vector<glm::vec4> m_curvesTextureData; // RGBA texture containing information about all glyph curves in the font
		std::vector<glm::u16vec2> m_bandsTextureData; // RG uint texture containing information....
		
		uint16_t m_curveTextureWidth = 0;
		uint16_t m_curveTextureHeight = 0;
		
		uint16_t m_bandsTextureWidth = 0;
		uint16_t m_bandsTextureHeight = 0;
	};
}
