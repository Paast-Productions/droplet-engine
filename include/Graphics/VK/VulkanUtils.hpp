#pragma once

#include <vulkan/vulkan.hpp>
#include <stdexcept>
#include <string_view>
#include <string>

namespace Vulkan::Utils
{
    /**
     * Throws a runtime error with the given reason and vk::Result as string
     * @param result vk::Result code
     * @param reason string with reason for failure
     * @throws std::runtime_error with reason + result as string
     */
    [[noreturn]] inline
    void ThrowResult(vk::Result result, std::string_view reason)
    {
        throw std::runtime_error(std::string(reason) + "\n  Result: " + vk::to_string(result));
    }

    /**
     * Helper function to throw if vk::Result is not one of the expected results
     * @tparam ExpectedResults acceptable vk::Result values.
     * @param result result to check
     * @param reason reason to throw if the result is not valid
     * @param expectedResults acceptable results, if left empty will check for vk::Result::eSuccess
     */
    template <typename... ExpectedResults>
        requires (std::same_as<ExpectedResults, vk::Result> && ...)
    void CheckResult(
        vk::Result result,
        std::string_view reason,
        ExpectedResults... expectedResults)
    {
        // if no expected results, only allow success
        if constexpr (sizeof...(expectedResults) == 0)
        {
            if (result == vk::Result::eSuccess)
                return;
        }
        else if (((result == expectedResults) || ...))
        {
            return;
        };

        ThrowResult(result, reason);
    }
}
