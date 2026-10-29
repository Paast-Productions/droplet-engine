#pragma once
#include <Stb/stb_truetype.h>
#include <filesystem>
#include <glm/vec2.hpp>

#include "SlugHelper.hpp"

namespace Droplet::Devtools::SlugGenerator
{
	class FontProcessor
	{
	public:
		explicit FontProcessor(const std::filesystem::path& p_path, uint32_t p_bandCount = 16, int p_asciiRangeStart = 33, int p_asciiRangeEnd = 126);
		
		FontProcessor(const FontProcessor&) = delete;
		FontProcessor& operator=(const FontProcessor&) = delete;
		
		bool IsValid() const { return m_isValid; } // TODO: HasFailed() instead?
		const std::string& GetError() const { return m_err; }
	
	private:
		bool OpenFont();
		bool ProcessCodePoint(int p_codePoint);
		bool SaveData(const std::filesystem::path& p_outPath);
		
		bool Error(const std::string& p_err);
		
		GlyphBounds GetGlyphBounds(int p_glyphIndex) const;
		static glm::vec2 ToGlyphLocal(const glm::vec2& p_glyphOffset, const float p_x, const float p_y);
		
		bool m_isValid = true;
		std::string m_err;
		uint32_t m_bandCount = 16;
		
		stbtt_fontinfo m_fontInfo;
		std::filesystem::path m_path;
		
		std::vector<char> m_fontData;
		
		std::vector<float> m_curvesTexture;
		std::vector<uint16_t> m_bandsTextureBandOffsets;
		std::vector<uint16_t> m_bandsTextureCurveOffsets;
		std::vector<SlugCodePoint> m_codePoints;
	};

	// Custom deleter to use unique_ptr to contain the glyph vertices, and then have it free the shape correctly
	struct StbVertexDeleter
	{
		const stbtt_fontinfo* font;
		void operator()(stbtt_vertex* vertices) const
		{
			if (vertices)
				stbtt_FreeShape(font, vertices);
		}
	};
}
