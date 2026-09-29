#include <Graphics/VK/VulkanContext.hpp>


#include <ranges>
#include <iostream>

using namespace Droplet::Graphics::VK;

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

const std::vector<char const *> validationLayers = {
	"VK_LAYER_KHRONOS_validation" };

/// @brief Callback function for the debug messenger
/// @param severity Severity flags of the error
/// @param type Type flag of the error
/// @param pCallbackData pointer to callback data 
/// @param void pointer 
/// @return false, to keep running
static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, void *)
{
	if (severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eError || severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
	{
		std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
	}

	return vk::False;
}

//Setup of the debug messenger
void VulkanContext::SetupDebugMessenger()
{
	if (!enableValidationLayers)
		return;

	vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
	vk::DebugUtilsMessageTypeFlagsEXT     messageTypeFlags(
		vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
	vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{ .messageSeverity = severityFlags,
																		  .messageType = messageTypeFlags,
																		  .pfnUserCallback = &DebugCallback };
	m_debugMessenger = m_instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
}

//Iterating through a list of available GPUs and choosing one to use
void VulkanContext::PickPhysicalDevice()
{
	std::vector<vk::raii::PhysicalDevice> physicalDevices = m_instance.enumeratePhysicalDevices();
	auto const                            devIter = std::ranges::find_if(physicalDevices, [&](auto const &physicalDevice) { return IsDeviceSuitable(physicalDevice); });
	if (devIter == physicalDevices.end())
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}
	m_physicalDevice = *devIter;
}

//Creation of a vulkan device
void VulkanContext::CreateLogicalDevice()
{
	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = m_physicalDevice.getQueueFamilyProperties();

	// get the first index into queueFamilyProperties which supports both graphics and present
	for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++)
	{
		if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) &&
			m_physicalDevice.getSurfaceSupportKHR(qfpIndex, *m_surface))
		{
			// found a queue family that supports both graphics and present
			m_queueIndex = qfpIndex;
			break;
		}
	}
	if (m_queueIndex == ~0)
	{
		throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
	}

	// query for Vulkan 1.3 features
	vk::StructureChain<vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan11Features,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
		featureChain = {
			{},									   // vk::PhysicalDeviceFeatures2
			{.shaderDrawParameters = true},        // vk::PhysicalDeviceVulkan11Features
			{.synchronization2 = true, .dynamicRendering = true}, // vk::PhysicalDeviceVulkan13Features   //Fixes sync2 warnings
			{.extendedDynamicState = true},        // vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
	};

	// create a Device
	float                     queuePriority = 0.5f;
	vk::DeviceQueueCreateInfo deviceQueueCreateInfo{ .queueFamilyIndex = m_queueIndex, .queueCount = 1, .pQueuePriorities = &queuePriority };
	vk::DeviceCreateInfo      deviceCreateInfo{ .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
											   .queueCreateInfoCount = 1,
											   .pQueueCreateInfos = &deviceQueueCreateInfo,
											   .enabledExtensionCount = static_cast<uint32_t>(m_requiredDeviceExtension.size()),
											   .ppEnabledExtensionNames = m_requiredDeviceExtension.data() };

	m_device = vk::raii::Device(m_physicalDevice, deviceCreateInfo);
	m_queue = vk::raii::Queue(m_device, m_queueIndex, 0);
}

