#pragma once

#include <cstdint>

namespace Droplet
{
    using GUID = uint64_t;
    static constexpr GUID C_INVALID_GUID = 0;
    
    namespace GuidUtils
    {
        /// @return True if the GUID is valid (non-zero), otherwise false.
        bool IsValid(GUID guid);

        /// @brief Generates a new 64-bit GUID.
        /// @return The newly generated GUID.
        static GUID Generate();
    }
}
