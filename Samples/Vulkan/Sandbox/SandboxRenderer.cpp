/**
 * @file Sandbox.cpp
 * @author Sebastian Lorensson
 * @date 2026-10-16
 * @brief Sandbox main for testing Vulkan functionality such as synchronisation with pipeline barriers, events, fences and timeline semaphores.
 */
#include "Sandbox.hpp"

#include "SandboxSDLVulkan.hpp"

#include <Core/FileUtils.hpp>


#include <vector>
#include <print>
#include <algorithm>
#include <string>
#include <cstring>
#include <utility>

using namespace Droplet::Sandbox;

// =====================================================================
// Sandbox Renderer
// =====================================================================
SandboxRenderer::SandboxRenderer(SDL_Window* window)
    : m_windowRef{window}
{
    // 1. Vulkan instance and window as surface
    CreateInstance();
    CreateSurface();

    // 2. Find a GPU and query its capabilities, then create a device interface
    SelectPhysicalDevice();
    CreateLogicalDevice();

    // VMA Allocator depends on device
    CreateAllocator();

    // 3. Swapchain and views to it
    CreateSwapchain();
    CreateImageViews();

    // 4. Create a render pass, frame buffers for the swapchain, command queue and fence + semaphore
    CreateRenderPass();
    CreateFramebuffers();
    CreateCommandResources();
    CreateSyncObjects();

    // 5. Pipeline layout TODO: move
    CreatePipelineLayout();
}

SandboxRenderer::~SandboxRenderer()
{
}

void SandboxRenderer::CreateShaderModules(
    const std::vector<std::uint32_t>& vertexSpirv,
    const std::vector<std::uint32_t>& fragmentSpirv)
{
    const vk::ShaderModuleCreateInfo vertexInfo{
        .codeSize =
        vertexSpirv.size() * sizeof(std::uint32_t),
        .pCode = vertexSpirv.data()
    };

    const vk::ShaderModuleCreateInfo fragmentInfo{
        .codeSize =
        fragmentSpirv.size() * sizeof(std::uint32_t),
        .pCode = fragmentSpirv.data()
    };

    m_vertexShaderModule =
        vk::raii::ShaderModule(m_device, vertexInfo);

    m_fragmentShaderModule =
        vk::raii::ShaderModule(m_device, fragmentInfo);
}

void SandboxRenderer::DrawFrame()
{
    // 1. wait for previous submission
    vk::Result result = m_device.waitForFences(
        *m_inFlightFence,
        VK_TRUE,
        UINT64_MAX);

    if (result != vk::Result::eSuccess)
        throw std::runtime_error("Wait for fences failed");

    // 2. get first available swapchain image
    const auto [acquireResult, imageIndex] =
        m_swapchain.acquireNextImage(
            UINT64_MAX,
            *m_imageAvailableSemaphore);

    if (acquireResult != vk::Result::eSuccess &&
        acquireResult != vk::Result::eSuboptimalKHR)
    {
        throw std::runtime_error(
            "Failed to acquire a swapchain image.");
    }

    // 3. record commands for the acquired image
    RecordClearCommandBuffer(imageIndex);

    // reset the fence only when we're about to submit work.
    m_device.resetFences(*m_inFlightFence);

    // 4. submit the command buffer
    const vk::Semaphore imageAvailable =
        *m_imageAvailableSemaphore;

    const vk::Semaphore renderFinished =
        *m_renderFinishedSemaphores.at(imageIndex);

    const vk::CommandBuffer commandBuffer =
        *m_commandBuffer;

    const vk::PipelineStageFlags waitStage{
        vk::PipelineStageFlagBits::eColorAttachmentOutput
    };

    const vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &imageAvailable,
        .pWaitDstStageMask = &waitStage,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &renderFinished
    };

    m_graphicsPresentQueue.submit(
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

    const auto presentResult =
        m_graphicsPresentQueue.presentKHR(presentInfo);

    if (presentResult != vk::Result::eSuccess)
    {
        throw std::runtime_error(
            "Swapchain presentation needs attention; "
            "swapchain recreation is not implemented yet.");
    }
}

