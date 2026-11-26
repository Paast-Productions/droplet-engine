/**
 * @file Context.cpp
 * @brief Vulkan::Context implementation: instance, surface, device and allocator, in that order.
 */
#include "Graphics/Vulkan/Core/Context.hpp"

#include <exception>
#include <print>
#include <vector>

namespace
{
    /**
     * @brief `instanceConfig` with the window system's instance extensions added.
     * @param surfaceProvider the window system
     * @param instanceConfig  configuration from ContextCreateInfo
     */
    Droplet::Graphics::Vulkan::InstanceConfig WithPlatformExtensions(
        const Droplet::Graphics::Vulkan::ISurfaceProvider& surfaceProvider,
        Droplet::Graphics::Vulkan::InstanceConfig instanceConfig)
    {
        const std::vector<const char*> platformExtensions = surfaceProvider.GetRequiredInstanceExtensions();
        instanceConfig.platformExtensions.insert(
            instanceConfig.platformExtensions.end(),
            platformExtensions.begin(),
            platformExtensions.end());
        return instanceConfig;
    }
}

namespace Droplet::Graphics::Vulkan
{
    Context::Context(const ISurfaceProvider& surfaceProvider, const ContextCreateInfo& createInfo)
        : m_surfaceProvider{&surfaceProvider},
          m_instance{WithPlatformExtensions(surfaceProvider, createInfo.instance)},
          m_surface{surfaceProvider, m_instance},
          m_device{m_instance, m_surface, createInfo.device}
    {
        CreateAllocator();
    }

    Context::~Context() noexcept
    {
        try
        {
            WaitIdle();
        }
        catch (const std::exception& exception)
        {
            std::println("Vulkan device wait during shutdown failed: {}", exception.what());
        }
        catch (...)
        {
            std::println("Vulkan device wait during shutdown failed.");
        }
    }

    void Context::WaitIdle() const
    {
        m_device.Get().waitIdle();
    }

    void Context::CreateAllocator()
    {
        const vma::AllocatorCreateInfo createInfo{
            .flags = {},
            .physicalDevice = *m_device.GetPhysicalDevice()
        };

        m_allocator = vma::raii::Allocator{
            m_instance.Get(),
            m_device.Get(),
            createInfo
        };
    }
}
