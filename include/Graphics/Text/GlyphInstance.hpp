#pragma once
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

namespace Droplet::Graphics::Text
{
	/// @brief Contains the per-glyph data required for instanced text rendering.
	/// @details Stores the glyph's transformation, dimensions, color, and
	/// band metadata used by the Slug rendering shader.
	struct GlyphInstance
	{
		/// @brief Glyph quad scale (xy) and position (zw).
		glm::vec4 scaleBias;

		/// @brief Glyph dimensions (xy) and band scaling factors (zw).
		glm::vec4 glyphBandScale;

		/// @brief Maximum horizontal and vertical band indices (xy),
		/// and starting coordinates in the band texture (zw).
		glm::uvec4 bandMaxTexCoords;
	};
	
	struct TextPushConstants
	{
		glm::mat4 model{1.0f};
		glm::vec4 color{1.0f};
	};
}