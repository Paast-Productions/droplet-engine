#pragma once
#include <Stb/stb_truetype.h>
#include <filesystem>
#include <glm/vec2.hpp>

#include "SlugHelper.hpp"

namespace Droplet::Devtools::SlugGenerator
{
	/// @brief Generates Slug font data from a TrueType font.
	///
	/// Loads a font, processes a specified range of code points,
	/// and generates the curve and band textures required by the
	/// Slug rendering algorithm.
	///
	/// Successfully generated data is serialized to a .slug file
	/// alongside the source font.
	///
	/// @note Individual unsupported or missing glyphs are skipped.
	///       Use IsValid() to determine whether the overall
	///       generation and serialization succeeded.
	class FontProcessor
	{
	public:
		/// @brief Constructs a font processor and generates Slug data.
		///
		/// @param p_path Path to the source TrueType font.
		/// @param p_destination Path to the directory in which to output .slug files.
		/// @param p_bandCount Requested number of horizontal and
		///        vertical bands per glyph. May be reduced for small glyphs.
		/// @param p_asciiRangeStart First code point to process (inclusive).
		/// @param p_asciiRangeEnd Last code point to process (inclusive).
		///
		/// @note Construction performs the complete generation process.
		///       Initialization failures are reported through IsValid()
		///       and GetError().
		explicit FontProcessor(const std::filesystem::path& p_path, const std::filesystem::path& p_destination, uint32_t p_bandCount = 16, int p_asciiRangeStart = 33, int p_asciiRangeEnd = 126);
		
		FontProcessor(const FontProcessor&) = delete;
		FontProcessor& operator=(const FontProcessor&) = delete;
		
		/// @brief Returns whether font generation and serialization succeeded.
		[[nodiscard]] bool IsValid() const { return m_isValid; }
		
		/// @brief Returns the stored error message, if any.
		[[nodiscard]] const std::string& GetError() const { return m_err; }
	
	private:
		/// @brief Loads the source font and initializes stb_truetype.
		///
		/// Reads the font into m_fontData and initializes m_fontInfo.
		/// The font data is retained because stbtt_fontinfo references
		/// the original buffer rather than copying it.
		///
		/// @return true on success, false otherwise.
		bool OpenFont();
		
		/// @brief Processes a code point into GPU-ready Slug font data.
		/// 
		/// @details
		/// Extracts the glyph outline using stb_truetype and converts its segments into
		/// quadratic Bézier curves in glyph-local coordinates. The curves are then encoded
		/// into the shared curve texture, reusing endpoints between connected curves.
		/// 
		/// The glyph is divided into horizontal and vertical bands, each containing
		/// references to curves that may contribute to coverage within that band.
		/// These references are stored in two separate arrays:
		/// 
		/// - Band headers: Each entry contains the number of curves in a band and an
		///   offset to its curve-reference list. Horizontal bands precede vertical bands.
		/// - Curve references: Each entry contains the X and Y texel coordinates of
		///   a curve in the shared curve texture.
		/// 
		/// Both arrays represent sections of the final RG16UI band texture, with the
		/// band headers preceding the curve references. During generation, band offsets
		/// are relative to the curve-reference section. They are adjusted in SaveData()
		/// to account for the preceding band headers.
		/// 
		/// Successfully processed glyphs also contribute metadata describing their
		/// dimensions, band configuration, and location within the band texture.
		/// 
		/// @param p_codePoint The ASCII code point of the glyph to process.
		/// @return true if the glyph was successfully processed, false otherwise.
		/// 
		/// @note The curve and band textures are shared across all processed glyphs.
		///       A GlyphTransaction is used to roll back appended texture data if
		///       processing fails.
		bool ProcessCodePoint(int p_codePoint);
		
