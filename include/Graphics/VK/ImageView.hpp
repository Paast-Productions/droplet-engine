#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#include <vulkan/vulkan_raii.hpp>
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

namespace Droplet::Graphics::VK
{
	/// @brief ImageView class for storing textures/image buffers to be bound to shader stages
	class ImageView
	{
	public:
		/// @brief Deleted constructor
		ImageView() = delete;
		/// @brief Default destructor
		~ImageView() = default;

		/// @brief Constructor for image view based on image/texture
		/// @param p_device reference to the Vulkan device
		/// @param p_physicalDevice reference to the hardware device
		/// @param p_commandPool reference to the command pool
		/// @param p_queue reference to the queue
		/// @param p_pixels unsigned char pointer to the image/texture data
		/// @param p_width width of the texture in pixels
		/// @param p_height height of the texture in pixels
		/// @param p_format color/data format of the texture
		/// @param p_tiling Image tiling flag
		/// @param p_usage Image usage flags
		/// @param p_properties Memory property flags
		ImageView(
			const vk::raii::Device &p_device, 
			const vk::raii::PhysicalDevice &p_physicalDevice,
			const vk::raii::CommandPool &p_commandPool, 
			const vk::raii::Queue &p_queue,
			const unsigned char *p_pixels, 
			std::uint32_t p_width,
			std::uint32_t p_height, 
			vk::Format p_format, 
			vk::ImageTiling p_tiling, 
			vk::ImageUsageFlags p_usage, 
			vk::MemoryPropertyFlags p_properties);

		/// @brief Constructor for creating an imageview
		/// @param p_device reference to the Vulkan device
		/// @param p_physicalDevice reference to the hardware device
		/// @param p_width width of the image
		/// @param p_height height of the image
		/// @param p_format data format of the image
		/// @param p_tiling Image tiling flag
		/// @param p_usage Image usage flags
		/// @param p_properties Memory property flags
		ImageView(
			const vk::raii::Device &p_device,
			const vk::raii::PhysicalDevice &p_physicalDevice,
			uint32_t p_width, 
			uint32_t p_height,
			vk::Format p_format,
			vk::ImageTiling p_tiling,
			vk::ImageUsageFlags p_usage,
			vk::MemoryPropertyFlags p_properties);

		/// @brief Image getter
		/// @return pointer to the image
		const vk::raii::Image *GetImage();

		/// @brief ImageView getter
		/// @return pointer to the imageview
		const vk::raii::ImageView *GetView();
	private:
		vk::raii::Image m_image = nullptr;
		vk::raii::ImageView m_view = nullptr;
		vk::raii::DeviceMemory m_imageMemory = nullptr;
	};

}