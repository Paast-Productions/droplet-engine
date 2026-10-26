#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace Droplet::Sandbox::Vulkan
{
    struct VulkanDeviceConfig
    {
        std::vector<const char*> requireDeviceExtensions = { vk::KHRSwapchainExtensionName };
    };
    
    class VulkanDevice
    {
    public:
        VulkanDevice(const VulkanDeviceConfig& cfg)
        {
            SelectPhysicalDevice();
            CreateLogicalDevice();
        }
    private:
        bool IsDeviceSuitable(const vk::raii::PhysicalDevice& candidate);

        void SelectPhysicalDevice();
        void CreateLogicalDevice();

    private:
        vk::raii::PhysicalDevice m_physicalDevice{nullptr};
        vk::raii::Device m_device{nullptr};

        vk::raii::Queue m_graphicsQueue{nullptr};
        vk::raii::Queue m_presentQueue{nullptr};

        uint32_t m_graphicsQueueFamily = 0;
        uint32_t m_presentQueueFamily = 0;
    };

    inline bool VulkanDevice::IsDeviceSuitable(const vk::raii::PhysicalDevice& candidate)
    {
        const vk::PhysicalDeviceProperties properties = candidate.getProperties();
    const char* deviceName = properties.deviceName.data();

    std::println("Checking GPU: {}", deviceName);

    bool supportsVulkanVersion = properties.apiVersion >= MINIMUM_API_VERSION;

    auto queueFamilies = candidate.getQueueFamilyProperties();
    bool supportsGraphics = std::ranges::any_of(queueFamilies, [](auto const& qfp)
    {
        return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics);
    });

    auto availableDeviceExtensions = candidate.enumerateDeviceExtensionProperties();
    bool supportsRequiredExtensions =
        std::ranges::all_of(m_requiredDeviceExtensions,
                            [&availableDeviceExtensions](auto const& requiredExtension)
                            {
                                return std::ranges::any_of(
                                    availableDeviceExtensions,
                                    [requiredExtension](auto const& availableDeviceExtension)
                                    {
                                        return std::strcmp(availableDeviceExtension.extensionName, requiredExtension) ==
                                            0;
                                    });
                            });

    auto features = candidate.template getFeatures2<vk::PhysicalDeviceFeatures2,
                                                    vk::PhysicalDeviceVulkan11Features,
                                                    vk::PhysicalDeviceVulkan13Features,
                                                    vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

    bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
        features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
        features.template get<vk::PhysicalDeviceVulkan13Features>().synchronization2 &&
        features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

    // returns true if all criteria are supported
    return supportsGraphics && supportsVulkanVersion && supportsRequiredExtensions && supportsRequiredFeatures;
    }

    inline void VulkanDevice::SelectPhysicalDevice()
    {
        std::vector<vk::raii::PhysicalDevice> physicalDevices = m_instance.enumeratePhysicalDevices();
        auto const devIter = std::ranges::find_if(
            physicalDevices.begin(), physicalDevices.end(),
            [this](auto const& device)
            {
                return IsDeviceSuitable(device);
            });

        if (devIter == physicalDevices.end())
            throw std::runtime_error("Failed to find a GPU with the required features.");

        m_physicalDevice = std::move(*devIter);
    }

    inline void VulkanDevice::CreateLogicalDevice()
    {
     // pick first available queue family
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = m_physicalDevice.getQueueFamilyProperties();

    auto graphicsQueueFamilyProperty = std::ranges::find_if(queueFamilyProperties, [](auto const& qfp)
    {
        return (qfp.queueFlags & vk::QueueFlagBits::eGraphics)
            != static_cast<vk::QueueFlags>(0);
    });
    assert(graphicsQueueFamilyProperty != queueFamilyProperties.end() && "No graphics queue family found");

    m_graphicsPresentQueueFamily = static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(),
                                                                       graphicsQueueFamilyProperty));

    // query for 1.3 features
    vk::StructureChain<vk::PhysicalDeviceFeatures2,
                       vk::PhysicalDeviceVulkan11Features,
                       vk::PhysicalDeviceVulkan13Features,
                       vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
        featureChain = {
            {}, // vk::PhysicalDeviceFeatures2
            {.shaderDrawParameters = vk::True}, // vk::PhysicalDeviceVulkan11Features
            {
                .synchronization2 = vk::True,
                .dynamicRendering = vk::True
            }, // vk::PhysicalDeviceVulkan13Features
            {.extendedDynamicState = vk::True} // vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
        };

    constexpr float queuePriority = 1.0f;

    vk::DeviceQueueCreateInfo queueCreateInfo
    {
        .queueFamilyIndex = m_graphicsPresentQueueFamily,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority
    };

    vk::DeviceCreateInfo deviceCreateInfo
    {
        .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCreateInfo,
        .enabledExtensionCount = static_cast<std::uint32_t>(m_requiredDeviceExtensions.size()),
        .ppEnabledExtensionNames = m_requiredDeviceExtensions.data(),
    };

    m_device = vk::raii::Device{m_physicalDevice, deviceCreateInfo};

    // retrieve first queue from device queue family
    m_graphicsQueue = vk::raii::Queue(m_device, m_graphicsPresentQueueFamily, 0);
    }
}
