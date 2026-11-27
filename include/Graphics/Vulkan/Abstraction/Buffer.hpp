/**
 * @file Buffer.hpp
 * @brief Abstraction wrapper around Vulkan Buffer
 * @date 2026-11-02
 */
#pragma once

#include <stdexcept>
#include <utility>

#include <vk_mem_alloc_raii.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace Droplet::Graphics::Vulkan 
{
    enum class MemoryAccess
    {
        GpuOnly,   // written once via staging, read many times (static meshes)
        Dynamic,   // CPU rewrites every frame, GPU reads ~once (UBOs, streamed vertices)
        Upload,    // CPU writes occasionally. Device-local, mapped if possible, else staged
        Staging,   // transfer source in system RAM
        Readback,  // GPU writes, CPU reads
    };

    inline vma::AllocationCreateInfo MakeAllocationInfo(MemoryAccess access)
    {
        using FlagBits = vma::AllocationCreateFlagBits;
        switch (access)
        {
        case MemoryAccess::GpuOnly:
            return { .usage = vma::MemoryUsage::eAutoPreferDevice };
        case MemoryAccess::Dynamic:   // picks ReBAR if present, else host RAM
            return { .flags = FlagBits::eHostAccessSequentialWrite | FlagBits::eMapped,
                     .usage = vma::MemoryUsage::eAutoPreferDevice };
        case MemoryAccess::Upload:    // device-local, mappable only if that's cheap
            return { .flags = FlagBits::eHostAccessSequentialWrite | FlagBits::eHostAccessAllowTransferInstead | FlagBits::eMapped,
                     .usage = vma::MemoryUsage::eAutoPreferDevice };
        case MemoryAccess::Staging:
            return { .flags = FlagBits::eHostAccessSequentialWrite | FlagBits::eMapped,
                     .usage = vma::MemoryUsage::eAutoPreferHost };
        case MemoryAccess::Readback:
            return { .flags = FlagBits::eHostAccessRandom | FlagBits::eMapped,
                     .usage = vma::MemoryUsage::eAutoPreferHost };
        default:
            return {};
        }
    }




    /// Options struct for buffer creation
    struct BufferCreateOptions
    {
        vk::DeviceSize size{ 0 };
        vk::BufferUsageFlags usage{};
        vk::BufferCreateFlags flags{};
        vma::AllocationCreateInfo allocationInfo = MakeAllocationInfo(MemoryAccess::GpuOnly);

        static BufferCreateOptions For(vk::DeviceSize size, vk::BufferUsageFlags usage, MemoryAccess access)
        {
            if (access == MemoryAccess::GpuOnly || access == MemoryAccess::Upload)
            {
                usage |= vk::BufferUsageFlagBits::eTransferDst;
            }
            return {
                .size = size,
                .usage = usage,
                .allocationInfo = MakeAllocationInfo(access)
            };
        }
        /**
         * @brief Returns a BufferCreateOptions struct to make a vertex buffer with Buffer::Create
         * @param size size of the buffer
         * @param access How the buffer will be handled with memory
         * @return
         */
        static BufferCreateOptions Vertex(
            vk::DeviceSize size,
            MemoryAccess access = MemoryAccess::GpuOnly,
            vk::BufferUsageFlags extra = {})
        {
            return For(size, vk::BufferUsageFlagBits::eVertexBuffer | extra, access);
        }

        /**
         * @brief Returns a BufferCreateOptions struct to make an index buffer with Buffer::Create
         * @param size size of the buffer
         * @param usage any additional flags.
         * @return
         */
        static BufferCreateOptions Index(
            vk::DeviceSize size,
            MemoryAccess access = MemoryAccess::GpuOnly,
            vk::BufferUsageFlags extra = {})
        {
            return For(size, vk::BufferUsageFlagBits::eIndexBuffer | extra, access);
        }

        /**
         * @brief Returns a BufferCreateOptions struct to make a uniform buffer with Buffer::Create
         * @param size size of the buffer
         * @param usage any additional flags.
         * @return
         */
        static BufferCreateOptions Uniform(
            vk::DeviceSize size,
            MemoryAccess access = MemoryAccess::Dynamic,
            vk::BufferUsageFlags extra = {})
        {
            return For(size, vk::BufferUsageFlagBits::eUniformBuffer | extra, access);
        }

        /**
         * @brief Staging buffer
         * @param size size of the device memory
         * @return
         */
        static BufferCreateOptions Staging(vk::DeviceSize size)
        {
            return For(size, vk::BufferUsageFlagBits::eTransferSrc, MemoryAccess::Staging);
        }
    };


    /// convenience wrapper around vk::Buffer
    class Buffer
    {
    public:
        Buffer() = default;
        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;

        Buffer(Buffer&& other) noexcept
            : m_buffer(std::move(other.m_buffer)),
            m_size(std::exchange(other.m_size, 0)),
            m_usage(std::exchange(other.m_usage, vk::BufferUsageFlags{})),
            m_memoryPropertyFlags(std::exchange(other.m_memoryPropertyFlags, vk::MemoryPropertyFlags{}))
        {
        }

        Buffer& operator=(Buffer&& other) noexcept
        {
            if (this == &other)
                return *this;

            m_buffer.clear();
            m_buffer = std::move(other.m_buffer);

            m_size = std::exchange(other.m_size, 0);
            m_usage = std::exchange(other.m_usage, vk::BufferUsageFlags{});
            m_memoryPropertyFlags = std::exchange(other.m_memoryPropertyFlags, vk::MemoryPropertyFlags{});

            return *this;
        }

        // ========================
        // FACTORY FUNCTIONS
        // ========================


        /// factory function
        static Buffer Create(
            const vma::raii::Allocator& allocator,
            const vk::BufferCreateInfo& createInfo,
            const vma::AllocationCreateInfo& allocationInfo =
            {
                .usage = vma::MemoryUsage::eAutoPreferDevice
            })
        {
            if (createInfo.size == 0 || !createInfo.usage)
                throw std::invalid_argument("Invalid buffer create info");

            vma::raii::Buffer buffer = allocator.createBuffer(createInfo, allocationInfo);

            return Buffer{
                std::move(buffer),
                createInfo
            };
        }

        // convenience factory for exclusive sharing buffers
        static Buffer Create(
            const vma::raii::Allocator& allocator,
            const BufferCreateOptions& options)
        {
            if (options.size == 0 || !options.usage)
                throw std::invalid_argument("Invalid buffer create options");

            vk::BufferCreateInfo info{};
            info.flags = options.flags;
            info.size = options.size;
            info.usage = options.usage;
            info.sharingMode = vk::SharingMode::eExclusive;

            return Create(allocator, info, options.allocationInfo);
        }

        void* Map() const
        {
            return m_buffer.getAllocation().map();
        }

        void Unmap() const { m_buffer.getAllocation().unmap(); }

        void Flush(vk::DeviceSize offset, vk::DeviceSize size) const
        {
            m_buffer.getAllocation().flush(offset, size);
        }

        void Invalidate(vk::DeviceSize offset, vk::DeviceSize size) const
        {
            m_buffer.getAllocation().invalidate(offset, size);
        }

        void Write(const void* data, vk::DeviceSize size, vk::DeviceSize offset = 0) const
        {
            if (size == 0)
                return;

            if (!data)
                throw std::invalid_argument("Cannot write data from a nullptr");

            if (offset > m_size || size > m_size - offset)
                throw std::out_of_range("Buffer write exceeds buffer size");

            if (!IsHostVisible())
                throw std::runtime_error("Cannot write to buffer not visible to host");

            m_buffer.getAllocation().copyFromMemory(data, offset, size);
        }

        bool IsHostVisible()const noexcept { return static_cast<bool>(m_memoryPropertyFlags & vk::MemoryPropertyFlagBits::eHostVisible); }
        bool IsDeviceLocal() const noexcept { return static_cast<bool>(m_memoryPropertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal); }

        [[nodiscard]] Buffer Upload(
            const vma::raii::Allocator& allocator,
            vk::CommandBuffer command,
            const void* data,
            vk::DeviceSize size,
            vk::DeviceSize offset = 0) const
        {
            if (IsHostVisible())
            {
                Write(data, size, offset);
                return {};
            }

            if (!(m_usage & vk::BufferUsageFlagBits::eTransferDst))
            {
                throw std::logic_error("Buffer is not host-visible and lacks eTransferDst");
            }
            if (offset > m_size || size > m_size - offset)
                throw std::out_of_range("Buffer upload exceeds buffer size");

            Buffer staging = Create(allocator, BufferCreateOptions::Staging(size));
            staging.Write(data, size);
            command.copyBuffer(staging.Get(), Get(), vk::BufferCopy{ 0, offset, size });
            return staging;
        }


        vk::Buffer Get() const noexcept { return *m_buffer; }
        vk::DeviceSize GetSize() const noexcept { return m_size; }
        vk::BufferUsageFlags GetUsage() const noexcept { return m_usage; }

        explicit operator bool() const noexcept { return Get() != vk::Buffer{}; }

    private:
        Buffer(vma::raii::Buffer&& buffer, const vk::BufferCreateInfo& createInfo)
            :
            m_buffer(std::move(buffer)),
            m_size(createInfo.size),
            m_usage(createInfo.usage),
            m_memoryPropertyFlags(m_buffer.getAllocation().getMemoryProperties())
        {
        }

        vma::raii::Buffer m_buffer{ nullptr };
        vk::DeviceSize m_size{ 0 };
        vk::BufferUsageFlags m_usage{};
        vk::MemoryPropertyFlags m_memoryPropertyFlags;
    };
}
