#include "HashUtils.hpp"

#include <unordered_map>

using namespace Droplet::Algorithm::Hash;

template <class T>
void Utilities::Combine(const T& p_value, std::size_t& p_result)
{
    std::hash<T> hash {};
    
    // Magic number
    // For context: https://stackoverflow.com/questions/19195183/how-to-properly-hash-the-custom-struct
    p_result ^= hash(p_value) + 0x9e3779b9 + (p_result << 6) + (p_result >> 2);
}
