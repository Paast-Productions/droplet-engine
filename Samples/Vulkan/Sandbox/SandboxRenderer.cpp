/**
 * @file SandboxRenderer.cpp
 * @author Sebastian Lorensson
 * @date 2026-10-16
 * @brief Sandbox main for testing Vulkan functionality such as synchronisation with pipeline barriers, events, fences and timeline semaphores.
 */
#include "SandboxRenderer.hpp"
#include "SandboxSDLVulkan.hpp"

#include <Core/FileUtils.hpp>
#include "SandboxVulkanUtils.hpp"

// stl
#include <vector>
#include <print>
#include <algorithm>
#include <string>
#include <cstring>
#include <iostream>
#include <utility>

using namespace Droplet::Sandbox;

using Droplet::Sandbox::VulkanUtils::CheckResult;

namespace
{
    void PrintRejectReason(std::string_view reason)
    {
        std::println(
            "  Device Rejected: {}", reason.data());
    }

    VKAPI_ATTR vk::Bool32 VKAPI_CALL
    DebugCallback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
        vk::DebugUtilsMessageTypeFlagsEXT type,
        const vk::DebugUtilsMessengerCallbackDataEXT* data,
        void*)
    {
        if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
            || severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
        {
            std::cerr << "validation layer: type " << to_string(type)
                << " msg: " << data->pMessage << '\n';
        }


        return vk::False;
    }
}

// =====================================================================
// Sandbox Renderer
// =====================================================================
SandboxRenderer::SandboxRenderer(SDL_Window* window)
    : m_windowRef{window}
{
    // 1. Vulkan instance and window as surface
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();

    // 2. Find a GPU and query its capabilities, then create a device interface
    SelectPhysicalDevice();
    CreateLogicalDevice();

    // VMA Allocator depends on device
    CreateAllocator();

    // 3. Swapchain and views to it
    CreateSwapchain();
    CreateImageViews();

    // 4. Create the command queue and fence + semaphore
    CreateCommandResources();
    CreateSyncObjects();

    // 5. Pipeline layout TODO: move
    CreatePipelineLayout();
}

SandboxRenderer::~SandboxRenderer() noexcept
{
    try
    {
        m_device.waitIdle();
    }
    catch (const std::exception& e)
    {
        std::println(
            "Vulkan device wait during shutdown failed: {}",
            e.what());
    }
    catch (...)
    {
        std::println(
            "Vulkan device wait during shutdown failed.");
    }
}

vk::raii::ShaderModule SandboxRenderer::CreateShaderModule(const std::vector<std::uint32_t>& spirv)
{
    const vk::ShaderModuleCreateInfo createInfo{
        .codeSize =
        spirv.size() * sizeof(std::uint32_t),
        .pCode = spirv.data()
    };

    return {m_device, createInfo};
}

void SandboxRenderer::CreateShaderModules(
    const std::vector<std::uint32_t>& vertexSpirv,
    const std::vector<std::uint32_t>& fragmentSpirv)
{
    m_vertexShaderModule = CreateShaderModule(vertexSpirv);
    m_fragmentShaderModule = CreateShaderModule(fragmentSpirv);
}

void SandboxRenderer::DrawFrame()
{
    // 1. wait for previous submission
    vk::Result result = m_device.waitForFences(
        *m_inFlightFence,
        vk::True,
        UINT64_MAX);
    CheckResult(result, "Wait for fences failed");

    // 2. get first available swapchain image
    const auto [acquireResult, imageIndex] =
        m_swapchain.acquireNextImage(
            UINT64_MAX,
            *m_imageAvailableSemaphore);
    CheckResult(acquireResult,
                "Failed to acquire a swapchain image.",
                vk::Result::eSuccess, vk::Result::eSuboptimalKHR);

    // 3. record commands for the acquired image
    RecordClearCommandBuffer(imageIndex);

    // reset the fence only when we're about to submit work.
    m_device.resetFences(*m_inFlightFence);

    // 4. submit the command buffer
    const vk::Semaphore renderFinished =
        *m_renderFinishedSemaphores.at(imageIndex);


    const vk::SemaphoreSubmitInfo waitSemaphoreInfo{
        .semaphore = *m_imageAvailableSemaphore,
        .value = 0,
        .stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        .deviceIndex = 0
    };

    const vk::CommandBufferSubmitInfo commandBufferSubmitInfo
    {
        .commandBuffer = *m_commandBuffer,
        .deviceMask = 0
    };

    const vk::SemaphoreSubmitInfo signalSemaphoreInfo{
        .semaphore = *m_renderFinishedSemaphores.at(imageIndex),
        .value = 0,
        .stageMask = vk::PipelineStageFlagBits2::eAllCommands,
        .deviceIndex = 0
    };

    const vk::SubmitInfo2 submitInfo{
        .waitSemaphoreInfoCount = 1,
        .pWaitSemaphoreInfos = &waitSemaphoreInfo,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &commandBufferSubmitInfo,
        .signalSemaphoreInfoCount = 1,
        .pSignalSemaphoreInfos = &signalSemaphoreInfo
    };

    m_graphicsQueue.submit2(
        submitInfo,
        *m_inFlightFence);

    // 5. present the rendered image
    const vk::SwapchainKHR swapchain = *m_swapchain;

    const vk::PresentInfoKHR presentInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &renderFinished,
        .swapchainCount = 1,
        .pSwapchains = &swapchain,
        .pImageIndices = &imageIndex
    };

    const auto presentResult = m_graphicsQueue.presentKHR(presentInfo);

    CheckResult(presentResult,
                "Swapchain presentation failed.",
                vk::Result::eSuccess);
}

