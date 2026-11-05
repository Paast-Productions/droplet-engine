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
		ImGuiTexture(DynamicResourceHandle<ImGuiTextureResource> p_textureHandle, DynamicResourceHandle<VK::Image> p_image, DynamicResourceHandle<VK::ImageView> p_imageView) : 
			m_textureHandle(p_textureHandle), m_image(p_image), m_imageView(p_imageView) {}
		
		ImGuiTexture(const ImGuiTexture &p_other) = default;
		ImGuiTexture &operator=(const ImGuiTexture &p_other) = default;
		ImGuiTexture(ImGuiTexture &&p_other) noexcept = default;
		ImGuiTexture &operator=(ImGuiTexture &&p_other) noexcept = default;

		~ImGuiTexture() = default;
		
		ImTextureRef Get() const { return static_cast<VkDescriptorSet>(m_textureHandle->Get()); }
		
		GUID GetGUID() const { return m_textureHandle.GetGUID(); }
		
	private:
		DynamicResourceHandle<ImGuiTextureResource> m_textureHandle {};
		DynamicResourceHandle<VK::Image> m_image {};
		DynamicResourceHandle<VK::ImageView> m_imageView {};
	};
}
