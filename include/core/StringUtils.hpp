#pragma once

#include <string>

namespace Droplet::StringUtils
{
    /// @brief Converts a string to lowercase in-place to avoid copying and allocating.
    void ToLowerInPlace(std::string &p_string);
    
    /// @return A lowercase copy of the provided string.
    [[nodiscard]] std::string ToLower(std::string_view p_string);
}