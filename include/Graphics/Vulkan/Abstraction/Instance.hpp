#pragma once

#include <cstdint>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

namespace Droplet::Graphics::Vulkan
{
    /// Vulkan::Instance creation config struct
    struct InstanceConfig
    {
#ifdef NDEBUG
        static constexpr bool DEFAULT_ENABLE_VALIDATION{false}; ///< release builds (NDEBUG) run w/o validation
#else
        static constexpr bool DEFAULT_ENABLE_VALIDATION{true}; ///< debug builds want validation
#endif

        std::vector<const char*> validationLayers{"VK_LAYER_KHRONOS_validation"}; ///< validation layers are enabled based on enableValidation
        std::vector<const char*> platformExtensions{};
        bool enableValidation = DEFAULT_ENABLE_VALIDATION; ///< sets validationLayers on Instance creation
        std::uint32_t minimumApiVersion{vk::ApiVersion13}; ///< lowest acceptable API version
        std::uint32_t apiVersion{vk::ApiVersion13}; ///< target api version
        const char* appName{"Vulkan App"};  ///< name of the application as visible to vulkan
        const char* engineName{"VulkanEngine"}; ///< name of the engine as visible to vulkan
    };

    /// Vulkan instance abstraction + debug messenger when validation is enabled
    class Instance
    {
    public:
        explicit Instance(const InstanceConfig& cfg);
        ~Instance() = default;

        // immutable
        Instance(const Instance&) = delete;
        Instance& operator=(const Instance&) = delete;
        Instance(Instance&&) = delete;
        Instance& operator=(Instance&&) = delete;

        /// const reference to internal vk::raii::instance
        [[nodiscard]] const vk::raii::Instance& Get() const { return m_instance; }
        /// returns true if validation was enabled on instance creation
        [[nodiscard]] bool ValidationEnabled() const noexcept { return m_validationEnabled; }

    private:
        /**
         * Creates m_instance
         * @param cfg valid config
         * @throws std::runtime_error see constructor
         */
        void CreateInstance(const InstanceConfig& cfg);

        /// Sets up the debug messenger to report on validation layers
        void SetupDebugMessenger();

    private:
        bool m_validationEnabled{false}; ///< whether to use validation layers

        // dependency order top to bottom
        vk::raii::Context m_context; ///< stored context
        vk::raii::Instance m_instance{nullptr}; ///< stored instance
        vk::raii::DebugUtilsMessengerEXT m_debugMessenger{nullptr}; ///< debug messenger instance
    };
}
