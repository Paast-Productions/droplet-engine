#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vk_mem_alloc_raii.hpp>
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 

namespace Droplet::Graphics::VK
{
    class StorageBuffer
    {
    public:
        StorageBuffer() = default;
        
        StorageBuffer(const std::nullptr_t p_nullptr) :
            m_buffer { p_nullptr } {}
        
        /// @brief StorageBuffer Constructor
        /// @param p_allocator VMA Allocator
        /// @param p_data Data to write
        template <typename T>
        StorageBuffer(const vma::raii::Allocator &p_allocator, const T &p_data)
        {
            static_assert(sizeof(p_data) % sizeof(std::uint32_t) * 4 == 0, "[ERROR: Uniform Buffer]\nSize of p_data must be a multiple of 16 bytes");
                
            m_bufferMaxSize = sizeof(p_data);
                
            const vk::BufferCreateInfo bufferCreateInfo
            {
                .size = m_bufferMaxSize, 
                .usage = vk::BufferUsageFlagBits::eStorageBuffer,
                .sharingMode = vk::SharingMode::eExclusive
            };

            constexpr vma::AllocationCreateInfo allocCreateInfo
            {
                .flags = vma::AllocationCreateFlagBits::eHostAccessRandom,
                .usage = vma::MemoryUsage::eAuto,
                .requiredFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
            };
        
            m_buffer = vma::raii::Buffer {p_allocator, bufferCreateInfo, allocCreateInfo};
        
            UpdateBuffer(p_data);
        }
        
        StorageBuffer(const StorageBuffer &p_other) = delete;
        StorageBuffer &operator=(const StorageBuffer &p_other) = delete;
        
        StorageBuffer(StorageBuffer &&p_other) noexcept
        {
            std::swap(m_buffer, p_other.m_buffer);
            m_bufferMaxSize = p_other.m_bufferMaxSize;
        }
        
        StorageBuffer &operator=(StorageBuffer &&p_other) noexcept
        {
            if (*this == p_other)
            {
                return *this;
            }
            
            std::swap(m_buffer, p_other.m_buffer);
            m_bufferMaxSize = p_other.m_bufferMaxSize;
            
            return *this;
        }
        
        ~StorageBuffer() = default;
        
        bool operator==(const StorageBuffer &p_other) const
        {
            if (m_buffer == p_other.m_buffer				&&
                m_bufferMaxSize == p_other.m_bufferMaxSize)
            {
                return true;
            }
                
            return false;
        }
        
        /// @brief Update buffer with new data
        /// @param p_data New data to update to
        template <typename T>
        void UpdateBuffer(const T& p_data) const
        {
            static_assert(sizeof(p_data) % sizeof(std::uint32_t) * 4 == 0, "[ERROR: Storage Buffer]\nSize of p_data must be a multiple of 16 bytes");
                
            if (sizeof(p_data) > m_bufferMaxSize)
            {
                throw std::runtime_error(std::format("[ERROR: Storage Buffer]\nData to be copied into buffer is bigger than the buffer!\nSize of buffer: {0}\nSize of data: {1}", m_bufferMaxSize, sizeof p_data));
            }
                
            const vma::raii::Allocation &allocation = m_buffer.getAllocation();
            allocation.copyFromMemory(&p_data, 0, sizeof(p_data));
        }
        
        /// @brief Getter-function for the buffer
        /// @returns VMA Buffer
        [[nodiscard]] const vma::raii::Buffer &Get() const { return m_buffer; } 
        
    private:
        
        vma::raii::Buffer m_buffer { nullptr }; 
        std::size_t m_bufferMaxSize { 0 };
    };
}

