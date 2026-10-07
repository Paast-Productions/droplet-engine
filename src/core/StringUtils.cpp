#include "core/StringUtils.hpp"

#include <algorithm>

namespace Droplet::StringUtils
{
    void ToLowerInPlace(std::string &p_string)
    {
        std::transform(p_string.begin(), p_string.end(), p_string.begin(),
            [](unsigned char c) { return std::tolower(c); });
    }
    
    std::string ToLower(std::string_view p_string)
    {
        std::string copy(p_string);
        ToLowerInPlace(copy);
        
        return copy;
    }
}
