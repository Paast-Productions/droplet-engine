#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#include <vulkan/vulkan_raii.hpp>
#undef VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

namespace Droplet::Graphics::VK
{
	/// @class DescriptorPool
	/// @brief Descriptor pool class that manages descriptor sets and their layouts for Vulkan resource binding
	class DescriptorPool
	{
	public:
		DescriptorPool() = delete;
		
		/// @brief DescriptorPool Nullptr Constructor
		DescriptorPool(nullptr_t p_nullptr) : 
			m_descriptorPool(p_nullptr) {}

		/// @brief Constructor for the Descriptor Pool Class
		/// @param p_device RAII pointer to the Vulkan device
		DescriptorPool(const vk::raii::Device &p_device);
		
		~DescriptorPool() = default;
		
		/// @brief Getter function for Vulkan Descriptor Pool
		/// @returns Vulkan Descriptor Pool
		[[nodiscard]] const vk::raii::DescriptorPool &Get() const { return m_descriptorPool; }

        DescriptorPool(const DescriptorPool &p_other) = delete;
        DescriptorPool &operator=(const DescriptorPool &p_other) = delete;
        
        DescriptorPool(DescriptorPool &&p_other) noexcept
        {
            std::swap(m_descriptorPool, p_other.m_descriptorPool);
        };
        
        DescriptorPool &operator=(DescriptorPool &&p_other) noexcept
        {
            if (*this == p_other)
            {
                return *this;
            }
            
            std::swap(m_descriptorPool, p_other.m_descriptorPool);
            
            return *this;
        };
        
        bool operator==(const DescriptorPool &p_other) const
        {
            return (m_descriptorPool == p_other.m_descriptorPool);
        }

	private:
		vk::raii::DescriptorPool m_descriptorPool = nullptr;

	};
}