void SandboxRenderer::CreateGraphicsPipeline()
{
    // TODO: move this
    // The entry-point names must match those emitted by Slang.
    const std::array shaderStages
    {
        vk::PipelineShaderStageCreateInfo
        {
            .stage = vk::ShaderStageFlagBits::eVertex,
            .module = *m_vertexShaderModule,
            .pName = "vertexMain"
        },
        vk::PipelineShaderStageCreateInfo
        {
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = *m_fragmentShaderModule,
            .pName = "fragmentMain"
        }
    };

    // Viewport and scissor are dynamic pipeline state
    constexpr std::array dynamicStates
    {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    const vk::PipelineDynamicStateCreateInfo dynamicStateInfo
    {
        .dynamicStateCount = static_cast<std::uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()
    };

    // Not using vertex buffers or vertex attributes
    const vk::PipelineVertexInputStateCreateInfo vertexInputInfo
    {
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = nullptr,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = nullptr
    };

    // Interpret every group of three vertices as a triangle.
    const vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo
    {
        .topology = vk::PrimitiveTopology::eTriangleList,
        .primitiveRestartEnable = vk::False
    };

    // This is set when recording commands, so no need to set now
    const vk::PipelineViewportStateCreateInfo viewportStateInfo
    {
        .viewportCount = 1,
        .pViewports = nullptr,
        .scissorCount = 1,
        .pScissors = nullptr
    };

    const vk::PipelineMultisampleStateCreateInfo multisampleInfo
    {
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = vk::False,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = vk::False,
        .alphaToOneEnable = vk::False
    };

    const vk::PipelineRasterizationStateCreateInfo rasterizationInfo{
        .depthClampEnable = vk::False,
        .rasterizerDiscardEnable = vk::False,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eClockwise,
        .depthBiasEnable = vk::False,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f
    };

    // Write RGBA to the attachment without blending
    const vk::PipelineColorBlendAttachmentState colorBlendAttachment
    {
        .blendEnable = vk::True,
        .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
        .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
        .colorBlendOp = vk::BlendOp::eAdd,
        .srcAlphaBlendFactor = vk::BlendFactor::eOne,
        .dstAlphaBlendFactor = vk::BlendFactor::eZero,
        .alphaBlendOp = vk::BlendOp::eAdd,
        .colorWriteMask =
        vk::ColorComponentFlagBits::eR |
        vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA
    };

    const vk::PipelineColorBlendStateCreateInfo colorBlendInfo
    {
        .logicOpEnable = vk::False,
        .logicOp = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment
    };

    // pipeline expects one color attachment and no depth or stencil attachment yet
    const vk::Format colorFormat = m_swapchainSurfaceFormat.format;

    const vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateChain{
        {
            .stageCount = static_cast<std::uint32_t>(shaderStages.size()),
            .pStages = shaderStages.data(),
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &inputAssemblyInfo,
            .pTessellationState = nullptr,
            .pViewportState = &viewportStateInfo,
            .pRasterizationState = &rasterizationInfo,
            .pMultisampleState = &multisampleInfo,
            .pDepthStencilState = nullptr,
            .pColorBlendState = &colorBlendInfo,
            .pDynamicState = &dynamicStateInfo,
        },
        {
            .viewMask = 0,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &colorFormat,
            .depthAttachmentFormat = vk::Format::eUndefined,
            .stencilAttachmentFormat = vk::Format::eUndefined
        }
    };
    const vk::PipelineRenderingCreateInfo renderingInfo
    {

    };

    const vk::GraphicsPipelineCreateInfo createInfo
    {
        .pNext = &renderingInfo,
        .stageCount =
        static_cast<std::uint32_t>(shaderStages.size()),
        .pStages = shaderStages.data(),
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssemblyInfo,
        .pTessellationState = nullptr,
        .pViewportState = &viewportStateInfo,
        .pRasterizationState = &rasterizationInfo,
        .pMultisampleState = &multisampleInfo,
        .pDepthStencilState = nullptr,
        .pColorBlendState = &colorBlendInfo,
        .pDynamicState = &dynamicStateInfo,

        .layout = *m_pipelineLayout,
        .renderPass = vk::RenderPass{},
        .subpass = 0,
        .basePipelineHandle = vk::Pipeline{},
        .basePipelineIndex = -1
    };

    m_graphicsPipeline = vk::raii::Pipeline(m_device, nullptr, createInfo);
}

void SandboxRenderer::CreateInstance()
{
    // Required version for dynamic rendering
    const std::uint32_t apiVersion = m_context.enumerateInstanceVersion();

    if (apiVersion < MINIMUM_API_VERSION)
    {
        throw std::runtime_error(
            "Vulkan 1.3 or newer is required. "
            "Context reports older API version.");
    }

    constexpr vk::ApplicationInfo appInfo
    {
        .pApplicationName = "Sandbox App",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "SandboxEngine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = vk::ApiVersion13
    };

    std::vector<const char*> requiredLayers;
    if constexpr (enableValidationLayers)
    {
        requiredLayers.assign(m_validationLayers.begin(), m_validationLayers.end());
    }

    // Check if the required layers are supported by the Vulkan implementation.
    auto layerProperties = m_context.enumerateInstanceLayerProperties();
    auto unsupportedLayerIt = std::ranges::find_if(requiredLayers,
                                                   [&layerProperties](auto const& requiredLayer)
                                                   {
                                                       return std::ranges::none_of(layerProperties,
                                                           [requiredLayer](auto const& layerProperty)
                                                           {
                                                               return strcmp(layerProperty.layerName, requiredLayer) ==
                                                                   0;
                                                           });
                                                   });
    if (unsupportedLayerIt != requiredLayers.end())
    {
        throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayerIt));
    }

    // Ask SDL for required Vulkan extensions
    std::vector<const char*> requiredExtensions = SandboxSDLVulkan::GetRequiredInstanceExtensions();
    if (enableValidationLayers)
    {
        requiredExtensions.push_back(vk::EXTDebugUtilsExtensionName);
    }

    // See which extensions are available
    const auto availableExtensions = m_context.enumerateInstanceExtensionProperties();

    // Make sure all required are available
    for (const char* required : requiredExtensions)
    {
        const bool supported = std::ranges::any_of(availableExtensions,
                                                   [required](
                                                   const vk::ExtensionProperties& extension)
                                                   {
                                                       return std::strcmp(
                                                           extension.extensionName,
                                                           required) == 0;
                                                   });

        if (!supported)
        {
            throw std::runtime_error(
                std::string("Required Vulkan instance extension "
                    "is not supported: ") + required);
        }
    }

    const vk::InstanceCreateInfo createInfo{
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<std::uint32_t>(requiredLayers.size()),
        .ppEnabledLayerNames = requiredLayers.data(),
        .enabledExtensionCount = static_cast<std::uint32_t>(requiredExtensions.size()),
        .ppEnabledExtensionNames = requiredExtensions.data()
    };

    m_instance = vk::raii::Instance(m_context, createInfo);
}

