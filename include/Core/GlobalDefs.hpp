#pragma once

#include <cstdint>
#include <limits>

namespace Droplet
{
	using GUID = std::uint64_t;
	
	inline constexpr GUID G_INVALID_GUID { 0 };
	inline constexpr std::uint32_t G_INVALID_INDEX { std::numeric_limits<std::uint32_t>::max() };
}