//Checking if the device supports the correct features and API version
bool VulkanContext::IsDeviceSuitable(vk::raii::PhysicalDevice const &physicalDevice)
{
	// Check if the physicalDevice supports the Vulkan 1.3 API version
	bool supportsVulkan1_3 = physicalDevice.getProperties().apiVersion >= VK_API_VERSION_1_3;

	// Check if any of the queue families support both graphics and presentation to our surface
	auto     queueFamilies = physicalDevice.getQueueFamilyProperties();
	uint32_t qfpIndex = 0;
	bool     supportsGraphicsAndPresent =
		std::ranges::any_of(queueFamilies,
			[&physicalDevice, &surface = this->m_surface, &qfpIndex](auto const &qfp) {
				bool const suitable = (qfp.queueFlags & vk::QueueFlagBits::eGraphics) && physicalDevice.getSurfaceSupportKHR(qfpIndex, *surface);
				qfpIndex++;
				return suitable;
			});

	// Check if all required physicalDevice extensions are available
	auto availableDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
	bool supportsAllRequiredExtensions =
		std::ranges::all_of(m_requiredDeviceExtension,
			[&availableDeviceExtensions](auto const &requiredDeviceExtension) {
				return std::ranges::any_of(availableDeviceExtensions,
					[requiredDeviceExtension](auto const &availableDeviceExtension) { return strcmp(availableDeviceExtension.extensionName, requiredDeviceExtension) == 0; });
			});

	// Check if the physicalDevice supports the required features
	auto features = physicalDevice.template getFeatures2<vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan11Features,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
	bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
		features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
		features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

	// Return true if the physicalDevice meets all the criteria
	return supportsVulkan1_3 && supportsGraphicsAndPresent && supportsAllRequiredExtensions && supportsRequiredFeatures;
}

//Fetches required SDL instance extensions
std::vector<const char *> getRequiredInstanceExtensions()
{
	uint32_t extensionCount = 0;
	auto     sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&extensionCount);

	std::vector extensions(sdlExtensions, sdlExtensions + extensionCount);
	if (enableValidationLayers)
	{
		extensions.push_back(vk::EXTDebugUtilsExtensionName);
	}

	return extensions;
}

//Creating a surface for rendering onto
void VulkanContext::CreateSurface(SDL_Window &p_window)
{
	VkSurfaceKHR _surface;
	if (!SDL_Vulkan_CreateSurface(&p_window, *m_instance, nullptr, &_surface))
	{
		throw std::runtime_error("failed to create window surface!");
	}
	m_surface = vk::raii::SurfaceKHR(m_instance, _surface);
}

void VulkanContext::CreateInstance()
{
	constexpr vk::ApplicationInfo appInfo{ .pApplicationName = "Hello Triangle!",
										   .applicationVersion = VK_MAKE_VERSION(1,0,0),
										   .pEngineName = "No Engine",
										   .engineVersion = VK_MAKE_VERSION(1,0,0),
										   .apiVersion = vk::ApiVersion14 };

	// Get the required layers
	std::vector<char const *> requiredLayers;
	if (enableValidationLayers)
	{
		requiredLayers.assign(validationLayers.begin(), validationLayers.end());
	}

	// Check if the required layers are supported by the Vulkan implementation.
	auto layerProperties = m_context.enumerateInstanceLayerProperties();
	auto unsupportedLayerIt = std::ranges::find_if(requiredLayers,
		[&layerProperties](auto const &requiredLayer) {
			return std::ranges::none_of(layerProperties,
				[requiredLayer](auto const &layerProperty) { return strcmp(layerProperty.layerName, requiredLayer) == 0; });
		});
	if (unsupportedLayerIt != requiredLayers.end())
	{
		throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayerIt));
	}

	// Get the required extensions.
	auto requiredExtensions = getRequiredInstanceExtensions();

	// Check if the required extensions are supported by the Vulkan implementation.
	auto extensionProperties = m_context.enumerateInstanceExtensionProperties();
	auto unsupportedPropertyIt =
		std::ranges::find_if(requiredExtensions,
			[&extensionProperties](auto const &requiredExtension) {
				return std::ranges::none_of(extensionProperties,
					[requiredExtension](auto const &extensionProperty) { return strcmp(extensionProperty.extensionName, requiredExtension) == 0; });
			});
	if (unsupportedPropertyIt != requiredExtensions.end())
	{
		throw std::runtime_error("Required extension not supported: " + std::string(*unsupportedPropertyIt));
	}

	vk::InstanceCreateInfo createInfo{ .pApplicationInfo = &appInfo,
									  .enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
									  .ppEnabledLayerNames = requiredLayers.data(),
									  .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
									  .ppEnabledExtensionNames = requiredExtensions.data() };
	m_instance = vk::raii::Instance(m_context, createInfo);
}

VulkanContext::VulkanContext(SDL_Window &p_window)
{
	CreateInstance();
	SetupDebugMessenger();
	CreateSurface(p_window);
	PickPhysicalDevice();
	CreateLogicalDevice();
}