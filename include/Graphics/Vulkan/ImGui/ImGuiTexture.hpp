<<<<<<<< HEAD:include/Graphics/Vulkan/ImGui/ImGuiTexture.hpp
#pragma once


#include <Graphics/Vulkan/ImGui/ImGuiTextureResource.hpp>
#include <Graphics/Vulkan/Image.hpp>
#include <Graphics/Vulkan/ImageView.hpp>

#include <Resource/DynamicResourceHandle.hpp>

namespace Droplet::Graphics::ImGui
{
	class ImGuiTexture
	{
	public:
		ImGuiTexture() = default;
		
		ImGuiTexture(DynamicResourceHandle<ImGuiTextureResource> p_textureHandle) :
			m_textureHandle(p_textureHandle) {}
		
		ImGuiTexture(DynamicResourceHandle<ImGuiTextureResource> p_textureHandle,
			DynamicResourceHandle<Vulkan::Image> p_image,
			DynamicResourceHandle<Vulkan::ImageView> p_imageView) : 
				m_textureHandle(p_textureHandle),
				m_image(p_image),
				m_imageView(p_imageView) {}
		
		ImGuiTexture(const ImGuiTexture &p_other) = default;
		ImGuiTexture &operator=(const ImGuiTexture &p_other) = default;
		ImGuiTexture(ImGuiTexture &&p_other) noexcept = default;
		ImGuiTexture &operator=(ImGuiTexture &&p_other) noexcept = default;

		~ImGuiTexture() = default;

#ifdef _EDITOR
		ImTextureRef Get() const { return static_cast<VkDescriptorSet>(m_textureHandle->Get()); }
#endif
		
		GUID GetGUID() const { return m_textureHandle.GetGUID(); }
		
	private:
		DynamicResourceHandle<ImGuiTextureResource> m_textureHandle {};
		DynamicResourceHandle<Vulkan::Image> m_image {};
		DynamicResourceHandle<Vulkan::ImageView> m_imageView {};
	};
}
========
>>>>>>>> 9678edf7 (Cleaning up and organising the Engine project for migration.):include/Graphics/Vulkan/ImGuiTexture.hpp
