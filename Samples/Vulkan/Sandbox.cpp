/**
 * @file Sandbox.cpp
 * @author Sebastian Lorensson
 * @date 2026-10-16
 * @brief Sandbox main for testing Vulkan functionality such as synchronisation with pipeline barriers, events, fences and timeline semaphores.
 */
#include "Sandbox.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#define VULKAN_HPP_NO_UNION_CONSTRUCTORS 
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc_raii.hpp>

#undef VULKAN_HPP_NO_UNION_CONSTRUCTORS
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS



#include <vector>
#include <print>
#include <algorithm>
#include <string>
#include <cstring>
#include <utility>

using namespace Droplet;
using namespace Droplet::Sandbox;

// global variables

class SandboxRenderer
{
public:
    SandboxRenderer(SDL_Window* window)
        :m_windowRef{ window }
    {

        CreateVulkanInstance();
        CreateVulkanSurface();
        CreateAllocator();

        SelectPhysicalDevice();
        CreateLogicalDevice();

        CreateSwapchain();
        CreateImageViews();

        CreateRenderPass();
        CreateFramebuffers();
        CreateCommandResources();
        CreateSyncObjects();

        CreatePipelineLayout();
    }
    ~SandboxRenderer() {}


    void CreateShaderModules(
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


    void DrawFrame()
    {
        // 1. Wait until the previous submission has finished.
        vk::Result result = m_device.waitForFences(
            *m_inFlightFence,
            VK_TRUE,
            UINT64_MAX);

        if (result != vk::Result::eSuccess)
            throw std::runtime_error("Wait for fences failed");

        // 2. Acquire an available swapchain image.
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

        // 3. Record commands for the acquired image.
        RecordClearCommandBuffer(imageIndex);

        // Reset the fence only when we're about to submit work.
        m_device.resetFences(*m_inFlightFence);

        // 4. Submit the command buffer.
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

        // 5. Present the rendered image.
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


    void CreateGraphicsPipeline()
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


private:

    void CreateVulkanInstance()
    {
        ::Uint32 sdlExtensionCount = 0;
        const char* const* sdlExtensions =
            ::SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount);

        if (!sdlExtensions)
        {
            throw std::runtime_error(
                std::string("SDL_Vulkan_GetInstanceExtensions failed: ")
                + SDL_GetError());
        }

        std::vector<const char*> extensions(
            sdlExtensions,
            sdlExtensions + sdlExtensionCount);

        constexpr vk::ApplicationInfo appInfo{
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


    void CreateVulkanSurface()
    {
        VkSurfaceKHR rawSurface = VK_NULL_HANDLE;

        if (!SDL_Vulkan_CreateSurface(
            m_windowRef, *m_instance, nullptr, &rawSurface))
        {
            throw std::runtime_error(
                std::string("SDL_Vulkan_CreateSurface failed: ")
                + SDL_GetError());
        }

        // give ownership of surface to owned raii surface
        m_surface = vk::raii::SurfaceKHR(m_instance, rawSurface);
    }

    /// @brief Iterate over all available physical devices (GPUs) and find one that supports all required properties
    void SelectPhysicalDevice()
    {
        std::vector<vk::raii::PhysicalDevice> devices = m_instance.enumeratePhysicalDevices();

        for (auto& device : devices)
        {
            const vk::PhysicalDeviceProperties properties = device.getProperties();

            auto deviceName = properties.deviceName.data();

            std::println("GPU: {}", deviceName);

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
                continue;


            // Make sure the device has valid formats and modes
            if (device.getSurfaceFormatsKHR(*m_surface).empty() ||
                device.getSurfacePresentModesKHR(*m_surface).empty())
            {
                continue;
            }

            // Find a queue family that supports graphics and presentation.
            const std::vector<vk::QueueFamilyProperties> familyProps = device.getQueueFamilyProperties();

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
                    m_graphicsPresentQueueFamily = i;
                    m_physicalDevice = std::move(device);

                    std::println("Selected physical device {}", deviceName);

                    return;
                }
            }

        }

        throw std::runtime_error("Failed to find a suitable GPU for rendering.");
    }

    
    void CreateLogicalDevice()
    {
        constexpr float queuePriority = 1.0f;

        vk::DeviceQueueCreateInfo queueInfo{
            .queueFamilyIndex = m_graphicsPresentQueueFamily,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        };

        // TODO: make match Graphics::Renderer 
        constexpr vk::PhysicalDeviceFeatures features{};

        // swapchain is non-optional
        constexpr const char* requiredExtensions[] =
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        vk::DeviceCreateInfo deviceInfo
        {
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &queueInfo,
            .enabledExtensionCount = static_cast<std::uint32_t>(std::size(requiredExtensions)),
            .ppEnabledExtensionNames = requiredExtensions,
            .pEnabledFeatures = &features
        };

        // creates logical device
        m_device = vk::raii::Device{ m_physicalDevice, deviceInfo };

        // retrieve queue 0 from queue family
        m_graphicsPresentQueue = m_device.getQueue(m_graphicsPresentQueueFamily, 0);
    }

    
    vk::SurfaceFormatKHR ChooseSurfaceFormat(
        const std::vector<vk::SurfaceFormatKHR>& formats)
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

    
    vk::Extent2D ChooseSwapExtent(
        const vk::SurfaceCapabilitiesKHR& capabilities)
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

    
    void CreateSwapchain()
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


    void CreateImageViews()
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


    void CreateRenderPass()
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


    void CreateFramebuffers()
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


    void CreateCommandResources()
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


    void RecordClearCommandBuffer(std::uint32_t imageIndex) const
    {
        if (imageIndex >= m_swapchainFramebuffers.size())
            throw std::runtime_error("Invalid swapchain image index.");

        m_commandBuffer.reset();

        const vk::CommandBufferBeginInfo beginInfo{
            .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
        };

        m_commandBuffer.begin(beginInfo);

        const vk::ClearValue clearValue
        {
            .color = {
                .float32 = std::array{ 0.2f, 0.2f, 0.4f, 1.0f }
            }
        };

        const vk::Rect2D renderArea{
            .offset = {.x = 0, .y = 0 },
            .extent = m_swapchainExtent
        };

        const vk::RenderPassBeginInfo renderPassInfo{
            .renderPass = *m_renderPass,
            .framebuffer = *m_swapchainFramebuffers[imageIndex],
            .renderArea = renderArea,
            .clearValueCount = 1,
            .pClearValues = &clearValue
        };

        m_commandBuffer.beginRenderPass(
            renderPassInfo,
            vk::SubpassContents::eInline);

        // 
        const vk::Viewport viewport{
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(m_swapchainExtent.width),
            .height = static_cast<float>(m_swapchainExtent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };

        const vk::Rect2D scissor{
            .offset = {.x = 0, .y = 0 },
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


    void CreateSyncObjects()
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


    void CreatePipelineLayout()
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

    void CreateAllocator()
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

private:
    static constexpr std::uint32_t maxFramesInFlight{ 2 };

    // dependency order top to bottom
    vk::raii::Context m_context;
    vk::raii::Instance m_instance{ nullptr };
    vk::raii::SurfaceKHR m_surface{ nullptr };

    // == Device ==
    vk::raii::PhysicalDevice m_physicalDevice{ nullptr };
    std::uint32_t m_graphicsPresentQueueFamily = 0;

    vk::raii::Device m_device{ nullptr };   ///< Logical device
    vk::raii::Queue m_graphicsPresentQueue{ nullptr };

    // == Swapchain ==
    vk::raii::SwapchainKHR m_swapchain{ nullptr };
    std::vector<vk::Image> m_swapchainImages; // non-owning handles to swapchain images
    std::vector<vk::raii::ImageView> m_swapchainImageViews;
    vk::SurfaceFormatKHR m_swapchainSurfaceFormat{};
    vk::Extent2D m_swapchainExtent{};

    // == Render Pass ==
    vk::raii::RenderPass m_renderPass{ nullptr };

    // == Frame Buffers ==
    std::vector<vk::raii::Framebuffer> m_swapchainFramebuffers;

    // == Command Pool ==
    vk::raii::CommandPool m_commandPool{ nullptr };
    vk::raii::CommandBuffer m_commandBuffer{ nullptr };

    // == Synchronization resources ==
    vk::raii::Semaphore m_imageAvailableSemaphore{ nullptr };
    std::vector<vk::raii::Semaphore> m_renderFinishedSemaphores;
    vk::raii::Fence m_inFlightFence{ nullptr };


    // == Shaders ==
    // move these
    vk::raii::ShaderModule m_vertexShaderModule{ nullptr };
    vk::raii::ShaderModule m_fragmentShaderModule{ nullptr };

    // == Pipeline == 
    vk::raii::PipelineLayout m_pipelineLayout{ nullptr };
    vk::raii::Pipeline m_graphicsPipeline{ nullptr };

    // == Vma Allocator == 
    vma::raii::Allocator m_allocator{nullptr};

    // keeping a reference to the window instance for convenience
    SDL_Window* m_windowRef{ nullptr };
};


/// Main entry point
int main(void)
{
    // initialize SDL state and create window
    SandboxSDL sdl{ SandboxSDL::Config{} };

    // initialize renderer
    SandboxRenderer renderer{ sdl.GetWindow() };

    // load shaders
    SandboxShaderCompiler shaderCompiler;

    const auto vertexSpirv = shaderCompiler.CompileSpirv(
        "triangle",
        "sandbox_shader.slang",
        "vertexMain");

    const auto fragmentSpirv = shaderCompiler.CompileSpirv(
        "triangle",
        "sandbox_shader.slang",
        "fragmentMain");

    renderer.CreateShaderModules(vertexSpirv, fragmentSpirv);
    renderer.CreateGraphicsPipeline();

    // main runtime loop 
    bool running = true;
    while (running)
    {
        sdl.PollEvents(&running);

        // TODO: create some situations that require synchronization

        // TODO: draw
        renderer.DrawFrame();
    }


}