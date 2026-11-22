#pragma once
#include <cstdint>
#include <functional>
#include <string_view>

#include <Resource/GuidUtils.hpp>

/// @brief A deterministic GUID from a name (FNV-1a), for the renderer's built-in assets
constexpr Droplet::GUID GuidFromName(std::string_view name) noexcept
{
    std::uint64_t hash = 14695981039346656037ull;
    for (const char c : name)
    {
        hash ^= static_cast<unsigned char>(c);
        hash *= 1099511628211ull;
    }
    return hash;
}

/// Stable identifier of the sandbox's own scene. The renderer takes Droplet::GUID.
struct Guid
{
    std::uint64_t value = 0;

    constexpr bool IsValid() const noexcept { return value != 0; }
    friend constexpr bool operator==(Guid, Guid) = default;

    // CONVERT: the sandbox scene's Guid to production's GUID, which the renderer takes
    constexpr operator Droplet::GUID() const noexcept { return value; }

    /// Deterministic ID from a name (FNV-1a)
    static constexpr Guid FromName(std::string_view name) noexcept { return Guid{ GuidFromName(name) }; }
};

template <>
struct std::hash<Guid>
{
    std::size_t operator()(Guid guid) const noexcept { 
        return std::hash<std::uint64_t>{}(guid.value);
     }
};