void SandboxRenderer::SetupDebugMessenger()
{
    if constexpr (!enableValidationLayers)
        return;

    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
    vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{
        .messageSeverity = severityFlags,
        .messageType = messageTypeFlags,
        .pfnUserCallback = &DebugCallback
    };
    m_debugMessenger = m_instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
}

void SandboxRenderer::CreateSurface()
{
    VkSurfaceKHR rawSurface;

    try
    {
        rawSurface = SandboxSDLVulkan::CreateSurface(m_windowRef, *m_instance);
    }
    catch (const std::exception& e)
    {
        std::println("{} threw while creating raw surface: {}", __func__, e.what());
        throw;
    }

    // give ownership of surface to owned raii surface
    m_surface = vk::raii::SurfaceKHR(m_instance, rawSurface);
}

bool SandboxRenderer::IsDeviceSuitable(const vk::raii::PhysicalDevice& candidate)
{
    const vk::PhysicalDeviceProperties properties = candidate.getProperties();
    const char* deviceName = properties.deviceName.data();

    std::println("Checking GPU: {}", deviceName);

    bool supportsVulkanVersion = properties.apiVersion >= MINIMUM_API_VERSION;

    auto queueFamilies = candidate.getQueueFamilyProperties();
    bool supportsGraphics = std::ranges::any_of(queueFamilies, [](auto const& qfp)
    {
        return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics);
    });

    auto availableDeviceExtensions = candidate.enumerateDeviceExtensionProperties();
    bool supportsRequiredExtensions =
        std::ranges::all_of(m_requiredDeviceExtensions,
                            [&availableDeviceExtensions](auto const& requiredExtension)
                            {
                                return std::ranges::any_of(
                                    availableDeviceExtensions,
                                    [requiredExtension](auto const& availableDeviceExtension)
                                    {
                                        return std::strcmp(availableDeviceExtension.extensionName, requiredExtension) ==
                                            0;
                                    });
                            });

    auto features = candidate.template getFeatures2<vk::PhysicalDeviceFeatures2,
                                                    vk::PhysicalDeviceVulkan11Features,
                                                    vk::PhysicalDeviceVulkan13Features,
                                                    vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

    bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
        features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
        features.template get<vk::PhysicalDeviceVulkan13Features>().synchronization2 &&
        features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

    // returns true if all criteria are supported
    return supportsGraphics && supportsVulkanVersion && supportsRequiredExtensions && supportsRequiredFeatures;
}

