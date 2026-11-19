#pragma once
#include <cstdint>
#include <functional>
#include <string_view>

/// Stable identifier shared by scene, resource store and renderer. No Vulkan, no scene types.
struct Guid
{
    std::uint64_t value = 0;

    constexpr bool IsValid() const noexcept { return value != 0; }
    friend constexpr bool operator==(Guid, Guid) = default;

    /// Deterministic ID from a name (FNV-1a)
    static constexpr Guid FromName(std::string_view name) noexcept
    {
        std::uint64_t hash = 14695981039346656037ull;
        for (const char c : name)
        {
            hash ^= static_cast<unsigned char>(c);
            hash *= 1099511628211ull;
        }
        return Guid{hash};
    }
};

template <>
struct std::hash<Guid>
{
    std::size_t operator()(Guid guid) const noexcept { 
        return std::hash<std::uint64_t>{}(guid.value);
     }
};
