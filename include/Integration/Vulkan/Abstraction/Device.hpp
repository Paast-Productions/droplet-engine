#pragma once

#include <cstdint>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include <algorithm>
#include <array>
#include <utility>

namespace Vulkan
{
    class Instance;
    class Surface;

    /// @brief The device queues by role (same order as RenderGraph::Queue)
    enum class QueueKind
    {
        Graphics,
        Compute,
        Transfer,
        Count
    };

    /// @return name of QueueKind enum field as string
    inline constexpr const char* ToString(QueueKind queueIndex)
    {
        switch (queueIndex)
        {
        case QueueKind::Graphics: return "Graphics";
        case QueueKind::Compute: return "Compute";
        case QueueKind::Transfer: return "Transfer";
        case QueueKind::Count: return "Count";
        default: return "unknown";
        }
    }

    /// @returns value as std::size_t index
    inline constexpr std::size_t ToIndex(QueueKind queueIndex) noexcept
    {
        return static_cast<std::size_t>(std::to_underlying(queueIndex));
    };
    inline constexpr std::size_t QUEUE_INDEX_COUNT = ToIndex(QueueKind::Count);
    ///< Total number of indices in the Queue array

    /// @brief Device's queues by role.
    /// @details Graphics queue always exists. Compute and transfer exists if the GPU supports giving them their own
    /// family, otherwise all work goes through the graphics queue.
    struct QueueInfo
    {
        std::array<vk::Queue, QUEUE_INDEX_COUNT> queues{nullptr};
        std::array<uint32_t, QUEUE_INDEX_COUNT> familyIndices;
        std::array<uint32_t, QUEUE_INDEX_COUNT> counts{};
        uint32_t timestampValidBits = 0;

        /// Constructor defaults to assuming graphics-queue only
        QueueInfo()
        {
            familyIndices.fill(vk::QueueFamilyIgnored);
        }

        /// @returns true if the device has set a queue for queueIndex
        [[nodiscard]] bool Has(QueueKind queueIndex) const noexcept
        {
            return familyIndices[ToIndex(queueIndex)] != vk::QueueFamilyIgnored;
        }

        /// @brief Get queue family for queue kind
        /// @returns the queue kind if device has that queue kind, else graphics
        [[nodiscard]] uint32_t FamilyOrGraphics(QueueKind kind) const noexcept
        {
            return Has(kind)
                       ? familyIndices[ToIndex(kind)]
                       : familyIndices[ToIndex(QueueKind::Graphics)];
        }
        
        /// @brief Distinct families in use, starting with graphics.
        /// @note An image used on several queues needs to be created VK_SHARING_MODE_CONCURRENT over them
        [[nodiscard]] std::vector<uint32_t> UniqueFamilies() const
        {
            std::vector<uint32_t> uniqueFamilies;
            for (const uint32_t familyIndex : familyIndices)
            {
                const bool alreadyListed = std::ranges::find(uniqueFamilies, familyIndex) != uniqueFamilies.end();
                if (familyIndex != vk::QueueFamilyIgnored && !alreadyListed)
                    uniqueFamilies.push_back(familyIndex);
            }
            return uniqueFamilies;
        }
    };

    /// Vulkan::Device creation config struct
    struct DeviceConfig
    {
        std::vector<const char*> requiredDeviceExtensions
        {
            vk::KHRSwapchainExtensionName,
            vk::KHRTimelineSemaphoreExtensionName
        }; ///< device extensions required by used, at least swapchain and timeline semaphores
        std::uint32_t minimumApiVersion{vk::ApiVersion13}; ///< api needs to be at least 1.3 for timeline semaphores
    };

    /// Physical device, logical device, the graphics/present queue and, where the GPU has a family of its own for
    /// them, a compute queue and a transfer queue
    class Device
    {
    public:
        Device(const Instance& instance, const Surface& surface, const DeviceConfig& config);
        ~Device() = default; // everything is raii so default destructor is fine

        // no move no copy
        Device(const Device&) = delete;
        Device& operator=(const Device&) = delete;
        Device(Device&&) = delete;
        Device& operator=(Device&&) = delete;

        /// @brief Physical Device for GPU selected on creation
        [[nodiscard]] const vk::raii::PhysicalDevice& GetPhysicalDevice() const noexcept { return m_physicalDevice; }

        /// @brief The logical device with which we interact with the GPU
        [[nodiscard]] const vk::raii::Device& Get() const noexcept { return m_device; }

        /// @brief The queue used for drawing, uploads and presenting
        [[nodiscard]] const vk::raii::Queue& GetGraphicsQueue() const noexcept { return m_graphicsQueue; }
        
        /// @brief Get pointer to queue of kind
        /// @returns Pointer to the queue kind or nullptr if not exists
        [[nodiscard]] const vk::raii::Queue* FindQueue(QueueKind kind) const noexcept;

        /// @brief Get struct containing handle, family and count of every queue
        [[nodiscard]] const QueueInfo& GetQueueInfo() const noexcept { return m_queueInfo; }

        /// @brief Highest sampler anisotropy the device was created for; 1 if the GPU has no anisotropic filtering
        [[nodiscard]] float GetMaxSamplerAnisotropy() const noexcept { return m_maxSamplerAnisotropy; }

        /// @brief Family index of GetGraphicsQueue()
        [[nodiscard]] std::uint32_t GetGraphicsPresentQueueFamily() const noexcept
        {
            return m_graphicsPresentQueueFamily;
        }

    private:
        /// @brief Sets m_physicalDevice to the best suitable GPU
        /// @details Selects a physical device by ranking them: Discrete > Integrated > Virtual > CPU > other.
        /// If there's a tie the first one wins.
        /// @throws std::runtime_error if no suitable GPU was found
        void SelectPhysicalDevice(
            const Instance& instance,
            const vk::raii::SurfaceKHR& surface,
            const DeviceConfig& config);

        /// @brief Creates the logical device and its queues on the chosen physical device, and fills m_queueInfo
        void CreateLogicalDevice(const vk::raii::SurfaceKHR& surface, const DeviceConfig& config);

    private:
            vk::raii::PhysicalDevice m_physicalDevice{nullptr}; ///< Interface to the GPU, selected by SelectPhysicalDevice
        std::uint32_t m_graphicsPresentQueueFamily{vk::QueueFamilyIgnored}; ///< Set by CreateLogicalDevice

        vk::raii::Device m_device{nullptr}; ///< Logical device
        vk::raii::Queue m_graphicsQueue{nullptr}; ///< main graphics queue
        vk::raii::Queue m_computeQueue{nullptr}; ///< null without a compute family that cannot draw
        vk::raii::Queue m_transferQueue{nullptr}; ///< null without a transfer-only family
        QueueInfo m_queueInfo{}; ///< Set by CreateLogicalDevice
        float m_maxSamplerAnisotropy{1.0f}; ///< Set by CreateLogicalDevice
    };
}
