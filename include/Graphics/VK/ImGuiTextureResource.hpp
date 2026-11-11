#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#include <vulkan/vulkan_raii.hpp>
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 

#include <IDynamicResource.hpp>

namespace Droplet::Graphics
{
	class ImGuiTextureResource : public IDynamicResource 
	{
	public:
		ImGuiTextureResource() = default;
		
		ImGuiTextureResource(vk::DescriptorSet p_texture) : m_texture(p_texture) {}
		
		ImGuiTextureResource(const ImGuiTextureResource &p_other) = default;
		ImGuiTextureResource &operator=(const ImGuiTextureResource &p_other) = default;
		ImGuiTextureResource(ImGuiTextureResource &&p_other) noexcept = default;
		ImGuiTextureResource &operator=(ImGuiTextureResource &&p_other) noexcept = default;

		~ImGuiTextureResource() final = default;
	
		vk::DescriptorSet Get() const { return m_texture; }
	private:
	
		vk::DescriptorSet m_texture { nullptr };
	};
}
