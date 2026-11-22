#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace Droplet::Graphics::Vulkan::BufferHelper
{
	// TODO: (RENDERING) MOVE
	/// @brief Translates image data from one layout to another
	/// @param p_commandBuffer RAII pointer to a command buffer
	/// @param p_image RAII pointer to the image
	/// @param p_oldLayout Specified layout to translate from
	/// @param p_newLayout Specified layout to translate to
	void TransitionImageLayout(
		const vk::raii::CommandBuffer &p_commandBuffer,
		const vk::raii::Image &p_image,
		vk::ImageLayout p_oldLayout,
		vk::ImageLayout p_newLayout);
}
