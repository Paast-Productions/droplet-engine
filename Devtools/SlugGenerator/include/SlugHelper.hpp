#pragma once

#include <cstdint>
#include <glm/vec2.hpp>

namespace Droplet::Devtools::SlugGenerator
{
	/// @brief Width, in texels, of the generated curve and band textures.
	inline constexpr uint32_t TEXTURE_WIDTH = 4096;
	
	/// @brief Magic identifier used to recognize Slug binary files.
	inline constexpr char SLUG_HEADER_DATA[] = "SLUG";
	
	/// @brief Number of bytes written for the Slug file identifier.
	inline constexpr size_t SLUG_HEADER_LEN = 4;
	
	

	/// @brief Represents a quadratic Bézier curve.
	///
	/// Contains three control points in glyph-local coordinates,
	/// as well as metadata used when packing the curve texture.
	struct Curve
	{
		/// @brief Starting point of the curve.
		glm::vec2 p1;
		
		/// @brief Control point of the curve.
		glm::vec2 p2;
		
		/// @brief Ending point of the curve.
		glm::vec2 p3;
		
		/// @brief Index of the first texel used to decode this curve.
		///
		/// Each curve is decoded using two adjacent RGBA texels.
		/// Connected curves may share control-point data.
		uint32_t texelIndex;

		/// @brief Whether this curve begins a new contour.
		///
		/// The first curve of each contour must store its starting
		/// point explicitly, rather than reusing the previous endpoint.
		bool first;
	};

	/// @brief Represents a glyph's bounding box in font coordinates.
	///
	/// Contains the minimum and maximum X and Y coordinates
	/// of a glyph, as returned by stb_truetype.
	struct GlyphBounds
	{
		int xMin;
		int yMin;
		int xMax;
		int yMax;
		
		/// @brief Returns the minimum X coordinate as a float.
		[[nodiscard]] float GetMinX() const { return static_cast<float>(xMin); }
		
		/// @brief Returns the minimum Y coordinate as a float.
		[[nodiscard]] float GetMinY() const { return static_cast<float>(yMin); }
		
		/// @brief Returns the maximum X coordinate as a float.
		[[nodiscard]] float GetMaxX() const { return static_cast<float>(xMax); }
		
		/// @brief Returns the maximum Y coordinate as a float.
		[[nodiscard]] float GetMaxY() const { return static_cast<float>(yMax); }
	};
	
#pragma pack(push, 1)
	/// @brief Metadata required to render a processed Slug glyph.
	///
	/// Describes the glyph's dimensions, band configuration, and
	/// location of its band headers within the shared band texture.
	///
	/// @note Serialized directly into the Slug binary file.
	///       The structure must retain its expected 28-byte layout.
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
	
	static_assert(sizeof(SlugCodePoint) == 40, "Unexpected SlugCodePoint binary layout.");
}
