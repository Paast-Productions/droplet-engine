#pragma once

#include <cstddef>

namespace Droplet::Algorithm::Hash
{
    class Utilities
    {
    public:
        
        template<class T>
        static void Combine(const T &p_value, std::size_t &p_result);
    };
}