void SandboxRenderer::SelectPhysicalDevice()
{
    std::vector<vk::raii::PhysicalDevice> physicalDevices = m_instance.enumeratePhysicalDevices();
    auto const devIter = std::ranges::find_if(
        physicalDevices.begin(), physicalDevices.end(),
        [this](auto const& device)
        {
            return IsDeviceSuitable(device);
        });

    if (devIter == physicalDevices.end())
        throw std::runtime_error("Failed to find a GPU with the required features.");

    m_physicalDevice = std::move(*devIter);
}

void SandboxRenderer::CreateLogicalDevice()
{
    // pick first available queue family
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = m_physicalDevice.getQueueFamilyProperties();

    auto graphicsQueueFamilyProperty = std::ranges::find_if(queueFamilyProperties, [](auto const& qfp)
    {
        return (qfp.queueFlags & vk::QueueFlagBits::eGraphics)
            != static_cast<vk::QueueFlags>(0);
    });
    assert(graphicsQueueFamilyProperty != queueFamilyProperties.end() && "No graphics queue family found");

    m_graphicsPresentQueueFamily = static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(),
                                                                       graphicsQueueFamilyProperty));

    // query for 1.3 features
    vk::StructureChain<vk::PhysicalDeviceFeatures2,
                       vk::PhysicalDeviceVulkan11Features,
                       vk::PhysicalDeviceVulkan13Features,
                       vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
        featureChain = {
            {}, // vk::PhysicalDeviceFeatures2
            {.shaderDrawParameters = vk::True}, // vk::PhysicalDeviceVulkan11Features
            {
                .synchronization2 = vk::True,
                .dynamicRendering = vk::True
            }, // vk::PhysicalDeviceVulkan13Features
            {.extendedDynamicState = vk::True} // vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
        };

    constexpr float queuePriority = 1.0f;

    vk::DeviceQueueCreateInfo queueCreateInfo
    {
        .queueFamilyIndex = m_graphicsPresentQueueFamily,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority
    };

    vk::DeviceCreateInfo deviceCreateInfo
    {
        .pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCreateInfo,
        .enabledExtensionCount = static_cast<std::uint32_t>(m_requiredDeviceExtensions.size()),
        .ppEnabledExtensionNames = m_requiredDeviceExtensions.data(),
    };

    m_device = vk::raii::Device{m_physicalDevice, deviceCreateInfo};

    // retrieve first queue from device queue family
    m_graphicsQueue = vk::raii::Queue(m_device, m_graphicsPresentQueueFamily, 0);
}

uint32_t SandboxRenderer::ChooseSwapchainMinImageCount(const vk::SurfaceCapabilitiesKHR& capabilities)
{
    auto minImageCount = std::max(3u, capabilities.minImageCount);
    if ((0 < capabilities.maxImageCount) && capabilities.maxImageCount < minImageCount)
    {
        minImageCount = capabilities.maxImageCount;
    }
    return minImageCount;
}