void SandboxRenderer::CreateGraphicsPipeline()
{
    // The entry-point names must match those emitted by Slang.
    const std::array shaderStages{
        vk::PipelineShaderStageCreateInfo{
            .stage = vk::ShaderStageFlagBits::eVertex,
            .module = *m_vertexShaderModule,
            .pName = "vertexMain"
        },
        vk::PipelineShaderStageCreateInfo{
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = *m_fragmentShaderModule,
            .pName = "fragmentMain"
        }
    };

    // No vertex buffers or vertex attributes.
    const vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = nullptr,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = nullptr
    };

    // Interpret every group of three vertices as a triangle.
    const vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo{
        .topology = vk::PrimitiveTopology::eTriangleList,
        .primitiveRestartEnable = VK_FALSE
    };

    // We set the viewport and scissor when recording commands.
    const vk::PipelineViewportStateCreateInfo viewportStateInfo{
        .viewportCount = 1,
        .pViewports = nullptr,
        .scissorCount = 1,
        .pScissors = nullptr
    };

    const vk::PipelineRasterizationStateCreateInfo rasterizationInfo{
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eNone,
        .frontFace = vk::FrontFace::eCounterClockwise,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f
    };

    const vk::PipelineMultisampleStateCreateInfo multisampleInfo{
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE
    };

    // Write RGBA to the attachment without blending.
    const vk::PipelineColorBlendAttachmentState colorBlendAttachment{
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = vk::BlendFactor::eOne,
        .dstColorBlendFactor = vk::BlendFactor::eZero,
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

    const vk::PipelineColorBlendStateCreateInfo colorBlendInfo{
        .logicOpEnable = VK_FALSE,
        .logicOp = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment
    };

    // Viewport and scissor are dynamic pipeline state.
    const std::array dynamicStates{
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    const vk::PipelineDynamicStateCreateInfo dynamicStateInfo{
        .dynamicStateCount =
        static_cast<std::uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()
    };

    const vk::GraphicsPipelineCreateInfo createInfo{
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
        .renderPass = *m_renderPass,
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

    // Ask SDL for required Vulkan extensions
    std::vector<const char*> extensions = SandboxSDLVulkan::GetRequiredInstanceExtensions();

    // See which extensions are available
    const auto availableExtensions = m_context.enumerateInstanceExtensionProperties();

    // Make sure all required 
    for (const char* required : extensions)
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

    constexpr vk::ApplicationInfo appInfo
    {
        .pApplicationName = "Sandbox App",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "SandboxEngine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = vk::ApiVersion14
    };

    const vk::InstanceCreateInfo createInfo{
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = static_cast<std::uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data()
    };

    m_instance = vk::raii::Instance(m_context, createInfo);
}

void SandboxRenderer::CreateSurface()
{
    VkSurfaceKHR rawSurface;
    
    try
    {
        rawSurface = SandboxSDLVulkan::CreateSurface(m_windowRef, *m_instance);
    }
    catch(const std::exception& e)
    {
        std::println("{} threw while creating raw surface: {}", __func__, e.what());
        throw;
    }

    // if (!SDL_Vulkan_CreateSurface(
    //     m_windowRef, *m_instance, nullptr, &rawSurface))
    // {
    //     throw std::runtime_error(
    //         std::string("SDL_Vulkan_CreateSurface failed: ")
    //         + SDL_GetError());
    // }

    // give ownership of surface to owned raii surface
    m_surface = vk::raii::SurfaceKHR(m_instance, rawSurface);
}

void SandboxRenderer::SelectPhysicalDevice()
{
    std::vector<vk::raii::PhysicalDevice> devices = m_instance.enumeratePhysicalDevices();

    for (auto& device : devices)
    {
        const vk::PhysicalDeviceProperties properties = device.getProperties();
        auto deviceName = properties.deviceName.data();
        std::println("Checking GPU: {}", deviceName);

        // Require Vulkan 1.3 or newer.
        if (properties.apiVersion < MINIMUM_API_VERSION)
        {
            std::println(
                "  Rejected: Vulkan 1.3 required.");
            continue;
        }

        const auto extensions = device.enumerateDeviceExtensionProperties();

        const bool supportsSwapChain =
            std::ranges::any_of(extensions,
                                [](const vk::ExtensionProperties& extension)
                                {
                                    return std::strcmp(
                                        extension.extensionName,
                                        VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0;
                                });

        if (!supportsSwapChain)
        {
            std::println(
                "  Rejected: VK_KHR_swapchain unavailable.");
            continue;
        }

        const auto formats = device.getSurfaceFormatsKHR(m_surface);
        const auto presentModes = device.getSurfacePresentModesKHR(m_surface);


        // Make sure the device has valid formats and modes
        if (formats.empty() || presentModes.empty())
        {
            std::println(
                "  Rejected: no usable surface formats or present modes.");
            continue;
        }

        // Query version support without enabling features
        const auto supportedFeatures = device.getFeatures2<vk::PhysicalDeviceFeatures2,
                                                           vk::PhysicalDeviceVulkan13Features>();
        
        const auto features13 = supportedFeatures.get<vk::PhysicalDeviceVulkan13Features>();
        
        if (!features13.dynamicRendering)
        {
            std::println("  Rejected: dynamicRendering feature unsupported.");
            continue;
        }
        
        if (!features13.synchronization2)
        {
            std::println("  Rejected: synchronization2 feature unsupported.");
        }

        
        // Find a queue family that supports graphics and presentation.
        const std::vector<vk::QueueFamilyProperties> familyProps = device.getQueueFamilyProperties();
        
        bool foundQueueFamily = false;
        std::uint32_t selectedQueueFamily = 0;
        
        for (std::uint32_t i = 0; i < familyProps.size(); ++i)
        {
            if (familyProps[i].queueCount == 0)
                continue;

            const bool supportsGraphics =
                static_cast<bool>(
                    familyProps[i].queueFlags &
                    vk::QueueFlagBits::eGraphics);

            const bool supportsPresentation = device.getSurfaceSupportKHR(i, *m_surface);

            if (supportsGraphics && supportsPresentation)
            {
                selectedQueueFamily = i;
                foundQueueFamily = true;
                break;
            }
        }
        
        if (!foundQueueFamily)
        {
            std::println(" Rejected: No graphics/presentation queue family present.");
            continue;
        }
        
        m_graphicsPresentQueueFamily = selectedQueueFamily;
        m_physicalDevice = std::move(device);
        
        std::println("Selected GPU {}", deviceName);
        return;
    }

    throw std::runtime_error("Failed to find a suitable GPU for rendering.");
}

void SandboxRenderer::CreateLogicalDevice()
{
    constexpr float queuePriority = 1.0f;

    vk::DeviceQueueCreateInfo queueInfo
    {
        .queueFamilyIndex = m_graphicsPresentQueueFamily,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority
    };

    constexpr vk::PhysicalDeviceVulkan13Features vulkan13Features
    {
        .synchronization2 = VK_TRUE,
        .dynamicRendering = VK_TRUE,
    };
    
    // TODO: make match Graphics::Renderer
    // constexpr vk::PhysicalDeviceFeatures features{};

    // swapchain is non-optional
    constexpr const char* requiredExtensions[] =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    vk::DeviceCreateInfo deviceInfo
    {
        .pNext = &vulkan13Features,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueInfo,
        .enabledExtensionCount = static_cast<std::uint32_t>(std::size(requiredExtensions)),
        .ppEnabledExtensionNames = requiredExtensions,
        // .pEnabledFeatures = &features
    };

    // creates logical device
    m_device = vk::raii::Device{m_physicalDevice, deviceInfo};

    // retrieve queue 0 from queue family
    m_graphicsPresentQueue = m_device.getQueue(m_graphicsPresentQueueFamily, 0);
}

vk::SurfaceFormatKHR SandboxRenderer::ChooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats)
{
    if (formats.empty())
        throw std::runtime_error("No surface formats available.");

    const auto it = std::ranges::find_if(
        formats,
        [](const vk::SurfaceFormatKHR& format)
        {
            return format.format == vk::Format::eB8G8R8A8Srgb &&
                format.colorSpace ==
                vk::ColorSpaceKHR::eSrgbNonlinear;
        });

    if (it != formats.end())
        return *it;

    return formats.front();
}

vk::Extent2D SandboxRenderer::ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
{
    // the surface specifies the size for us
    if (capabilities.currentExtent.width !=
        std::numeric_limits<std::uint32_t>::max())
    {
        return capabilities.currentExtent;
    }

    int width = 0;
    int height = 0;

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
    // 1. Query the capabilities of this surface.
    const vk::SurfaceCapabilitiesKHR capabilities =
        m_physicalDevice.getSurfaceCapabilitiesKHR(*m_surface);

    // 2. Query supported formats and choose one.
    const std::vector<vk::SurfaceFormatKHR> formats =
        m_physicalDevice.getSurfaceFormatsKHR(*m_surface);

    m_swapchainSurfaceFormat = ChooseSurfaceFormat(formats);

    // 3. Query presentation modes
    const std::vector<vk::PresentModeKHR> presentModes =
        m_physicalDevice.getSurfacePresentModesKHR(*m_surface);

    if (std::ranges::find(
            presentModes, vk::PresentModeKHR::eFifo) ==
        presentModes.end())
    {
        throw std::runtime_error(
            "FIFO presentation mode is unavailable.");
    }

    // 4. Choose the image extent.
    m_swapchainExtent = ChooseSwapExtent(capabilities);

    // 5. Request one more image than the minimum, if supported.
    std::uint32_t imageCount = capabilities.minImageCount + 1;

    if (capabilities.maxImageCount != 0 &&
        imageCount > capabilities.maxImageCount)
    {
        imageCount = capabilities.maxImageCount;
    }

    // We intend to render directly into swapchain images
    if (!static_cast<bool>(
        capabilities.supportedUsageFlags &
        vk::ImageUsageFlagBits::eColorAttachment))
    {
        throw std::runtime_error(
            "Surface does not support color-attachment swapchain images.");
    }

    // Choose a supported composite-alpha mode.
    constexpr std::array alphaModes{
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        vk::CompositeAlphaFlagBitsKHR::ePreMultiplied,
        vk::CompositeAlphaFlagBitsKHR::ePostMultiplied,
        vk::CompositeAlphaFlagBitsKHR::eInherit
    };

    const auto alphaIt = std::ranges::find_if(
        alphaModes,
        [&](vk::CompositeAlphaFlagBitsKHR mode)
        {
            return static_cast<bool>(
                capabilities.supportedCompositeAlpha & mode);
        });

    if (alphaIt == alphaModes.end())
    {
        throw std::runtime_error(
            "No supported composite-alpha mode.");
    }

    // 6. Describe the swapchain
    vk::SwapchainCreateInfoKHR createInfo{
        .surface = *m_surface,
        .minImageCount = imageCount,
        .imageFormat = m_swapchainSurfaceFormat.format,
        .imageColorSpace = m_swapchainSurfaceFormat.colorSpace,
        .imageExtent = m_swapchainExtent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
        .imageSharingMode = vk::SharingMode::eExclusive, // graphics and presentation family is the same
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = *alphaIt,
        .presentMode = vk::PresentModeKHR::eFifo,
        .clipped = VK_TRUE
    };

    // 7. Create the RAII swapchain.
    m_swapchain = vk::raii::SwapchainKHR(m_device, createInfo);

    // 8. Retrieve the non-owning handles of its images.
    m_swapchainImages = m_swapchain.getImages();

    std::println(
        "Swapchain created: {} images, {}x{}",
        m_swapchainImages.size(),
        m_swapchainExtent.width,
        m_swapchainExtent.height);
}

void SandboxRenderer::CreateImageViews()
{
    m_swapchainImageViews.reserve(m_swapchainImages.size());

    for (const auto& image : m_swapchainImages)
    {
        vk::ImageViewCreateInfo info
        {
            .image = image,
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

        // create an owned raii view
        m_swapchainImageViews.emplace_back(m_device, info);
    }

    std::println("Created {} swapchain image views.", m_swapchainImageViews.size());
}

void SandboxRenderer::CreateRenderPass()
{
    const vk::AttachmentDescription colorAttachment{
        .format = m_swapchainSurfaceFormat.format,
        .samples = vk::SampleCountFlagBits::e1,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
        .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
        .initialLayout = vk::ImageLayout::eUndefined,
        .finalLayout = vk::ImageLayout::ePresentSrcKHR
    };

    const vk::AttachmentReference colorAttachmentRef{
        .attachment = 0,
        .layout = vk::ImageLayout::eColorAttachmentOptimal
    };

    const vk::SubpassDescription subpass{
        .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef
    };

    const vk::SubpassDependency dependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask =
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        .dstStageMask =
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        .srcAccessMask = {},
        .dstAccessMask =
        vk::AccessFlagBits::eColorAttachmentWrite
    };

    const vk::RenderPassCreateInfo createInfo{
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency
    };

    m_renderPass = vk::raii::RenderPass(m_device, createInfo);
}

void SandboxRenderer::CreateFramebuffers()
{
    m_swapchainFramebuffers.reserve(
        m_swapchainImageViews.size());

    for (const auto& imageView : m_swapchainImageViews)
    {
        const vk::ImageView attachment = *imageView;

        const vk::FramebufferCreateInfo createInfo{
            .renderPass = *m_renderPass,
            .attachmentCount = 1,
            .pAttachments = &attachment,
            .width = m_swapchainExtent.width,
            .height = m_swapchainExtent.height,
            .layers = 1
        };

        m_swapchainFramebuffers.emplace_back(
            m_device, createInfo);
    }

    std::println(
        "Created {} framebuffers.",
        m_swapchainFramebuffers.size());
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
    if (imageIndex >= m_swapchainFramebuffers.size())
        throw std::runtime_error("Invalid swapchain image index.");

    m_commandBuffer.reset();

    const vk::CommandBufferBeginInfo beginInfo{
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    };

    m_commandBuffer.begin(beginInfo);

    const vk::Rect2D renderArea{
        .offset = {.x = 0, .y = 0},
        .extent = m_swapchainExtent
    };

    const vk::RenderPassBeginInfo renderPassInfo{
        .renderPass = *m_renderPass,
        .framebuffer = *m_swapchainFramebuffers[imageIndex],
        .renderArea = renderArea,
        .clearValueCount = 1,
        .pClearValues = &m_defaultClearColor
    };

    m_commandBuffer.beginRenderPass(
        renderPassInfo,
        vk::SubpassContents::eInline);

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

    // Three vertices, one instance.
    m_commandBuffer.draw(3, 1, 0, 0);

    m_commandBuffer.endRenderPass();

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
    const vk::PipelineLayoutCreateInfo createInfo{
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr
    };

    m_pipelineLayout =
        vk::raii::PipelineLayout(m_device, createInfo);
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
