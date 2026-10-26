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
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "SandboxGeometry.hpp"

using namespace Droplet::Sandbox;

using Droplet::Sandbox::VulkanUtils::CheckResult;

namespace
{
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
    // 1. Vulkan instance
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
    CreateDescriptorSetLayout();

    // 4. Command pool
    CreateCommandPool();

    // 5. Buffers
    CreateVertexBuffer();
    CreateIndexBuffer();
    CreateUniformBuffers();
    CreateDescriptorPool();
    CreateDescriptorSets();
    CreateCommandBuffers();

    CreateSyncObjects();

    // 6. Pipeline layout TODO: move
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

void SandboxRenderer::WaitIdle()
{
    m_device.waitIdle();
}


void SandboxRenderer::DrawFrame()
{
    // 1. wait for previous submission
    vk::Result fenceResult = m_device.waitForFences(
        *m_inFlightFences[m_frameIndex],
        vk::True,
        UINT64_MAX);
    CheckResult(fenceResult, "Wait for fences failed");

    // 2. get first available swapchain image
    auto [result, imageIndex] =
        m_swapchain.acquireNextImage(
            UINT64_MAX,
            *m_presentCompleteSemaphores[m_frameIndex]);
    if (result == vk::Result::eErrorOutOfDateKHR || m_frameBufferResized)
    {
        m_frameBufferResized = false;
        RecreateSwapchain();
        // Try again next frame
        return;
    }
    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
    {
        assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
        VulkanUtils::ThrowResult(result, "Failed to acquire a swapchain image");
    }
    UpdateUniformBuffer(m_frameIndex);

    // Reset fences for the current frame
    m_device.resetFences(*m_inFlightFences[m_frameIndex]);

    // 3. record commands for the acquired image
    RecordCommandBuffer(imageIndex);

    m_graphicsQueue.waitIdle();

    vk::PipelineStageFlags waitDestinationStageMask{vk::PipelineStageFlagBits::eColorAttachmentOutput};

    vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*m_presentCompleteSemaphores[m_frameIndex],
        .pWaitDstStageMask = &waitDestinationStageMask,
        .commandBufferCount = 1,
        .pCommandBuffers = &*m_commandBuffers[m_frameIndex],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &*m_renderFinishedSemaphores[m_frameIndex]
    };
    m_graphicsQueue.submit(submitInfo, *m_inFlightFences[m_frameIndex]);

    const vk::PresentInfoKHR presentInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &*m_renderFinishedSemaphores[m_frameIndex],
        .swapchainCount = 1,
        .pSwapchains = &*m_swapchain,
        .pImageIndices = &imageIndex,
    };

    result = m_graphicsQueue.presentKHR(presentInfo);
    CheckResult(result, "Presentation failed", vk::Result::eSuccess, vk::Result::eSuboptimalKHR);
    if (result == vk::Result::eSuboptimalKHR)
    {
        RecreateSwapchain();
    }

    // always increment frame index
    IncrementFrameIndex();
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
            .pName = "vMain"
        },
        vk::PipelineShaderStageCreateInfo
        {
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = *m_fragmentShaderModule,
            .pName = "fMain"
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

    auto bindingDescription = Vertex::GetBindingDescription();
    auto attributeDescription = Vertex::GetAttributeDescription();

    const vk::PipelineVertexInputStateCreateInfo vertexInputInfo
    {
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size()),
        .pVertexAttributeDescriptions = attributeDescription.data()
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
        .frontFace = vk::FrontFace::eCounterClockwise, // this is needed because we flip the viewport y
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
            .layout = *m_pipelineLayout,
            .renderPass = nullptr
        },
        {
            .viewMask = 0,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &colorFormat,
            .depthAttachmentFormat = vk::Format::eUndefined,
            .stencilAttachmentFormat = vk::Format::eUndefined
        }
    };

    m_graphicsPipeline = vk::raii::Pipeline(m_device, nullptr,
                                            pipelineCreateChain.get<vk::GraphicsPipelineCreateInfo>());
}