vk::SurfaceFormatKHR SandboxRenderer::ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
    assert(!availableFormats.empty());

    const auto formatIt = std::ranges::find_if(
        availableFormats,
        [](const auto& format)
        {
            return format.format == vk::Format::eB8G8R8A8Srgb
                && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
        });

    return formatIt != availableFormats.end() ? *formatIt : availableFormats.front();
}

vk::PresentModeKHR SandboxRenderer::ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availableModes)
{
    // ensure at least one mode supports FIFO
    assert(std::ranges::any_of(availableModes,
        [](auto presentMode)
        {
        return presentMode == vk::PresentModeKHR::eFifo;
        }));

    return std::ranges::any_of(availableModes,
                               [](const vk::PresentModeKHR value)
                               {
                                   return vk::PresentModeKHR::eMailbox == value;
                               })
               ? vk::PresentModeKHR::eMailbox
               : vk::PresentModeKHR::eFifo;
}

vk::Extent2D SandboxRenderer::ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
{
    // the surface specifies the size for us
    if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max())
    {
        return capabilities.currentExtent;
    }

    // query window size
    int width, height;

    if (!SDL_GetWindowSizeInPixels(m_windowRef, &width, &height))
    {
        throw std::runtime_error(
            std::string("SDL_GetWindowSizeInPixels failed: ") +
            SDL_GetError());
    }

    if (width <= 0 || height <= 0)
    {
        throw std::runtime_error(
            "Cannot create a swapchain for a zero-sized window.");
    }

    // clamp window size to surface capabilities
    return vk::Extent2D{
        .width = std::clamp(
            static_cast<std::uint32_t>(width),
            capabilities.minImageExtent.width,
            capabilities.maxImageExtent.width),

        .height = std::clamp(
            static_cast<std::uint32_t>(height),
            capabilities.minImageExtent.height,
            capabilities.maxImageExtent.height)
    };
}

void SandboxRenderer::CreateSwapchain()
{
    // query device for surface capabilities
    auto capabilities = m_physicalDevice.getSurfaceCapabilitiesKHR(*m_surface);
    m_swapchainExtent = ChooseSwapExtent(capabilities);
    std::uint32_t minImageCount = ChooseSwapchainMinImageCount(capabilities);

    // 2. Query supported formats and choose one.
    const auto formats = m_physicalDevice.getSurfaceFormatsKHR(*m_surface);
    m_swapchainSurfaceFormat = ChooseSwapSurfaceFormat(formats);

    // 3. Query presentation modes
    const auto availablePresentModes = m_physicalDevice.getSurfacePresentModesKHR(*m_surface);
    vk::PresentModeKHR presentMode = ChooseSwapPresentMode(availablePresentModes);

    vk::SwapchainCreateInfoKHR swapChainCreateInfo{
        .surface = *m_surface,
        .minImageCount = minImageCount,
        .imageFormat = m_swapchainSurfaceFormat.format,
        .imageColorSpace = m_swapchainSurfaceFormat.colorSpace,
        .imageExtent = m_swapchainExtent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
        .presentMode = presentMode,
        .clipped = true
    };

    // 7. Create the RAII swapchain.
    m_swapchain = vk::raii::SwapchainKHR(m_device, swapChainCreateInfo);
    m_swapchainImages = m_swapchain.getImages();

    std::println(
        "Swapchain created: {} images, extents {}x{}",
        m_swapchainImages.size(),
        m_swapchainExtent.width,
        m_swapchainExtent.height);
}

