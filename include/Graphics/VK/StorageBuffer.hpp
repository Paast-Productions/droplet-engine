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
        StorageBuffer(
            const vma::raii::Allocator &p_allocator,
            const T &p_data)
            : StorageBuffer(p_allocator, &p_data, sizeof(T))
        {
            static_assert(std::is_trivially_copyable_v<T>);
        }
        
        /// @brief StorageBuffer Constructor
        /// @param p_allocator VMA Allocator
        /// @param p_data Data to write
        template <typename T>
        StorageBuffer(
            const vma::raii::Allocator &p_allocator,
            const std::vector<T> &p_data)
            : StorageBuffer(p_allocator, p_data.data(), p_data.size() * sizeof(T))
        {
            static_assert(std::is_trivially_copyable_v<T>, "[ERROR: Storage Buffer]\nStorage buffer data must be trivially copyable.");
        }
        
        StorageBuffer(const StorageBuffer &p_other) = delete;
        StorageBuffer &operator=(const StorageBuffer &p_other) = delete;
        
        StorageBuffer(StorageBuffer &&p_other) noexcept : m_buffer(std::move(p_other.m_buffer)), m_bufferCapacity(std::exchange(p_other.m_bufferCapacity, 0)) { }
        
        StorageBuffer &operator=(StorageBuffer &&p_other) noexcept
        {
            if (this == &p_other)
            {
                return *this;
            }
            
            m_buffer = std::move(p_other.m_buffer);
            m_bufferCapacity = std::exchange(p_other.m_bufferCapacity, 0);
            
            return *this;
        }
        
        ~StorageBuffer() = default;
        
        bool operator==(const StorageBuffer &p_other) const
        {
            if (m_buffer == p_other.m_buffer				&&
                m_bufferCapacity == p_other.m_bufferCapacity)
            {
                return true;
            }
                
            return false;
        }
        
        /// @brief Update buffer with new data
        /// @param p_data New data to update to
        template <typename T>
        void UpdateBuffer(const T& p_data)
        {
            static_assert(std::is_trivially_copyable_v<T>, "[ERROR: Storage Buffer]\nStorage buffer data must be trivially copyable.");
            UpdateBytes(&p_data, sizeof(T));
        }
        
        /// @brief Update buffer with new data
        /// @param p_data New data to update to
        template <typename T>
        void UpdateBuffer(const std::vector<T> &p_data)
        {
            static_assert(std::is_trivially_copyable_v<T>, "[ERROR: Storage Buffer]\nStorage buffer data must be trivially copyable.");
            UpdateBytes(p_data.data(), p_data.size() * sizeof(T));
        }
        
        /// @brief Getter-function for the buffer
        /// @returns VMA Buffer
        [[nodiscard]] const vma::raii::Buffer &Get() const { return m_buffer; } 
        
    private:
        StorageBuffer(const vma::raii::Allocator &p_allocator, const void* p_data, size_t p_size);
        void UpdateBytes(const void* p_data, std::size_t p_size);
        
        vma::raii::Buffer m_buffer { nullptr }; 
        std::size_t m_bufferCapacity { 0 };
    };
}

