#include <Graphics/VK/ImageView.hpp>
#include <BufferHelper.hpp>

using namespace Droplet::Graphics::VK;

uint32_t findMemoryType(const vk::raii::PhysicalDevice &p_physicalDevice, uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
	vk::PhysicalDeviceMemoryProperties memProperties = p_physicalDevice.getMemoryProperties();

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

ImageView::ImageView(const vk::raii::Device &p_device,
					 const vk::raii::PhysicalDevice &p_physicalDevice, 
					 const vk::raii::CommandPool &p_commandPool,
					 const vk::raii::Queue &p_queue,
					 const unsigned char *p_pixels, 
					 std::uint32_t p_width, 
					 std::uint32_t p_height, 
					 vk::Format p_format, 
					 vk::ImageTiling p_tiling, 
					 vk::ImageUsageFlags p_usage, 
					 vk::MemoryPropertyFlags p_properties)
{
	vk::DeviceSize imageSize = p_width * p_height * 4;
	auto [stagingBuffer, stagingBufferMemory] =
		CreateBuffer(p_device, p_physicalDevice, imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	void *data = stagingBufferMemory.mapMemory(0, imageSize);
	memcpy(data, p_pixels, imageSize);
	stagingBufferMemory.unmapMemory();

	vk::ImageCreateInfo imageInfo{ .imageType = vk::ImageType::e2D,
								  .format = p_format,
								  .extent = {p_width, p_height, 1},
								  .mipLevels = 1,
								  .arrayLayers = 1,
								  .samples = vk::SampleCountFlagBits::e1,
								  .tiling = p_tiling,
								  .usage = p_usage,
								  .sharingMode = vk::SharingMode::eExclusive };

	m_image = vk::raii::Image(p_device, imageInfo);

	vk::MemoryRequirements memRequirements = m_image.getMemoryRequirements();
	vk::MemoryAllocateInfo allocInfo{ .allocationSize = memRequirements.size,
									 .memoryTypeIndex = findMemoryType(p_physicalDevice, memRequirements.memoryTypeBits, p_properties) };
	m_imageMemory = vk::raii::DeviceMemory(p_device, allocInfo);
	m_image.bindMemory(m_imageMemory, 0);

	vk::raii::CommandBuffer commandBuffer = BeginSingleTimeCommands(p_device, p_commandPool);
	TransitionImageLayout(commandBuffer, m_image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

	vk::BufferImageCopy region{ .bufferOffset = 0,
							   .bufferRowLength = 0,
							   .bufferImageHeight = 0,
							   .imageSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1},
							   .imageOffset = {0, 0, 0},
							   .imageExtent = {p_width, p_height, 1} };
	commandBuffer.copyBufferToImage(stagingBuffer, m_image, vk::ImageLayout::eTransferDstOptimal, region);

	TransitionImageLayout(commandBuffer, m_image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
	EndSingleTimeCommands(std::move(commandBuffer), p_queue);

	vk::ImageViewCreateInfo viewInfo{
		.image = m_image,
		.viewType = vk::ImageViewType::e2D,
		.format = vk::Format::eR8G8B8A8Srgb,
		.subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1} };
	m_view = vk::raii::ImageView(p_device, viewInfo);
}

//std::pair<vk::raii::Image, vk::raii::DeviceMemory> 
ImageView::ImageView(const vk::raii::Device &p_device, 
					 const vk::raii::PhysicalDevice &p_physicalDevice, 
					 uint32_t p_width, 
					 uint32_t p_height, 
					 vk::Format p_format, 
					 vk::ImageTiling p_tiling, 
					 vk::ImageUsageFlags p_usage, 
					 vk::MemoryPropertyFlags p_properties)
{
	vk::ImageCreateInfo imageInfo{.imageType = vk::ImageType::e2D,
								  .format = p_format,
								  .extent = {p_width, p_height, 1},
								  .mipLevels = 1,
								  .arrayLayers = 1,
								  .samples = vk::SampleCountFlagBits::e1,
								  .tiling = p_tiling,
								  .usage = p_usage,
								  .sharingMode = vk::SharingMode::eExclusive };

	m_image = vk::raii::Image(p_device, imageInfo);

	vk::MemoryRequirements memRequirements = m_image.getMemoryRequirements();
	vk::MemoryAllocateInfo allocInfo{ .allocationSize = memRequirements.size,
									 .memoryTypeIndex = findMemoryType(p_physicalDevice, memRequirements.memoryTypeBits, p_properties) };
	m_imageMemory = vk::raii::DeviceMemory(p_device, allocInfo);
	m_image.bindMemory(m_imageMemory, 0);

}