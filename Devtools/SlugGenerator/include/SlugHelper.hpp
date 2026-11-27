#pragma once

#include <cstdint>
#include <glm/vec2.hpp>

namespace Droplet::Devtools::SlugGenerator
{
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
}
