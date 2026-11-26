/**
 * @file Device.cpp
 * @brief Device implementation: GPU selection, the logical device and its graphics/present, compute and
 *        transfer queues.
 */
#include "Graphics/Vulkan/Abstraction/Device.hpp"

#include <algorithm>
#include <bit>
#include <cstring>
#include <print>
#include <stdexcept>
#include <string>

#include "Graphics/Vulkan/Abstraction/Instance.hpp"
#include "Graphics/Vulkan/Abstraction/Surface.hpp"

namespace
{
    using namespace Droplet::Graphics::Vulkan;
    /**
     * @brief The first queue family of `candidate` that supports graphics and can present to `surface`.
     * @param candidate GPU to look at
     * @param surface   surface the family must be able to present to
     * @return the family index, or vk::QueueFamilyIgnored if there is none
     */
    std::uint32_t FindGraphicsPresentQueueFamily(
        const vk::raii::PhysicalDevice& candidate,
        const vk::raii::SurfaceKHR& surface)
    {
        const std::vector<vk::QueueFamilyProperties> queueFamilies = candidate.getQueueFamilyProperties();
        for (std::uint32_t familyIndex = 0; familyIndex < queueFamilies.size(); ++familyIndex)
        {
            const bool supportsGraphics =
                static_cast<bool>(queueFamilies[familyIndex].queueFlags & vk::QueueFlagBits::eGraphics);
            if (supportsGraphics && candidate.getSurfaceSupportKHR(familyIndex, *surface))
                return familyIndex;
        }
        return vk::QueueFamilyIgnored;
    }

    /**
     * @brief The family of `physicalDevice` that has all of `required`, none of `excluded` and the fewest other
     *        capabilities (the most specialised one); the first one wins a tie.
     * @return the family index, or vk::QueueFamilyIgnored if there is none
     */
    std::uint32_t FindDedicatedQueueFamily(
        const vk::raii::PhysicalDevice& physicalDevice,
        vk::QueueFlags required,
        vk::QueueFlags excluded)
    {
        const std::vector<vk::QueueFamilyProperties> queueFamilies = physicalDevice.getQueueFamilyProperties();
        std::uint32_t bestFamily = vk::QueueFamilyIgnored;
        int bestCapabilityCount = 0;
        for (std::uint32_t familyIndex = 0; familyIndex < queueFamilies.size(); ++familyIndex)
        {
            const vk::QueueFlags capabilities = queueFamilies[familyIndex].queueFlags;
            if (queueFamilies[familyIndex].queueCount == 0 || (capabilities & required) != required
                || (capabilities & excluded))
                continue;

            const int capabilityCount = std::popcount(static_cast<std::uint32_t>(capabilities));
            if (bestFamily == vk::QueueFamilyIgnored || capabilityCount < bestCapabilityCount)
            {
                bestFamily = familyIndex;
                bestCapabilityCount = capabilityCount;
            }
        }
        return bestFamily;
    }

    /// @brief "family N", or "none (graphics queue)" for vk::QueueFamilyIgnored.
    std::string DescribeQueueFamily(std::uint32_t familyIndex)
    {
        return familyIndex == vk::QueueFamilyIgnored
                   ? std::string("none (graphics queue)")
                   : "family " + std::to_string(familyIndex);
    }

    /**
     * @brief True if `name` is among a GPU's device extensions
     * @param availableExtensions the GPU's extensions (enumerateDeviceExtensionProperties)
     * @param name the extension's name
     */
    bool HasExtension(const std::vector<vk::ExtensionProperties>& availableExtensions, const char* name)
    {
        return std::ranges::any_of(availableExtensions,
            [name](const vk::ExtensionProperties& extension)
            {
                return std::strcmp(extension.extensionName, name) == 0;
            });
    }

