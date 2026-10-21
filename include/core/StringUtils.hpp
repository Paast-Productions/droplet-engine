#pragma once

#include <string>

namespace Droplet::StringUtils
{
    /// @brief Transforms a string to lowercase in-place to avoid copying and allocating.
    /// @param p_string The string to transform.
    void ToLowerInPlace(std::string &p_string);
    
    /// @brief Transforms a string into lowercase.
    /// @param p_string The string to transform.
    /// @return A lowercase copy of the provided string.
    [[nodiscard]] std::string ToLower(std::string_view p_string);
}