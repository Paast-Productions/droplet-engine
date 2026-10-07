#include <Graphics/VK/Swapchain.hpp>

#include <assert.h>

using namespace Droplet::Graphics::VK;

vk::SurfaceFormatKHR Swapchain::ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &p_availableFormats)
{
	const auto formatIt = std::ranges::find_if(
		p_availableFormats,
		[](const auto &format) { return format.format == vk::Format::eR8G8B8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; });
	return formatIt != p_availableFormats.end() ? *formatIt : p_availableFormats[0];
}

std::uint32_t Swapchain::ChooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const &p_surfaceCapabilities)
{
	auto minImageCount = std::max(3u, p_surfaceCapabilities.minImageCount);
	if ((0 < p_surfaceCapabilities.maxImageCount) && (p_surfaceCapabilities.maxImageCount < minImageCount))
	{
		minImageCount = p_surfaceCapabilities.maxImageCount;
	}
	return minImageCount;
}

void Swapchain::CreateSwapchain(const vk::raii::Device &p_device, const vk::raii::PhysicalDevice &p_physicalDevice, SDL_Window *p_window, const vk::raii::SurfaceKHR &p_swapSurface)
{
	vk::SurfaceCapabilitiesKHR surfaceCapabilities = p_physicalDevice.getSurfaceCapabilitiesKHR(*p_swapSurface);
	m_swapchainExtent = ChooseSwapExtent(surfaceCapabilities, p_window);
	uint32_t minImageCount = ChooseSwapMinImageCount(surfaceCapabilities);

	std::vector<vk::SurfaceFormatKHR> availableFormats = p_physicalDevice.getSurfaceFormatsKHR(*p_swapSurface);
	m_swapchainSurfaceFormat = ChooseSwapSurfaceFormat(availableFormats);

	std::vector<vk::PresentModeKHR> availablePresentModes = p_physicalDevice.getSurfacePresentModesKHR(*p_swapSurface);

	vk::SwapchainCreateInfoKHR swapChainCreateInfo{ .surface = *p_swapSurface,
											   .minImageCount = minImageCount,
											   .imageFormat = m_swapchainSurfaceFormat.format,
											   .imageColorSpace = m_swapchainSurfaceFormat.colorSpace,
											   .imageExtent = m_swapchainExtent,
											   .imageArrayLayers = 1,
											   .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
											   .imageSharingMode = vk::SharingMode::eExclusive,
											   .preTransform = surfaceCapabilities.currentTransform,
											   .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
											   .presentMode = ChooseSwapPresentMode(availablePresentModes),
											   .clipped = true };

	m_swapchain = vk::raii::SwapchainKHR(p_device, swapChainCreateInfo);
	m_swapchainImages = { m_swapchain.getImages() };
}

Swapchain::Swapchain(const vk::raii::Device &p_device, const vk::raii::PhysicalDevice &p_physicalDevice, SDL_Window *p_window, const vk::raii::SurfaceKHR &p_swapSurface)
{
	CreateSwapchain(p_device, p_physicalDevice, p_window, p_swapSurface);
	CreateImageViews(p_device);
}

vk::Extent2D Swapchain::ChooseSwapExtent(vk::SurfaceCapabilitiesKHR const &p_capabilities, SDL_Window *p_window)
{
	// currentExtent is only set to the special "undefined" value described above
	// when the window manager lets us choose the extent ourselves; any other value
	// means the surface already dictates a fixed extent that we must use as-is.
	if (p_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return p_capabilities.currentExtent;
	}
	int width {0}, height {0};
	SDL_GetWindowSize(p_window, &width, &height);

	return {
		std::clamp<uint32_t>(width, p_capabilities.minImageExtent.width, p_capabilities.maxImageExtent.width),
		std::clamp<uint32_t>(height, p_capabilities.minImageExtent.height, p_capabilities.maxImageExtent.height)
	};
}

void Swapchain::Cleanup(const vk::raii::Device &p_device)
{
	p_device.waitIdle();
	m_swapchainImageViews.clear();
	m_swapchain = nullptr;
}

void Swapchain::Recreate(const vk::raii::Device &p_device, const vk::raii::PhysicalDevice &p_physicalDevice, SDL_Window *p_window, const vk::raii::SurfaceKHR &p_surface)
{
	p_device.waitIdle();
	CreateSwapchain(p_device, p_physicalDevice, p_window, p_surface);
	CreateImageViews(p_device);
}

//Use eMailbox to avoid tearing while still maintaining fairly low latency by rendering new images that are as up to date as possible right until the vertical blank (higher energy usage?)
vk::PresentModeKHR Swapchain::ChooseSwapPresentMode(std::vector<vk::PresentModeKHR> const &availablePresentModes)
{
	assert(std::ranges::any_of(availablePresentModes, [](auto presentMode) { return presentMode == vk::PresentModeKHR::eFifo; }));
	return std::ranges::any_of(availablePresentModes,
		[](const vk::PresentModeKHR value) { return vk::PresentModeKHR::eMailbox == value; }) ?
		vk::PresentModeKHR::eMailbox :
		vk::PresentModeKHR::eFifo;
}

void Swapchain::CreateImageViews(const vk::raii::Device &p_device)
{
	assert(m_swapchainImageViews.empty());

	m_swapchainImageViews.reserve(m_swapchainImages.size());
	for (auto &image : m_swapchainImages)
	{
		vk::ImageViewCreateInfo viewInfo{
		.image = image,
		.viewType = vk::ImageViewType::e2D,
		.format = m_swapchainSurfaceFormat.format,
		.subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1} };
		m_swapchainImageViews.emplace_back(p_device, viewInfo);
	}


}