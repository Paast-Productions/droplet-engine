/**
 * @file Buffer.hpp
 * @brief Abstraction wrapper around Vulkan Buffer
 * @date 2026-11-02
 */
#pragma once

#include <vk_mem_alloc_raii.hpp>
#include <vulkan/vulkan_raii.hpp>

#include <Graphics/Vulkan/Helper/BufferCreateOptions.hpp>
#include <Resource/IDynamicResource.hpp>

namespace Droplet::Graphics::Vulkan 
{
    /// convenience wrapper around vk::Buffer
    class Buffer : public IDynamicResource
    {
    public:
        Buffer() = delete;
        
        Buffer(nullptr_t) {}
        
        explicit Buffer(const vma::raii::Allocator& p_allocator,
            const vk::raii::Device &p_device,
            const vk::BufferCreateInfo& p_createInfo,
            const vma::AllocationCreateInfo& p_allocationInfo = { .usage = vma::MemoryUsage::eAutoPreferDevice },
            const std::string &p_bufferName = "Unnamed Buffer");
        
        explicit Buffer(const vma::raii::Allocator& p_allocator,
            const vk::raii::Device &p_device,
            const BufferCreateOptions& p_options,
            const std::string &p_bufferName = "Unnamed Buffer");
        
        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;
        
        Buffer(Buffer&& other) noexcept : 
            m_buffer(std::move(other.m_buffer)),
            m_size(std::exchange(other.m_size, 0)),
            m_usage(std::exchange(other.m_usage, vk::BufferUsageFlags{})) {}

        Buffer& operator=(Buffer&& other) noexcept
        {
            if (this == &other)
                return *this;

            m_buffer.clear();
            m_buffer = std::move(other.m_buffer);

            m_size = std::exchange(other.m_size, 0);
            m_usage = std::exchange(other.m_usage, vk::BufferUsageFlags{});

            return *this;
        }
        
        ~Buffer() final = default;

        void HostWrite(const void* p_data, vk::DeviceSize p_size, vk::DeviceSize p_offset = 0) const;

        [[nodiscard]] bool IsHostVisible() const noexcept { return static_cast<bool>(m_buffer.getAllocation().getMemoryProperties() & vk::MemoryPropertyFlagBits::eHostVisible); }
        [[nodiscard]] bool IsDeviceLocal() const noexcept { return static_cast<bool>(m_buffer.getAllocation().getMemoryProperties() & vk::MemoryPropertyFlagBits::eDeviceLocal); }

        void Upload(const Buffer &p_stagingBuffer,
            const vk::raii::CommandBuffer &p_commandBuffer,
            const vk::DependencyInfo &p_dependencyInfo,
            const void* p_data,
            vk::DeviceSize p_size, 
            vk::DeviceSize p_offset = 0) const;

        [[nodiscard]] const vma::raii::Buffer &Get() const noexcept { return m_buffer; }
        [[nodiscard]] vk::DeviceSize GetSize() const noexcept { return m_size; }
        [[nodiscard]] vk::BufferUsageFlags GetUsage() const noexcept { return m_usage; }

        //explicit operator bool() const noexcept { return Get() != vma::raii::Buffer{}; }

    private:
        vma::raii::Buffer m_buffer { nullptr };
        vk::DeviceSize m_size {};
        vk::BufferUsageFlags m_usage {};
        
    };
}
