/**
 * @brief A random number generator that can be used globally
 */
#pragma once

#include <concepts>
#include <cstdint>
#include <random>

namespace Droplet
{
    /// Singleton class for generating random numbers
    class Random
    {
    public:
        Random() = delete; // singleton
        
        /// reseeds the random number generator with the given seed for reproducibility
        static void Seed(std::uint64_t seed){ Engine().seed(seed);}
        
        [[nodiscard]] static std::uint64_t Uint64(){ return Engine()(); }
        
        /// Returns a random uniform integer in the range [min, max]. Range is inclusive.
        template<std::integral T>
        [[nodiscard]] static T Range(T min, T max)
        {
            return std::uniform_int_distribution<T>{min, max}(Engine());
        }
        
        /// Returns a random uniform floating point number in the range [min, max]. Range is inclusive.
        template<std::floating_point T>
        [[nodiscard]] static T Range(T min, T max)
        {
            return std::uniform_real_distribution<T>{min, max}(Engine());
        }
        
        /// True with the given probability (0.0 to 1.0)
        [[nodiscard]] static bool Chance(double probability)
        {
            return std::bernoulli_distribution{probability}(Engine());
        }
    
    private:
        /// one engine enabled per thread, created and seeded on first use
        static std::mt19937_64& Engine()
        {
            thread_local std::mt19937_64 engine{std::random_device{}()};
            return engine;
        }
    };
}