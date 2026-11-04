#pragma once

#include <Graphics/VK/ImGuiTextureResource.hpp>
#include <Resource/DynamicResourceHandle.hpp>

namespace Droplet::Graphics
{
	class ImGuiTexture
	{
	public:
		ImGuiTexture() = default;
		
		ImGuiTexture(DynamicResourceHandle<ImGuiTextureResource> p_textureHandle) : m_textureHandle(p_textureHandle) {}
		
		ImGuiTexture(const ImGuiTexture &p_other) = default;
		ImGuiTexture &operator=(const ImGuiTexture &p_other) = default;
		ImGuiTexture(ImGuiTexture &&p_other) noexcept = default;
		ImGuiTexture &operator=(ImGuiTexture &&p_other) noexcept = default;

		~ImGuiTexture() = default;
		
		vk::DescriptorSet Get() { return m_textureHandle->Get(); }
		
	private:
		DynamicResourceHandle<ImGuiTextureResource> m_textureHandle {};
	};
}
