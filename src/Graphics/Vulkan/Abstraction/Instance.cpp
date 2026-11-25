#include "Graphics/Vulkan/Abstraction/Instance.hpp"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

namespace
{
    /// @brief Prints validation warnings and errors to stderr. Never aborts the call (returns VK_FALSE).
    VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
        vk::DebugUtilsMessageTypeFlagsEXT type,
        const vk::DebugUtilsMessengerCallbackDataEXT* callbackData,
        void*)
    {
        if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
            || severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
        {
            std::cerr << "validation layer: type " << vk::to_string(type)
                << " msg: " << callbackData->pMessage << '\n';
        }

        return vk::False;
    }

    /**
     * @brief The layers to enable: config.validationLayers with validation, none otherwise.
     * @param loaderContext loader to ask for the installed layers
     * @param config        layers and whether validation is enabled
     * @throws std::runtime_error if one of them is not installed
     */
    std::vector<const char*> GetRequiredLayers(
        const vk::raii::Context& loaderContext,
        const Droplet::Graphics::Vulkan::InstanceConfig& config)
    {
        std::vector<const char*> requiredLayers;
        if (config.enableValidation)
            requiredLayers.assign(config.validationLayers.begin(), config.validationLayers.end());

        // Check if the required layers are supported by the Vulkan implementation.
        const std::vector<vk::LayerProperties> layerProperties = loaderContext.enumerateInstanceLayerProperties();
        const auto unsupportedLayerIt = std::ranges::find_if(
            requiredLayers,
            [&layerProperties](const char* requiredLayer)
            {
                return std::ranges::none_of(
                    layerProperties,
                    [requiredLayer](const vk::LayerProperties& layerProperty)
                    {
                        return std::strcmp(layerProperty.layerName, requiredLayer) == 0;
                    });
            });

        if (unsupportedLayerIt != requiredLayers.end())
            throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayerIt));

        return requiredLayers;
    }
    
    /**
    * @brief The instance extensions to enable: the platform's, plus VK_EXT_debug_utils with validation.
    * @param loaderContext loader to ask for the supported extensions
    * @param config        platform extensions and whether validation is enabled
    * @throws std::runtime_error if one of them is not supported
    */
    std::vector<const char*> GetRequiredExtensions(
        const vk::raii::Context& loaderContext,
        const Droplet::Graphics::Vulkan::InstanceConfig& config)
    {
        // the window system's extensions (surface creation), handed in by Context
        std::vector<const char*> requiredExtensions = config.platformExtensions;
        if (config.enableValidation)
            requiredExtensions.push_back(vk::EXTDebugUtilsExtensionName);

        // See which extensions are available
        const std::vector<vk::ExtensionProperties> availableExtensions =
            loaderContext.enumerateInstanceExtensionProperties();

        // Make sure all required are available
        for (const char* required : requiredExtensions)
        {
            const bool supported = std::ranges::any_of(
                availableExtensions,
                [required](const vk::ExtensionProperties& extension)
                {
                    return std::strcmp(extension.extensionName, required) == 0;
                });

            if (!supported)
                throw std::runtime_error(
                    std::string("Required Vulkan instance extension is not supported: ") + required);
        }

        return requiredExtensions;
    }
}


using namespace Droplet::Graphics::Vulkan;

Instance::Instance(const InstanceConfig& config)
      : m_validationEnabled{config.enableValidation}
{
    CreateInstance(config);
    SetupDebugMessenger();
}

    void Instance::CreateInstance(const InstanceConfig& config)
    {
        // Required version for dynamic rendering
        if (m_context.enumerateInstanceVersion() < config.minimumApiVersion)
            throw std::runtime_error("Vulkan 1.3 or newer is required. Context reports older API version.");

        const vk::ApplicationInfo appInfo{
            .pApplicationName = config.appName,
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = config.engineName,
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = config.apiVersion
        };

        // No layers is valid (release builds); the platform always asks for at least VK_KHR_surface.
        const std::vector<const char*> enabledLayers = GetRequiredLayers(m_context, config);
        const std::vector<const char*> enabledExtensions = GetRequiredExtensions(m_context, config);

        const vk::InstanceCreateInfo createInfo{
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = static_cast<std::uint32_t>(enabledLayers.size()),
            .ppEnabledLayerNames = enabledLayers.data(),
            .enabledExtensionCount = static_cast<std::uint32_t>(enabledExtensions.size()),
            .ppEnabledExtensionNames = enabledExtensions.data()
        };

        m_instance = vk::raii::Instance(m_context, createInfo);
    }

    void Instance::SetupDebugMessenger()
    {
        if (!m_validationEnabled)
            return;

        constexpr vk::DebugUtilsMessageSeverityFlagsEXT severityFlags =
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
            | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
        constexpr vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags =
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
            | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
            | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;

        constexpr vk::DebugUtilsMessengerCreateInfoEXT messengerCreateInfo{
            .messageSeverity = severityFlags,
            .messageType = messageTypeFlags,
            .pfnUserCallback = &DebugCallback
        };
        m_debugMessenger = m_instance.createDebugUtilsMessengerEXT(messengerCreateInfo);
    }
