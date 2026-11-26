/**
 * @file Context.hpp
 * @brief Vulkan::Context: everything that lives as long as the device (instance, surface, device, queue, allocator).
 */
#pragma once

#include <cstdint>

#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc_raii.hpp>

#include "Graphics/Vulkan/Core/SurfaceProvider.hpp"
#include "Graphics/Vulkan/Abstraction/Device.hpp"
#include "Graphics/Vulkan/Abstraction/Instance.hpp"
#include "Graphics/Vulkan/Abstraction/Surface.hpp"

namespace Droplet::Graphics::Vulkan
{
    /// @brief What Context is created with
    struct ContextCreateInfo
    {
        InstanceConfig instance{}; ///< Layers, versions and names for the instance.
        DeviceConfig device{};     ///< Required device extensions and API version.
    };

    /**
     * @brief Owns the instance, the window surface, the device with its queues, and the VMA allocator.
     *
     * @details Created first and destroyed last. Everything else on the GPU side (swapchain, frame sync, compositor, 
     * resource manager, transient pool, passes) is created from it and keeps a pointer to it.
     * Not copyable or movable: they hold pointers into it.
     */
    class Context
    {
    public:
        /**
         * @brief Creates the instance, surface, device and allocator, in that order.
         * @param surface window system providing a surface; must outlive the Context
         * @param createInfo instance and device configuration
         * @throws std::runtime_error if Vulkan 1.3, a required layer or extension, or a suitable GPU is missing, or the
         *         surface cannot be created
         */
        explicit Context(const ISurfaceProvider &surface, const ContextCreateInfo& createInfo);

        /// @brief Waits for the device to go idle, so nothing it owns is destroyed while in use.
        ~Context() noexcept;

        // No-copy no-move
        Context(const Context&) = delete;
        Context& operator=(const Context&) = delete;
        Context(Context&&) = delete;
        Context& operator=(Context&&) = delete;

        /// @brief The RAII instance.
        [[nodiscard]] const vk::raii::Instance& GetInstance() const noexcept { return m_instance.Get(); }

        /// @brief The window surface.
        [[nodiscard]] const vk::raii::SurfaceKHR& GetSurface() const noexcept { return m_surface.Get(); }

        /// @brief The window system the surface was created from (framebuffer size for the swapchain).
        [[nodiscard]] const ISurfaceProvider& GetSurfaceProvider() const noexcept { return *m_surfaceProvider; }

        /// @brief The selected GPU.
        [[nodiscard]] const vk::raii::PhysicalDevice& GetPhysicalDevice() const noexcept
        {
            return m_device.GetPhysicalDevice();
        }

        /// @brief The logical device.
        [[nodiscard]] const vk::raii::Device& GetDevice() const noexcept { return m_device.Get(); }

        /// @brief The queue used for drawing, uploads and presenting.
        [[nodiscard]] const vk::raii::Queue& GetGraphicsQueue() const noexcept { return m_device.GetGraphicsQueue(); }

        /// @brief The device's own queue for `queueIndex`, or nullptr if it has none.
        [[nodiscard]] const vk::raii::Queue* FindQueue(QueueKind queueIndex) const noexcept
        {
            return m_device.FindQueue(queueIndex);
        }

        /// @brief Handle, family and count of every queue; a missing queue's family is vk::QueueFamilyIgnored.
        [[nodiscard]] const QueueInfo& GetQueueInfo() const noexcept { return m_device.GetQueueInfo(); }

        /// @brief Family index of GetGraphicsQueue().
        [[nodiscard]] std::uint32_t GetGraphicsPresentQueueFamily() const noexcept
        {
            return m_device.GetGraphicsPresentQueueFamily();
        }

        /// @brief Highest sampler anisotropy the device supports; 1 if none.
        [[nodiscard]] float GetMaxSamplerAnisotropy() const noexcept { return m_device.GetMaxSamplerAnisotropy(); }

        /// @brief Widest line the device supports; 1 if none.
        [[nodiscard]] float GetMaxLineWidth() const noexcept { return m_device.GetMaxLineWidth(); }
        [[nodiscard]] bool HasGeometryShader() const noexcept { return m_device.HasGeometryShader(); }
        [[nodiscard]] bool HasTessellationShader() const noexcept { return m_device.HasTessellationShader(); }

        /// @brief The VMA allocator.
        [[nodiscard]] const vma::raii::Allocator& GetAllocator() const noexcept { return m_allocator; }

        /// @brief Blocks until the device has finished all submitted work
        void WaitIdle() const;
        
        /// @brief Convenience operator to get surface render capabilities
        auto GetSurfaceCapabilities() const
        {
            return GetPhysicalDevice().getSurfaceCapabilitiesKHR(*GetSurface());
        }
        /// @brief Convenience operator to get surface format capabilities
        auto GetSurfaceFormats()const
        {
            return GetPhysicalDevice().getSurfaceFormatsKHR(*GetSurface());
        }
        
        /// @brief Convenience operator to get surface format capabilities
        auto GetSurfacePresentModes()const
        {
            return GetPhysicalDevice().getSurfacePresentModesKHR(*GetSurface());
        }


    private:
        /// @brief Creates m_allocator for the device.
        void CreateAllocator();

    private:
        const ISurfaceProvider* m_surfaceProvider; ///< Not owned. outlives the Context

        // dependency order top to bottom
        Instance m_instance;                        ///< Instance and debug messenger.
        Surface m_surface;                          ///< Window surface.
        Device m_device;                            ///< GPU, logical device and queues.
        vma::raii::Allocator m_allocator{nullptr};  ///< Device memory allocator.
    };
}
