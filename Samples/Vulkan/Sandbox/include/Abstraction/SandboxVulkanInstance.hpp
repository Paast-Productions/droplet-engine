#pragma once

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "SandboxSDLVulkan.hpp"

namespace Droplet::Sandbox::Vulkan
{
    struct VulkanInstanceConfig
    {
        std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
        std::uint32_t minimumApiVersion = vk::ApiVersion13;
        std::uint32_t apiVersion = vk::ApiVersion13;
        const char* appName{nullptr};
        const char* engineName{nullptr};
    };

    class VulkanInstance
    {
    public:
        VulkanInstance(const VulkanInstanceConfig& cfg)
        {
            CreateInstance(cfg);
        }

        ~VulkanInstance() = default;
        
        vk::raii::Instance &Get(){return m_instance;}
        const vk::raii::Instance &Get()const{return m_instance;}

    private:
        void CreateInstance(const VulkanInstanceConfig& cfg);

        bool SupportsMinimumApiVersion(std::uint32_t version) const
        {
            return version <= m_context.enumerateInstanceVersion();
        }

        std::vector<const char*> GetRequiredLayers(std::vector<const char*> validationLayers) const;
        std::vector<const char*> GetRequiredExtensions() const;

    private:
        vk::raii::Context m_context;
        vk::raii::Instance m_instance{nullptr};

#ifndef NDEBUG
        vk::raii::DebugUtilsMessengerEXT m_debugMessenger{nullptr};
        static constexpr bool ENABLE_VALIDATION_LAYERS = true;
#else
        static constexpr bool ENABLE_VALIDATION_LAYERS = false;
#endif
    };
    
    inline std::vector<const char*> VulkanInstance::GetRequiredExtensions() const
    {
        // Ask SDL for required Vulkan extensions
        std::vector<const char*> requiredExtensions = SandboxSDLVulkan::GetRequiredInstanceExtensions();
        if (ENABLE_VALIDATION_LAYERS)
        {
            requiredExtensions.push_back(vk::EXTDebugUtilsExtensionName);
        }

        // See which extensions are available
        const auto availableExtensions = m_context.enumerateInstanceExtensionProperties();

        // Make sure all required are available
        for (const char* required : requiredExtensions)
        {
            const bool supported = std::ranges::any_of(availableExtensions,
                                                       [required](
                                                       const vk::ExtensionProperties& extension)
                                                       {
                                                           return std::strcmp(
                                                               extension.extensionName,
                                                               required) == 0;
                                                       });

            if (!supported)
            {
                throw std::runtime_error(
                    std::string("Required Vulkan instance extension "
                        "is not supported: ") + required);
            }
        }

        return requiredExtensions;
    }

    inline std::vector<const char*> VulkanInstance::GetRequiredLayers(std::vector<const char*> validationLayers) const
    {
        std::vector<const char*> requiredLayers;
        if constexpr (ENABLE_VALIDATION_LAYERS)
        {
            requiredLayers.assign(validationLayers.begin(), validationLayers.end());
        }

        // Check if the required layers are supported by the Vulkan implementation.
        auto layerProperties = m_context.enumerateInstanceLayerProperties();
        auto unsupportedLayerIt = std::ranges::find_if(requiredLayers,
                                                       [&layerProperties](auto const& requiredLayer)
                                                       {
                                                           return std::ranges::none_of(layerProperties,
                                                               [requiredLayer](auto const& layerProperty)
                                                               {
                                                                   return strcmp(layerProperty.layerName, requiredLayer)
                                                                       ==
                                                                       0;
                                                               });
                                                       });
        if (unsupportedLayerIt != requiredLayers.end())
        {
            throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayerIt));
        }

        return requiredLayers;
    }

    inline void VulkanInstance::CreateInstance(const VulkanInstanceConfig& cfg)
    {
        if (!SupportsMinimumApiVersion(cfg.minimumApiVersion))
            throw std::runtime_error("Vulkan Api version not supported.");

        const vk::ApplicationInfo appInfo
        {
            .pApplicationName = cfg.appName,
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = cfg.engineName,
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = cfg.apiVersion
        };

        auto enabledLayers = GetRequiredLayers(cfg.validationLayers);
        auto enabledExtensions = GetRequiredExtensions();

        if (enabledLayers.empty() || enabledExtensions.empty())
        {
            throw std::runtime_error("Layers or extensions was empty when creating vulkan instance");
        }

        const vk::InstanceCreateInfo createInfo{
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = static_cast<std::uint32_t>(enabledLayers.size()),
            .ppEnabledLayerNames = enabledLayers.data(),
            .enabledExtensionCount = static_cast<std::uint32_t>(enabledExtensions.size()),
            .ppEnabledExtensionNames = enabledExtensions.data()
        };

        m_instance = vk::raii::Instance(m_context, createInfo);
    }
}
