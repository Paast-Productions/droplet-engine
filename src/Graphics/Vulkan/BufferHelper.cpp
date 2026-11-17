#include "BufferHelper.hpp"

void Droplet::Graphics::Vulkan::BufferHelper::TransitionImageLayout(const vk::raii::CommandBuffer &p_commandBuffer, const vk::raii::Image &p_image, vk::ImageLayout p_oldLayout, vk::ImageLayout p_newLayout)
{
	vk::ImageMemoryBarrier barrier{ .oldLayout = p_oldLayout,
								   .newLayout = p_newLayout,
								   .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
								   .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
								   .image = p_image,
								   .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor, .levelCount = 1, .layerCount = 1} };

	vk::PipelineStageFlags sourceStage;
	vk::PipelineStageFlags destinationStage;

	if (p_oldLayout == vk::ImageLayout::eUndefined && p_newLayout == vk::ImageLayout::eTransferDstOptimal)
	{
		barrier.srcAccessMask = {};
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (p_oldLayout == vk::ImageLayout::eTransferDstOptimal && p_newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else
	{
		throw std::invalid_argument("unsupported layout transition!");
	}
	p_commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, {}, barrier);
}