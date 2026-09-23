#pragma once

#include <Graphics/VK/DescriptorPool.hpp>
#include <Graphics/VK/DescriptorSet.hpp>

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

    struct alignas(16) test
    {
        BufferHandle meshTransforms;
        BufferHandle Camera;
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
        [[nodiscard]] std::uint32_t PadSizeToMinAlignment(std::uint32_t p_originalSize);
        
        std::vector<vk::raii::ImageView> m_textures {};
        std::vector<vk::raii::Buffer> m_buffers {};
        
        std::uint32_t m_minUniformBufferOffsetAlignment {};
        
    };

    inline std::uint32_t BufferStorage::PadSizeToMinAlignment(std::uint32_t p_originalSize)
    {
        return (p_originalSize + m_minUniformBufferOffsetAlignment - 1) & ~(m_minUniformBufferOffsetAlignment - 1);
    }
}
