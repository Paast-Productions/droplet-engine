/**
 * @file Guid.hpp
 * @brief Core Guid implementation
 */
#pragma once

#include <cstdint>
#include <string_view> // also includes std::hash

#include "Random.hpp"

namespace Droplet
{
    /// Guid type, created from string with Guid::FromName or randomly generated with Guid::Generate()
    struct Guid
    {
        std::uint64_t value = INVALID;
        static constexpr std::uint64_t INVALID = 0;

        constexpr bool IsValid() const { return value != INVALID; }
        
        /// Implicit conversion so a Guid can be used wherever a uint64_t is expected
        constexpr operator std::uint64_t() const noexcept { return value; }
        
        /// Convenience accessor to value, *guid
        constexpr std::uint64_t operator*() const noexcept { return value; }
        
        friend constexpr bool operator==(Guid, Guid) = default;
        
        /// Hash a string into a Guid
        [[nodiscard]] static constexpr Guid FromName(std::string_view name) noexcept
        {
            std::uint64_t hash = 14695981039346656037ull;
            for (const char c : name)
            {
                hash ^= static_cast<unsigned char>(c);
                hash *= 1099511628211ull;
            }
            return Guid{hash};
        }
        /// Random, never invalid Guid
        [[nodiscard]] static Guid Generate()
        {
            Guid guid{Random::Uint64()};
            while (!guid.IsValid())
            {
                guid = Guid{Random::Uint64()};
            }
            return guid;
        }
    };
}

/// Enable hashing of Guid into std::unordered_map
template <>
struct std::hash<Droplet::Guid>
{
    std::size_t operator()(Droplet::Guid guid) const noexcept { 
        return std::hash<std::uint64_t>{}(guid.value);
     }
};
