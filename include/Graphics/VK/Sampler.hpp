#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <Graphics/VK/Context.hpp>

namespace Droplet::Graphics::VK
{
	class Sampler
	{
	public:
		Sampler() = delete;
		~Sampler() = default;

		Sampler(const Context &p_context);

	private:
		vk::raii::Sampler m_sampler { nullptr };
	};
}