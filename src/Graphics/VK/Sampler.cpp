#include <Graphics/VK/Sampler.hpp>

using namespace Droplet::Graphics::VK;

Sampler::Sampler(const vk::raii::PhysicalDevice &p_physicalDevice, const vk::raii::Device &p_device)
{
	vk::PhysicalDeviceProperties properties = p_physicalDevice.getProperties();
	vk::SamplerCreateInfo        samplerInfo
	{
		.magFilter = vk::Filter::eLinear,
		.minFilter = vk::Filter::eLinear,
		.mipmapMode = vk::SamplerMipmapMode::eLinear,
		.addressModeU = vk::SamplerAddressMode::eRepeat,
		.addressModeV = vk::SamplerAddressMode::eRepeat,
		.addressModeW = vk::SamplerAddressMode::eRepeat,
		.mipLodBias = 0.0f,
		.anisotropyEnable = vk::False,
		.maxAnisotropy = properties.limits.maxSamplerAnisotropy,
		.compareEnable = vk::False,
		.compareOp = vk::CompareOp::eAlways
	};
	m_sampler = vk::raii::Sampler(p_device, samplerInfo);
}