    /**
     * @brief True if `candidate` meets everything listed in Device's class description.
     * @param candidate GPU to check
     * @param surface   surface a queue family must be able to present to
     * @param config    required extensions and API version
     */
    bool IsDeviceSuitable(
        const vk::raii::PhysicalDevice& candidate,
        const vk::raii::SurfaceKHR& surface,
        const DeviceConfig& config)
    {
        const vk::PhysicalDeviceProperties properties = candidate.getProperties();
        std::println("Checking GPU: {}", properties.deviceName.data());

        const bool supportsVulkanVersion = properties.apiVersion >= config.minimumApiVersion;

        const bool supportsGraphicsAndPresent =
            FindGraphicsPresentQueueFamily(candidate, surface) != vk::QueueFamilyIgnored;

        const auto availableDeviceExtensions = candidate.enumerateDeviceExtensionProperties();
        const bool supportsRequiredExtensions = std::ranges::all_of(
            config.requiredDeviceExtensions,
            [&availableDeviceExtensions](const char* requiredExtension)
            {
                return HasExtension(availableDeviceExtensions, requiredExtension);
            });

        const auto features = candidate.getFeatures2<
            vk::PhysicalDeviceFeatures2,
            vk::PhysicalDeviceVulkan11Features,
            vk::PhysicalDeviceVulkan12Features,
            vk::PhysicalDeviceVulkan13Features,
            vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

        // the texture array (set 2): an unsized array, entries left unwritten, grown by allocating a larger count
        const vk::PhysicalDeviceVulkan12Features& vulkan12 = features.get<vk::PhysicalDeviceVulkan12Features>();
        const bool supportsTextureArray = vulkan12.runtimeDescriptorArray && vulkan12.descriptorBindingPartiallyBound
            && vulkan12.descriptorBindingVariableDescriptorCount;
        const bool supportsBindless = vulkan12.descriptorBindingSampledImageUpdateAfterBind
            && vulkan12.descriptorBindingUpdateUnusedWhilePending;

        const bool supportsRequiredFeatures =
            features.get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters
            && supportsTextureArray && (!Droplet::Graphics::Vulkan::USE_BINDLESS || supportsBindless)
            && features.get<vk::PhysicalDeviceVulkan12Features>().timelineSemaphore
            && features.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering
            && features.get<vk::PhysicalDeviceVulkan13Features>().synchronization2
            && features.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

        // returns true if all criteria are supported
        return supportsVulkanVersion && supportsGraphicsAndPresent && supportsRequiredExtensions
            && supportsRequiredFeatures;
    }

    /**
     * @brief How much a GPU of `deviceType` is preferred: Discrete > Integrated > Virtual > CPU > other.
     * @return a higher value for a more preferred type
     */
    int DeviceTypeRank(vk::PhysicalDeviceType deviceType)
    {
        switch (deviceType)
        {
        case vk::PhysicalDeviceType::eDiscreteGpu: return 4;
        case vk::PhysicalDeviceType::eIntegratedGpu: return 3;
        case vk::PhysicalDeviceType::eVirtualGpu: return 2;
        case vk::PhysicalDeviceType::eCpu: return 1;
        default: return 0;
        }
    }
}

namespace Droplet::Graphics::Vulkan
{
    Device::Device(
        const Instance& instance,
        const Surface& surface,
        const DeviceConfig& config)
    {
        SelectPhysicalDevice(instance, surface.Get(), config);
        CreateLogicalDevice(surface.Get(), config);
    }

    void Device::SelectPhysicalDevice(
        const Instance& instance,
        const vk::raii::SurfaceKHR& surface,
        const DeviceConfig& config)
    {
        std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.Get().enumeratePhysicalDevices();

        // keep the best-ranked suitable GPU; on a tie the one enumerated first stays
        vk::raii::PhysicalDevice* bestDevice = nullptr;
        int bestRank = -1;
        for (vk::raii::PhysicalDevice& candidate : physicalDevices)
        {
            if (!IsDeviceSuitable(candidate, surface, config))
                continue;

            const int candidateRank = DeviceTypeRank(candidate.getProperties().deviceType);
            if (candidateRank > bestRank)
            {
                bestDevice = &candidate;
                bestRank = candidateRank;
            }
        }

        if (bestDevice == nullptr)
            throw std::runtime_error("Failed to find a GPU with the required features.");

        m_physicalDevice = std::move(*bestDevice);
        std::println("Selected GPU: {}", m_physicalDevice.getProperties().deviceName.data());
    }