std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> SandboxRenderer::CreateBuffer(
    vk::DeviceSize size,
    vk::BufferUsageFlags usage,
    vk::MemoryPropertyFlags properties) const
{
    vk::BufferCreateInfo bufferInfo{.size = size, .usage = usage, .sharingMode = vk::SharingMode::eExclusive};
    auto buffer = vk::raii::Buffer(m_device, bufferInfo);
    vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
    vk::MemoryAllocateInfo allocInfo{
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties)
    };
    vk::raii::DeviceMemory bufferMemory = vk::raii::DeviceMemory(m_device, allocInfo);
    buffer.bindMemory(*bufferMemory, 0);
    return {std::move(buffer), std::move(bufferMemory)};
}

void SandboxRenderer::CopyBuffer(vk::raii::Buffer& srcBuffer, vk::raii::Buffer& dstBuffer, vk::DeviceSize size) const
{
    vk::CommandBufferAllocateInfo allocInfo{
        .commandPool = m_commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = 1
    };
    vk::raii::CommandBuffer commandCopyBuffer = std::move(m_device.allocateCommandBuffers(allocInfo).front());
    commandCopyBuffer.begin({.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    commandCopyBuffer.copyBuffer(srcBuffer, dstBuffer, vk::BufferCopy{.srcOffset = 0, .dstOffset = 0, .size = size});
    commandCopyBuffer.end();
    m_graphicsQueue.submit(vk::SubmitInfo{.commandBufferCount = 1, .pCommandBuffers = &*commandCopyBuffer}, nullptr);
    m_graphicsQueue.waitIdle();
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

vk::Extent2D SandboxRenderer::ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const
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

void SandboxRenderer::CreateDescriptorSetLayout()
{
    vk::DescriptorSetLayoutBinding uboLayoutBinding = {
        .binding = 0,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eVertex
    };
    vk::DescriptorSetLayoutCreateInfo layoutInfo{
        .bindingCount = 1,
        .pBindings = &uboLayoutBinding
    };
    m_descriptorSetLayout = vk::raii::DescriptorSetLayout(m_device, layoutInfo);
}

void SandboxRenderer::CreateCommandPool()
{
    const vk::CommandPoolCreateInfo poolInfo
    {
        .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
        .queueFamilyIndex = m_graphicsPresentQueueFamily
    };

    m_commandPool = vk::raii::CommandPool(m_device, poolInfo);
}

void SandboxRenderer::CreateCommandBuffers()
{
    const vk::CommandBufferAllocateInfo allocateInfo{
        .commandPool = *m_commandPool,
        .level = vk::CommandBufferLevel::ePrimary,
        .commandBufferCount = MAX_FRAMES_IN_FLIGHT
    };

    m_commandBuffers = vk::raii::CommandBuffers(m_device, allocateInfo);
}

void SandboxRenderer::TransitionImageLayout(
    uint32_t imageIndex,
    vk::ImageLayout old_layout,
    vk::ImageLayout new_layout,
    vk::AccessFlags2 src_access_mask,
    vk::AccessFlags2 dst_access_mask,
    vk::PipelineStageFlags2 src_stage_mask,
    vk::PipelineStageFlags2 dst_stage_mask) const
{
    auto& commandBuffer = m_commandBuffers[m_frameIndex];
    vk::ImageMemoryBarrier2 barrier = {
        .srcStageMask = src_stage_mask,
        .srcAccessMask = src_access_mask,
        .dstStageMask = dst_stage_mask,
        .dstAccessMask = dst_access_mask,
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = m_swapchainImages.at(imageIndex),
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };
    vk::DependencyInfo dependency_info = {
        .dependencyFlags = {},
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier
    };

    commandBuffer.pipelineBarrier2(dependency_info);
}

void SandboxRenderer::RecordCommandBuffer(std::uint32_t imageIndex) const
{
    auto& commandBuffer = m_commandBuffers[m_frameIndex];
    commandBuffer.begin({});

    TransitionImageLayout(
        imageIndex,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        {},
        vk::AccessFlagBits2::eColorAttachmentWrite,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        vk::PipelineStageFlagBits2::eColorAttachmentOutput);

    // set up up color attachment
    const vk::ClearValue clearColor = m_defaultClearColor;
    vk::RenderingAttachmentInfo attachmentInfo = {
        .imageView = m_swapchainImageViews[imageIndex],
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = clearColor
    };

    const vk::RenderingInfo renderingInfo{
        .renderArea = {.offset = {.x = 0, .y = 0}, .extent = m_swapchainExtent},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &attachmentInfo
    };

    commandBuffer.beginRendering(renderingInfo);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_graphicsPipeline);

    // viewport and scissors are dynamic so they need to be set every time
    const vk::Viewport viewport{
        .x = 0.0f,
        .y = static_cast<float>(m_swapchainExtent.height), // move the offset to account for the flipped Y
        .width = static_cast<float>(m_swapchainExtent.width),
        .height = -static_cast<float>(m_swapchainExtent.height), // negative to flip the Y coordinate for Vulkan
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    const vk::Rect2D scissor{
        .offset = {.x = 0, .y = 0},
        .extent = m_swapchainExtent
    };

    commandBuffer.setViewport(0, viewport);
    commandBuffer.setScissor(0, scissor);

    commandBuffer.bindVertexBuffers(0, *m_vertexBuffer, {0});
    commandBuffer.bindIndexBuffer(*m_indexBuffer, 0, vk::IndexType::eUint16);

    // set descriptor set
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout, 0,
                                     *m_descriptorSets[m_frameIndex], nullptr);

    commandBuffer.drawIndexed(static_cast<uint32_t>(m_indices.size()), 1, 0, 0, 0);
    commandBuffer.endRendering();

    // transition image to present
    TransitionImageLayout(
        imageIndex,
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        vk::AccessFlagBits2::eColorAttachmentWrite, // srcAccessMask
        {}, // dstAccessMask
        vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
        vk::PipelineStageFlagBits2::eBottomOfPipe // dstStage
    );

    commandBuffer.end();
}

void SandboxRenderer::CreateSyncObjects()
{
    assert(m_presentCompleteSemaphores.empty() && m_renderFinishedSemaphores.empty() && m_inFlightFences.empty());

    for (std::size_t i = 0; i < m_swapchainImages.size(); i++)
    {
        m_renderFinishedSemaphores.emplace_back(m_device, vk::SemaphoreCreateInfo{});
    }

    for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        m_presentCompleteSemaphores.emplace_back(m_device, vk::SemaphoreCreateInfo{});
        m_inFlightFences.emplace_back(m_device, vk::FenceCreateInfo{
                                          .flags = vk::FenceCreateFlagBits::eSignaled
                                      });
    }
}

void SandboxRenderer::CreatePipelineLayout()
{
    const vk::PipelineLayoutCreateInfo createInfo
    {
        .setLayoutCount = 1,
        .pSetLayouts = &*m_descriptorSetLayout,
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

void SandboxRenderer::CreateVertexBuffer()
{
    const vk::DeviceSize bufferSize = sizeof(m_vertices[0]) * m_vertices.size();

    auto [stagingBuffer, stagingBufferMemory] = CreateBuffer(bufferSize,
                                                             vk::BufferUsageFlagBits::eTransferSrc,
                                                             vk::MemoryPropertyFlagBits::eHostVisible |
                                                             vk::MemoryPropertyFlagBits::eHostCoherent);


    void* data = stagingBufferMemory.mapMemory(0, bufferSize);
    memcpy(data, m_vertices.data(), bufferSize);
    stagingBufferMemory.unmapMemory();

    std::tie(m_vertexBuffer, m_vertexBufferMemory) = CreateBuffer(bufferSize,
                                                                  vk::BufferUsageFlagBits::eVertexBuffer |
                                                                  vk::BufferUsageFlagBits::eTransferDst,
                                                                  vk::MemoryPropertyFlagBits::eDeviceLocal);

    CopyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);
}

void SandboxRenderer::CreateIndexBuffer()
{
    vk::DeviceSize bufferSize = sizeof(m_indices.front()) * m_indices.size();

    auto [stagingBuffer, stagingBufferMemory] =
        CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
                     vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

    void* data = stagingBufferMemory.mapMemory(0, bufferSize);
    memcpy(data, m_indices.data(), static_cast<size_t>(bufferSize));
    stagingBufferMemory.unmapMemory();

    std::tie(m_indexBuffer, m_indexBufferMemory) = CreateBuffer(bufferSize,
                                                                vk::BufferUsageFlagBits::eIndexBuffer |
                                                                vk::BufferUsageFlagBits::eTransferDst,
                                                                vk::MemoryPropertyFlagBits::eDeviceLocal);

    CopyBuffer(stagingBuffer, m_indexBuffer, bufferSize);
}

void SandboxRenderer::CreateUniformBuffers()
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vk::DeviceSize bufferSize = sizeof(MVPUniformData);
        auto [buffer, bufferMem] = CreateBuffer(
            bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
        m_uniformBuffers.emplace_back(std::move(buffer));
        m_uniformBuffersMemory.emplace_back(std::move(bufferMem));
        m_uniformBuffersMapped.emplace_back(m_uniformBuffersMemory.back().mapMemory(0, bufferSize));
    }
}

void SandboxRenderer::UpdateUniformBuffer(uint32_t currentImage) const
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    MVPUniformData ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f),
                                static_cast<float>(m_swapchainExtent.width) / static_cast<float>(m_swapchainExtent.
                                    height), 0.1f,
                                10.0f);
    memcpy(m_uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void SandboxRenderer::CreateDescriptorPool()
{
    vk::DescriptorPoolSize poolSize = {
        .type = vk::DescriptorType::eUniformBuffer,
        .descriptorCount = MAX_FRAMES_IN_FLIGHT
    };

    vk::DescriptorPoolCreateInfo poolInfo = {
        .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
        .maxSets = MAX_FRAMES_IN_FLIGHT,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize
    };

    m_descriptorPool = vk::raii::DescriptorPool{m_device, poolInfo};
}

void SandboxRenderer::CreateDescriptorSets()
{
    std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *m_descriptorSetLayout);
    vk::DescriptorSetAllocateInfo allocInfo = {
        .descriptorPool = m_descriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
        .pSetLayouts = layouts.data()
    };

    m_descriptorSets = vk::raii::DescriptorSets{m_device, allocInfo};

    // populate
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vk::DescriptorBufferInfo bufferInfo = {
            .buffer = m_uniformBuffers[i],
            .offset = 0,
            .range = sizeof(MVPUniformData)
        };
        vk::WriteDescriptorSet descriptorWrite{
            .dstSet = m_descriptorSets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = vk::DescriptorType::eUniformBuffer,
            .pBufferInfo = &bufferInfo
        };
        m_device.updateDescriptorSets(descriptorWrite, {});
    }
}

uint32_t SandboxRenderer::FindMemoryType(uint32_t typeFiller, vk::MemoryPropertyFlags properties) const
{
    vk::PhysicalDeviceMemoryProperties memProperties = m_physicalDevice.getMemoryProperties();

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFiller) & (1 << i) && memProperties.memoryTypes[i].propertyFlags & properties)
        {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type.");
}

void SandboxRenderer::CleanupSwapchain()
{
    m_swapchainImageViews.clear();
    m_swapchain = nullptr;
}

void SandboxRenderer::RecreateSwapchain()
{
    // TODO: solve minimized
    // int width = 0, height = 0;
    // SDL_GetWindowSizeInPixels(m_windowRef, &width, &height);
    // while ((width == 0 || height == 0)/* && check if window is closing*/) 
    // {
    //     SDL_GetWindowSizeInPixels(m_windowRef, &width, &height);
    // }
    //
    WaitIdle();

    CleanupSwapchain();

    CreateSwapchain();
    CreateImageViews();
}
