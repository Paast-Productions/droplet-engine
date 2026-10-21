#pragma once
// TODO: Change the header name...

#include <cstdint>
#include <glm/glm.hpp>

struct Curve
{
	glm::vec2 p1;
	glm::vec2 p2;
	glm::vec2 p3;
	
	uint32_t texelIndex;
	bool first;
};