void SandboxRenderer::CreateImageViews()
{
    assert(!m_swapchainImages.empty());

    m_swapchainImageViews.reserve(m_swapchainImages.size());

    vk::ImageViewCreateInfo info
    {
        .image = VK_NULL_HANDLE, // we assign this in the loop
        .viewType = vk::ImageViewType::e2D,
        .format = m_swapchainSurfaceFormat.format,
        .subresourceRange = vk::ImageSubresourceRange
        {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    for (const auto& image : m_swapchainImages)
    {
        info.image = image;
        m_swapchainImageViews.emplace_back(m_device, info);
    }

    std::println("Created {} swapchain image views.", m_swapchainImageViews.size());
}

void SandboxRenderer::CreateCommandResources()
{
    const vk::CommandPoolCreateInfo poolInfo{
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = m_graphicsPresentQueueFamily
    };

    m_commandPool = vk::raii::CommandPool(m_device, poolInfo);

    const vk::CommandBufferAllocateInfo allocateInfo{
        .commandPool = *m_commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };

    vk::raii::CommandBuffers commandBuffers{
        m_device, allocateInfo
    };

    m_commandBuffer = std::move(commandBuffers.front());
}

void SandboxRenderer::RecordClearCommandBuffer(std::uint32_t imageIndex) const
{
    if (imageIndex >= m_swapchainImages.size() ||
        imageIndex >= m_swapchainImageViews.size())
        throw std::runtime_error("Invalid swapchain image index.");

    // DrawFrame waits for previous submission's frame first
    m_commandBuffer.reset();

    // prepare new command buffer submission
    constexpr vk::CommandBufferBeginInfo beginInfo{
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    };

    m_commandBuffer.begin(beginInfo);

    // Prepare to draw to current swapchain image and set up synchronization
    const vk::Image image = m_swapchainImages[imageIndex];

    constexpr vk::ImageSubresourceRange colorRange{
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
    };

    const vk::ImageMemoryBarrier2 toColorAttachment{
        .srcStageMask = vk::PipelineStageFlagBits2::eNone,
        .srcAccessMask = {},
        .dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        .dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
        .oldLayout = vk::ImageLayout::eUndefined, // Discard image's previous contents
        .newLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = colorRange
    };

    const vk::DependencyInfo toColorDependency{
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &toColorAttachment
    };

    m_commandBuffer.pipelineBarrier2(toColorDependency);

    // Prepare begin rendering
    const vk::ClearValue clearColor = m_defaultClearColor;

    const vk::RenderingAttachmentInfo colorAttachment{
        .imageView = *m_swapchainImageViews[imageIndex],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clearColor
    };

    const vk::RenderingInfo renderingInfo{
        .renderArea = vk::Rect2D{
            .offset = {.x = 0, .y = 0},
            .extent = m_swapchainExtent
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachment
    };

    m_commandBuffer.beginRendering(renderingInfo);

    const vk::Viewport viewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(m_swapchainExtent.width),
        .height = static_cast<float>(m_swapchainExtent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    const vk::Rect2D scissor{
        .offset = {.x = 0, .y = 0},
        .extent = m_swapchainExtent
    };

    m_commandBuffer.bindPipeline(
        vk::PipelineBindPoint::eGraphics,
        *m_graphicsPipeline);

    m_commandBuffer.setViewport(0, viewport);
    m_commandBuffer.setScissor(0, scissor);

    // Three vertices, one instance
    m_commandBuffer.draw(3, 1, 0, 0);

    m_commandBuffer.endRendering();

    // prepare image for presentation
    const vk::ImageMemoryBarrier2 toPresent{
        .srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        .srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
        .dstStageMask = vk::PipelineStageFlagBits2::eNone,
        .dstAccessMask = {},
        .oldLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .newLayout = vk::ImageLayout::ePresentSrcKHR,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = colorRange
    };

    const vk::DependencyInfo toPresentDependency{
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &toPresent
    };

    m_commandBuffer.pipelineBarrier2(toPresentDependency);
    m_commandBuffer.end();
}

void SandboxRenderer::CreateSyncObjects()
{
    const vk::SemaphoreCreateInfo semaphoreInfo{};

    // Signaled when a swapchain image is acquired.
    m_imageAvailableSemaphore =
        vk::raii::Semaphore(m_device, semaphoreInfo);

    // One presentation-wait semaphore per swapchain image.
    m_renderFinishedSemaphores.reserve(m_swapchainImages.size());

    for (std::size_t i = 0; i < m_swapchainImages.size(); ++i)
    {
        m_renderFinishedSemaphores.emplace_back(
            m_device, semaphoreInfo);
    }

    // Start signaled so the first frame doesn't wait forever.
    const vk::FenceCreateInfo fenceInfo{
        .flags = vk::FenceCreateFlagBits::eSignaled
    };

    m_inFlightFence = vk::raii::Fence(m_device, fenceInfo);
}

void SandboxRenderer::CreatePipelineLayout()
{
    constexpr vk::PipelineLayoutCreateInfo createInfo{
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr
    };

    m_pipelineLayout = vk::raii::PipelineLayout(m_device, createInfo);
}

void SandboxRenderer::CreateAllocator()
{
    const vma::AllocatorCreateInfo createInfo{
        .flags = {},
        .physicalDevice = *m_physicalDevice
    };

    m_allocator = vma::raii::Allocator{
        m_instance,
        m_device,
        createInfo
    };
}
