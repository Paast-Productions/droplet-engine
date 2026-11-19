#pragma once

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
    
    // TODO: <REFACTOR>
    // Author: SkyH1ghDev
    // Should not clutter the Vulkan namespace. Make it private in a class or something
    inline vma::AllocationCreateInfo MakeAllocationInfo(const MemoryAccess p_access)
    {
        using FlagBits = vma::AllocationCreateFlagBits;
        switch (p_access)
        {
            case MemoryAccess::GpuOnly:
                return 
                {
                    .flags = {},
                    .usage = vma::MemoryUsage::eAutoPreferDevice
                };
                    
            case MemoryAccess::Dynamic:   // picks ReBAR if present, else host RAM
                return
                {
                    .flags = FlagBits::eHostAccessSequentialWrite | FlagBits::eMapped,
                    .usage = vma::MemoryUsage::eAutoPreferDevice 
                };
                    
            case MemoryAccess::Upload:    // device-local, mappable only if that's cheap
                return 
                {
                    .flags = FlagBits::eHostAccessSequentialWrite | FlagBits::eHostAccessAllowTransferInstead | FlagBits::eMapped,
                    .usage = vma::MemoryUsage::eAutoPreferDevice 
                };
                    
            case MemoryAccess::Staging:
                return 
                {
                    .flags = FlagBits::eHostAccessSequentialWrite | FlagBits::eMapped,
                    .usage = vma::MemoryUsage::eAutoPreferHost 
                };
                    
            case MemoryAccess::Readback:
                return 
            {
                    .flags = FlagBits::eHostAccessRandom | FlagBits::eMapped,
                    .usage = vma::MemoryUsage::eAutoPreferHost 
                };
                    
            default:
                return {};
        }
    }
    // TODO: </REFACTOR>
    
    /// Options struct for buffer creation
    struct BufferCreateOptions
    {
        vk::DeviceSize size { 0 };
        vk::BufferUsageFlags usage {};
        vk::BufferCreateFlags flags {};
        vma::AllocationCreateInfo allocationInfo { MakeAllocationInfo(MemoryAccess::GpuOnly) };
    
        // TODO: <REFACTOR> 
        // Author: SkyH1ghDev
        // Function makes no sense outside of struct. Make private or something.
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
        // TODO: </REFACTOR>
        
        /**
         * @brief Returns a BufferCreateOptions struct to make a vertex buffer with Buffer::Create
         * @param p_size size of the buffer
         * @param p_access How the buffer will be handled with memory
         * @return
         */
        static BufferCreateOptions Vertex(
            vk::DeviceSize p_size,
            MemoryAccess p_access = MemoryAccess::GpuOnly,
            vk::BufferUsageFlags p_extra = {})
        {
            return For(p_size, vk::BufferUsageFlagBits::eVertexBuffer | p_extra, p_access);
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
         * @param p_size size of the buffer
         * @param usage any additional flags.
         * @return
         */
        static BufferCreateOptions Uniform(const vk::DeviceSize p_size,
            const MemoryAccess p_access = MemoryAccess::Dynamic,
            const vk::BufferUsageFlags p_extra = {})
        {
            return For(p_size, vk::BufferUsageFlagBits::eUniformBuffer | p_extra, p_access);
        }

        /**
         * @brief Staging buffer
         * @param p_size size of the device memory
         * @return
         */
        static BufferCreateOptions Staging(vk::DeviceSize p_size)
        {
            return For(p_size, vk::BufferUsageFlagBits::eTransferSrc, MemoryAccess::Staging);
        }
    };
}
