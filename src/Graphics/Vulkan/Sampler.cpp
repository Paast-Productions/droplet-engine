#include "Sampler.hpp"

using namespace Droplet::Graphics::Vulkan;

Sampler::Sampler(const vk::raii::PhysicalDevice &p_physicalDevice,
	const vk::raii::Device &p_device)
{
	const vk::PhysicalDeviceProperties properties = p_physicalDevice.getProperties();
	const bool anisotropySupported = p_physicalDevice.getFeatures().samplerAnisotropy;
	vk::SamplerCreateInfo        samplerInfo
	{
		.magFilter = vk::Filter::eLinear,
		.minFilter = vk::Filter::eLinear,
		.mipmapMode = vk::SamplerMipmapMode::eLinear,
		.addressModeU = vk::SamplerAddressMode::eRepeat,
		.addressModeV = vk::SamplerAddressMode::eRepeat,
		.addressModeW = vk::SamplerAddressMode::eRepeat,
		.mipLodBias = 0.0f,
		.anisotropyEnable = anisotropySupported ? vk::True : vk::False,
		.maxAnisotropy = std::min(Sampler::C_DEFAULT_MAX_ANISOTROPY, properties.limits.maxSamplerAnisotropy),
		.compareEnable = vk::False,
		.compareOp = vk::CompareOp::eAlways,
		.minLod = 0.0f,
		.maxLod = VK_LOD_CLAMP_NONE // Without this, only the base mip level is ever sampled
	};
	m_sampler = vk::raii::Sampler(p_device, samplerInfo);
}