		/// @brief Serializes the generated font data to a binary file.
		///
		/// Adjusts band offsets to their final texture-relative
		/// positions and writes the file header, code-point metadata,
		/// curve texture, and band texture.
		///
		/// @param p_outPath Destination path of the .slug file.
		/// @return true if serialization succeeded, false otherwise.
		///
		/// @note Currently uses native-endian serialization.
		/// @note The current implementation modifies band offsets
		///       in place and is intended to be called only once.
		bool SaveData(const std::filesystem::path& p_outPath);
		
		/// @brief Records a fatal processing error.
		///
		/// Stores the error message and marks the processor invalid.
		///
		/// @param p_err Description of the error.
		/// @return Always returns false, allowing direct use in
		///         failure return statements.
		bool Error(const std::string& p_err);
		
		/// @brief Retrieves a glyph's bounding box in font coordinates.
		///
		/// @param p_glyphIndex Glyph index in the TrueType font.
		/// @return The glyph's minimum and maximum coordinates.
		GlyphBounds GetGlyphBounds(int p_glyphIndex) const;
		
		/// @brief Translates a point into glyph-local coordinates.
		///
		/// Subtracts the minimum bounding-box coordinates, placing
		/// the glyph's bounding-box origin at (0, 0).
		///
		/// @param p_glyphOffset Minimum X and Y coordinates of the glyph.
		/// @param p_x Original X coordinate.
		/// @param p_y Original Y coordinate.
		/// @return The translated point in glyph-local coordinates.
		static glm::vec2 ToGlyphLocal(const glm::vec2& p_glyphOffset, const float p_x, const float p_y);
		
		/// @brief Converts a glyph outline into quadratic Bézier curves.
		///
		/// Processes TrueType move, line, and quadratic-curve segments.
		/// Straight lines are represented as quadratic Bézier curves
		/// using the midpoint between their endpoints.
		///
		/// Curves are translated into glyph-local coordinates and
		/// marked when they begin a new contour. Degenerate control
		/// points are corrected before returning the curves.
		///
		/// @param p_vertices Pointer to the glyph's outline vertices.
		/// @param p_numVerts Number of vertices in the outline.
		/// @param p_glyphOffset Minimum glyph bounding-box coordinates.
		/// @return The extracted quadratic Bézier curves.
		///
		/// @pre p_vertices must reference at least p_numVerts valid
		///      vertices, and p_numVerts must be non-negative.
		std::vector<Curve> ExtractCurves(const stbtt_vertex* p_vertices, int p_numVerts, glm::vec2 p_glyphOffset);
		
		bool m_isValid = false;
		std::string m_err;
		uint32_t m_bandCount = 16;
		
		stbtt_fontinfo m_fontInfo{};
		std::filesystem::path m_path;
		
		/// @brief Font bytes retained for the lifetime of m_fontInfo.
		std::vector<char> m_fontData;
		
		/// @brief Packed Bézier control points (RGBA32F).
		std::vector<float> m_curvesTexture;
		
		/// @brief Band headers containing curve counts and reference offsets.
		std::vector<uint16_t> m_bandsTextureBandOffsets;
		
		/// @brief Curve references containing texture X/Y coordinates.
		std::vector<uint16_t> m_bandsTextureCurveOffsets;
		
		/// @brief Metadata describing successfully processed code points.
		std::vector<SlugCodePoint> m_codePoints;
	};
	
	/// @brief Custom deleter for glyph vertices allocated by stb_truetype.
	///
	/// Calls stbtt_FreeShape() using the same font information that
	/// was used to allocate the glyph outline.
	///
	/// Intended for use with std::unique_ptr.
	///
	/// @note The referenced stbtt_fontinfo must outlive the deleter's
	///       invocation.
	struct StbVertexDeleter
	{
		const stbtt_fontinfo* font;
		
		/// @brief Releases a glyph outline allocated by stb_truetype.
		/// @param vertices Pointer to the allocated vertices.
		void operator()(stbtt_vertex* vertices) const
		{
			if (vertices)
				stbtt_FreeShape(font, vertices);
		}
	};
}
