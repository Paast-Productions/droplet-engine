#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#include <vulkan/vulkan_raii.hpp>
#undef VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

namespace Droplet::Graphics::VK
{
	/// @brief Descriptor pool class that manages descriptor sets and their layouts for Vulkan resource binding
	class DescriptorPool
	{
	public:
		/// @brief Deleted default constructor
		DescriptorPool() = delete;

		/// @brief Default deconstructor
		~DescriptorPool() = default;

		/// @brief Constructor for the Descriptor Pool Class
		/// @param p_device RAII pointer to the Vulkan device
		/// @param p_maxFramesInFlight highest count of frames in flight to be used in runtime
		DescriptorPool(vk::raii::Device const &p_device, std::uint32_t p_maxFramesInFlight);

		/// @brief Adds a new image to the descriptor set
		/// @param p_sampler RAII pointer to a sampler
		/// @param p_imageView RAII pointer to an image view
		void AddDescriptorImage(vk::raii::Sampler const &p_sampler, vk::raii::ImageView const &p_imageView);

		/// @brief Adds a new buffer to the descriptor set
		/// @param p_buffer RAII pointer to a buffer
		/// @param p_byteSize buffer size in bytes
		void AddDescriptorBuffer(vk::raii::Buffer const &p_buffer, uint32_t p_byteSize);

		/// @brief Initializes the descriptor set layout based on added descriptors
		/// @param p_device RAII pointer to the Vulkan device
		void CreateDescriptorSetLayout(vk::raii::Device const &p_device);
	private:
		vk::raii::DescriptorPool m_descriptorPool = nullptr;
		std::vector<vk::raii::DescriptorSet> m_descriptorSets;
		vk::raii::DescriptorSetLayout m_descriptorSetLayout = nullptr;

		std::vector<vk::DescriptorBufferInfo> m_bufferDescs;
		std::vector<vk::DescriptorImageInfo> m_imageDescs;
		std::vector<vk::DescriptorSetLayoutBinding> m_layoutBindings;

		void CreateDescriptorPool(vk::raii::Device const &p_device, std::uint32_t p_maxFramesInFlight);
		void CreateDescriptorSets();
	};
}