    void Device::CreateLogicalDevice(const vk::raii::SurfaceKHR& surface, const DeviceConfig& config)
    {
        // IsDeviceSuitable has checked that there is one
        m_graphicsPresentQueueFamily = FindGraphicsPresentQueueFamily(m_physicalDevice, surface);
        if (m_graphicsPresentQueueFamily == vk::QueueFamilyIgnored)
            throw std::logic_error("Device: the selected GPU has no graphics/present queue family");

        // anisotropic filtering is optional: enabled if the GPU has it and set to the higher value
        const bool hasSamplerAnisotropy = m_physicalDevice.getFeatures().samplerAnisotropy == vk::True;
        m_maxSamplerAnisotropy = hasSamplerAnisotropy
                                     ? m_physicalDevice.getProperties().limits.maxSamplerAnisotropy
                                     : 1.0f;

        // wide lines are optional too (debug drawing uses them)
        const bool hasWideLines = m_physicalDevice.getFeatures().wideLines == vk::True;
        m_maxLineWidth = hasWideLines ? m_physicalDevice.getProperties().limits.lineWidthRange[1] : 1.0f;

        // geometry and tessellation stages are optional too: particles need the geometry stage
        const bool hasGeometryShader = m_physicalDevice.getFeatures().geometryShader == vk::True;
        const bool hasTessellationShader = m_physicalDevice.getFeatures().tessellationShader == vk::True;
        m_hasGeometryShader = hasGeometryShader;
        m_hasTessellationShader = hasTessellationShader;

        // features to enable, checked by IsDeviceSuitable
        const vk::StructureChain<
            vk::PhysicalDeviceFeatures2,
            vk::PhysicalDeviceVulkan11Features,
            vk::PhysicalDeviceVulkan12Features,
            vk::PhysicalDeviceVulkan13Features,
            vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain{
            {.features = {.geometryShader = hasGeometryShader ? vk::True : vk::False,
                          .tessellationShader = hasTessellationShader ? vk::True : vk::False,
                          .wideLines = hasWideLines ? vk::True : vk::False,
                          .samplerAnisotropy = hasSamplerAnisotropy ? vk::True : vk::False}}, // Features2
            {.shaderDrawParameters = vk::True}, // vk::PhysicalDeviceVulkan11Features
            {
                .descriptorBindingSampledImageUpdateAfterBind = Droplet::Graphics::Vulkan::USE_BINDLESS ? vk::True : vk::False,
                .descriptorBindingUpdateUnusedWhilePending = Droplet::Graphics::Vulkan::USE_BINDLESS ? vk::True : vk::False,
                .descriptorBindingPartiallyBound = vk::True,
                .descriptorBindingVariableDescriptorCount = vk::True,
                .runtimeDescriptorArray = vk::True,
                                .timelineSemaphore = vk::True
            }, // vk::PhysicalDeviceVulkan12Features
            {
                .synchronization2 = vk::True,
                .dynamicRendering = vk::True
            }, // vk::PhysicalDeviceVulkan13Features
            {.extendedDynamicState = vk::True} // vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
        };

        // A compute family that cannot draw and a transfer-only family, where the GPU has them: work submitted there
        // runs next to the graphics queue's. Video families are left alone. Without one, that work goes to the
        // graphics queue (FrameSubmitter).
        const std::uint32_t computeQueueFamily = FindDedicatedQueueFamily(
            m_physicalDevice, vk::QueueFlagBits::eCompute, vk::QueueFlagBits::eGraphics);
        const std::uint32_t transferQueueFamily = FindDedicatedQueueFamily(
            m_physicalDevice,
            vk::QueueFlagBits::eTransfer,
            vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute
            | vk::QueueFlagBits::eVideoDecodeKHR | vk::QueueFlagBits::eVideoEncodeKHR);

        // one queue in each family
        constexpr float queuePriority = 1.0f;
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        for (const std::uint32_t queueFamily : {m_graphicsPresentQueueFamily, computeQueueFamily, transferQueueFamily})
        {
            if (queueFamily == vk::QueueFamilyIgnored)
                continue;
            queueCreateInfos.push_back({
                .queueFamilyIndex = queueFamily,
                .queueCount = 1,
                .pQueuePriorities = &queuePriority
            });
        }
        
        // == Extensions ==
        // Look for calibrated timestamps for the sake of profiling timings
        std::vector<const char*> extensions = config.requiredDeviceExtensions;
        m_calibratedTimestamps = HasExtension(m_physicalDevice.enumerateDeviceExtensionProperties(),
                                                 VK_EXT_CALIBRATED_TIMESTAMPS_EXTENSION_NAME);
        if (m_calibratedTimestamps)
        {
            extensions.push_back(VK_EXT_CALIBRATED_TIMESTAMPS_EXTENSION_NAME);
        }
        

        const vk::DeviceCreateInfo deviceCreateInfo{
            .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
            .queueCreateInfoCount = static_cast<std::uint32_t>(queueCreateInfos.size()),
            .pQueueCreateInfos = queueCreateInfos.data(),
            .enabledExtensionCount = static_cast<std::uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data(),
        };

        m_device = vk::raii::Device{m_physicalDevice, deviceCreateInfo};

        // retrieve the first queue of each family
        m_graphicsQueue = vk::raii::Queue(m_device, m_graphicsPresentQueueFamily, 0);
        if (computeQueueFamily != vk::QueueFamilyIgnored)
            m_computeQueue = vk::raii::Queue(m_device, computeQueueFamily, 0);
        if (transferQueueFamily != vk::QueueFamilyIgnored)
            m_transferQueue = vk::raii::Queue(m_device, transferQueueFamily, 0);

        const auto setQueueInfo = [this](QueueKind kind, const vk::raii::Queue& queue, std::uint32_t family)
        {
            if (family == vk::QueueFamilyIgnored)
                return;
            m_queueInfo.queues[ToIndex(kind)] = *queue;
            m_queueInfo.familyIndices[ToIndex(kind)] = family;
            m_queueInfo.counts[ToIndex(kind)] = 1;
        };
        setQueueInfo(QueueKind::Graphics, m_graphicsQueue, m_graphicsPresentQueueFamily);
        setQueueInfo(QueueKind::Compute, m_computeQueue, computeQueueFamily);
        setQueueInfo(QueueKind::Transfer, m_transferQueue, transferQueueFamily);
        m_queueInfo.timestampValidBits =
            m_physicalDevice.getQueueFamilyProperties()[m_graphicsPresentQueueFamily].timestampValidBits;

        std::println("Queues: graphics family {}, compute {}, transfer {}",
                     m_graphicsPresentQueueFamily,
                     DescribeQueueFamily(computeQueueFamily),
                     DescribeQueueFamily(transferQueueFamily));
    }

    const vk::raii::Queue* Device::FindQueue(QueueKind kind) const noexcept
    {
        switch (kind)
        {
        case QueueKind::Graphics: return &m_graphicsQueue;
        case QueueKind::Compute: return m_queueInfo.Has(kind) ? &m_computeQueue : nullptr;
        case QueueKind::Transfer: return m_queueInfo.Has(kind) ? &m_transferQueue : nullptr;
        case QueueKind::Count: break;
        }
        return nullptr;
    }
}
