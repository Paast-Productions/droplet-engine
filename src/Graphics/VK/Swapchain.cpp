#include <Graphics/VK/Swapchain.hpp>

#include <cassert>

using namespace Droplet::Graphics::VK;

std::uint32_t Swapchain::AcquireNextImageIndex(const vk::raii::Device &p_device, const vk::raii::Semaphore &p_semaphore,
                                          const vk::raii::Fence &p_fence, uint64_t
                                          p_timeout)
{
	vk::AcquireNextImageInfoKHR nextImageInfo 
	{
		.swapchain = m_swapchain,	
		.timeout = p_timeout,
		.semaphore = p_semaphore,
		.fence = p_fence,
		.deviceMask = 1
	};
	
	vk::ResultValue<std::uint32_t> resVal = p_device.acquireNextImage2KHR(nextImageInfo);
	vk::Result result = resVal.result;
	
	if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
	{
		assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
		throw std::runtime_error("failed to acquire swap chain image!");
	}
	
	m_currentIndex = resVal.value;
	return m_currentIndex;
}

vk::SurfaceFormatKHR Swapchain::ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &p_availableFormats)
{
	const auto formatIt = std::ranges::find_if(
		p_availableFormats,
		[](const auto &format)
		{
			return format.format == vk::Format::eR8G8B8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
		});
	
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

Swapchain::Swapchain(const vma::raii::Allocator &p_allocator, const vk::raii::Device &p_device, const vk::raii::PhysicalDevice &p_physicalDevice, SDL_Window *p_window, const vk::raii::SurfaceKHR &p_swapSurface)
{
	vk::SurfaceCapabilitiesKHR surfaceCapabilities = p_physicalDevice.getSurfaceCapabilitiesKHR(*p_swapSurface);

	std::vector<vk::SurfaceFormatKHR> availableFormats = p_physicalDevice.getSurfaceFormatsKHR(*p_swapSurface);
	m_surfaceFormat = ChooseSwapSurfaceFormat(availableFormats);
	
	vk::Extent2D extent = ChooseSwapExtent(surfaceCapabilities, p_window);

	vk::SwapchainCreateInfoKHR swapChainCreateInfo
	{ 
		.surface = *p_swapSurface,
		.minImageCount = ChooseSwapMinImageCount(surfaceCapabilities),
		.imageFormat = m_surfaceFormat.format,
		.imageColorSpace = m_surfaceFormat.colorSpace,
		.imageExtent = extent,
		.imageArrayLayers = 1,
		.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
		.imageSharingMode = vk::SharingMode::eExclusive,
		.preTransform = surfaceCapabilities.currentTransform,
		.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
		.presentMode = ChooseSwapPresentMode(p_physicalDevice.getSurfacePresentModesKHR(*p_swapSurface)),
		.clipped = true 
	};
	
	m_swapchain = vk::raii::SwapchainKHR(p_device, swapChainCreateInfo);
	
	for (const vk::Image &image : m_swapchain.getImages())
	{
		m_targetIndices.push_back(IntermediaryStorage::GetInstance().CreatePresentTarget(image, m_surfaceFormat.format, extent.width, extent.height));
	}
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

	return 
	{
		std::clamp<uint32_t>(width, p_capabilities.minImageExtent.width, p_capabilities.maxImageExtent.width),
		std::clamp<uint32_t>(height, p_capabilities.minImageExtent.height, p_capabilities.maxImageExtent.height)
	};
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