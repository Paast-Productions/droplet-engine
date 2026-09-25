#pragma once

#include <Graphics/VK/DescriptorPool.hpp>
#include <Graphics/VK/DescriptorSet.hpp>
#include <vk_mem_alloc_raii.hpp>

namespace Droplet::Graphics::VK
{
    enum class BufferBindingType : std::uint8_t
    {
        UNIFORM = 0,
        STORAGE = 1,
        TEXTURE = 2
    };
    
    enum class TextureHandle : std::uint32_t { Invalid = 0 };
    enum class BufferHandle : std::uint32_t { Invalid = 0 };

    struct test
    {
        BufferHandle meshTransforms {};
        BufferHandle Camera {};
        std::uint32_t pad0 {};
        std::uint32_t pad1 {};
    };
    
    class BufferStorage
    {
    public:
        BufferStorage() = delete;
        
        BufferStorage(const vk::raii::PhysicalDevice &p_device);
        
        BufferStorage(const BufferStorage &) = delete;
        BufferStorage &operator=(const BufferStorage &) = delete;
        BufferStorage(BufferStorage &&) = delete;
        BufferStorage &operator=(BufferStorage &&) = delete;

        ~BufferStorage() = default;
        
        [[nodiscard]] BufferHandle StoreBuffer(const vk::raii::Device &p_device, const vk::raii::Buffer &buffer, vk::BufferUsageFlagBits usage);
        
    private:
        std::vector<vk::raii::ImageView> m_textures {};
        std::vector<vk::Buffer> m_buffers {};
        
        std::uint32_t m_minUniformBufferOffsetAlignment {};
        
    };
}
