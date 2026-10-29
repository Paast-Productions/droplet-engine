#pragma once
// TODO: Change the header name...

#include <cstdint>
#include <glm/glm.hpp>

namespace Droplet::Devtools::SlugGenerator
{
	#define TEXTURE_WIDTH 4096 
	
	struct Curve
	{
		glm::vec2 p1;
		glm::vec2 p2;
		glm::vec2 p3;
		
		uint32_t texelIndex;
		bool first;
	};

	struct GlyphBounds
	{
		int x1;
		int y1;
		int x2;
		int y2;
		
		[[nodiscard]] float GetMinX() const { return static_cast<float>(x1); }
		[[nodiscard]] float GetMinY() const { return static_cast<float>(y1); }
		[[nodiscard]] float GetMaxX() const { return static_cast<float>(x2); }
		[[nodiscard]] float GetMaxY() const { return static_cast<float>(y2); }
	};
	
#pragma pack(push, 1)
	struct SlugCodePoint
	{
		uint32_t codePoint;
		uint32_t width;
		uint32_t height;
		uint32_t bandCount;
		uint32_t bandDimX;
		uint32_t bandDimY;
		uint16_t bandsTexCoordX;
		uint16_t bandsTexCoordY;
	};
#pragma pack(pop)
}
