//#pragma once
//
//#include <vulkan/vulkan_raii.hpp>
//#include <ImGui/imgui.h>
//#include <glm/glm.hpp>
//
//#include <Graphics/VK/BufferHelper.hpp>
//#include <Graphics/VK/VertexBuffer.hpp>
//#include <Graphics/VK/IndexBuffer.hpp>
//#include <Graphics/VK/ImageView.hpp>
//#include <optional>
//#include <Graphics/VK/CommandPool.hpp>
//
//class ImGuiVulkanUtil
//{
//public:
//	ImGuiVulkanUtil();
//	~ImGuiVulkanUtil();
//
//public:
//    // Lifecycle management for proper resource initialization and cleanup
//    ImGuiVulkanUtil(vk::raii::Device &device, vk::raii::PhysicalDevice &physicalDevice,
//        vk::raii::Queue &graphicsQueue, uint32_t graphicsQueueFamily, Droplet::Graphics::VK::CommandPool &p_commandPool);
//    ~ImGuiVulkanUtil();
//
//    // Core functionality methods for ImGui integration
//    void Init(float width, float height);                   // Initialize ImGui context and configure display
//    void InitResources();                                    // Create all Vulkan resources for rendering
//    void SetStyle(uint32_t index);                          // Apply visual styling themes
//    void UpdateTexture(ImTextureData *tex);                 // Dynamically update/create textures (v1.92+)
//
//    // Frame-by-frame rendering operations
//    void NewFrame();                                         // Begin new ImGui frame and generate geometry
//    bool EndFrame();
//    void UpdateBuffers(
//        const vk::raii::Device &p_device,
//        const vk::raii::PhysicalDevice &p_physicalDevice,
//        const Droplet::Graphics::VK::CommandPool &p_commandPool,
//        const vk::raii::Queue &p_queue);                                    // Upload updated geometry to GPU buffers
//    void DrawFrame(vk::raii::CommandBuffer &commandBuffer); // Record rendering commands to command buffer
//
//    // Input event handling for interactive UI elements
//    void HandleKey(int key, int scancode, int action, int mods); // Process keyboard input events
//    void HandleMousePos(float x, float y);                  // Process mouse movement events (v1.87+)
//    void HandleMouseButton(int button, bool pressed);       // Process mouse button events (v1.87+)
//    bool GetWantKeyCapture();                               // Query if ImGui wants keyboard focus
//    void CharPressed(uint32_t key);                         // Handle character input for text widgets
//
//private:
//    // Core GPU rendering resources for UI display
//    // These objects form the foundation of our ImGui-to-Vulkan rendering pipeline
//    vk::raii::Sampler m_sampler{ nullptr };                  // Texture sampling configuration for font rendering
//    std::optional<Droplet::Graphics::VK::VertexBuffer> m_vertexBuffer;               // Dynamic vertex buffer for UI geometry
//    std::optional<Droplet::Graphics::VK::IndexBuffer> m_indexBuffer;                // Dynamic index buffer for UI triangle connectivity
//    uint32_t m_vertexCount = 0;                              // Current vertex count for draw commands
//    uint32_t m_indexCount = 0;                               // Current index count for draw commands
//
//    // GPU texture containing ImGui font atlas
//    // Shader-accessible view of font texture
//    std::optional<Droplet::Graphics::VK::ImageView> m_fontImageView;
//
//    // Vulkan pipeline infrastructure for UI rendering
//    // These objects define the complete GPU processing pipeline for ImGui elements
//    vk::raii::PipelineCache m_pipelineCache{ nullptr };        // Pipeline compilation cache for faster startup
//    vk::raii::PipelineLayout m_pipelineLayout{ nullptr };      // Resource binding layout (textures, uniforms)
//    vk::raii::Pipeline m_pipeline{ nullptr };                  // Complete graphics pipeline for UI rendering
//    vk::raii::DescriptorPool m_descriptorPool{ nullptr };      // Pool for allocating descriptor sets
//    vk::raii::DescriptorSetLayout m_descriptorSetLayout{ nullptr }; // Layout defining shader resource bindings
//    vk::raii::DescriptorSet m_descriptorSet{ nullptr };        // Actual resource bindings for font texture
//
//    // Vulkan device context and system integration
//    // These references connect our UI system to the broader Vulkan application context
//    vk::raii::Device *m_device = nullptr;                    // Primary Vulkan device for resource creation
//    vk::raii::PhysicalDevice *m_physicalDevice = nullptr;    // GPU hardware info for capability queries
//    vk::raii::Queue *m_graphicsQueue = nullptr;              // Command submission queue for UI rendering
//    uint32_t m_graphicsQueueFamily = 0;                      // Queue family index for validation
//
//    // UI state management and rendering configuration
//    // These members control the visual appearance and dynamic behavior of the UI system
//    ImGuiStyle m_vulkanStyle;                                // Custom visual styling for Vulkan applications
//
//    // Push constants for efficient per-frame parameter updates
//    // This structure enables fast updates of transformation and styling data
//    struct PushConstBlock {
//        glm::vec2 scale;                                     // UI scaling factors for different screen sizes
//        glm::vec2 translate;                                 // Translation offset for UI positioning
//    } m_pushConstBlock;
//
//    // Dynamic state tracking for performance optimization
//    bool m_needsUpdateBuffers = false;                       // Flag indicating buffer resize requirements
//
//    // Modern Vulkan rendering configuration
//    vk::PipelineRenderingCreateInfo m_renderingInfo{};       // Dynamic rendering setup parameters
//    vk::Format m_colorFormat = vk::Format::eB8G8R8A8Unorm;   // Target framebuffer format
//};
//
//ImGuiVulkanUtil::ImGuiVulkanUtil(vk::raii::Device &p_device, vk::raii::PhysicalDevice &p_physicalDevice,
//    vk::raii::Queue &p_graphicsQueue, uint32_t p_graphicsQueueFamily, Droplet::Graphics::VK::CommandPool &p_commandPool)
//    : m_device(&p_device), m_physicalDevice(&p_physicalDevice),
//    m_graphicsQueue(&p_graphicsQueue), m_graphicsQueueFamily(p_graphicsQueueFamily)
//    // Initialize buffers directly
//{
//    m_vertexBuffer.emplace(p_device, p_physicalDevice, p_commandPool, p_);
//    // Set up dynamic rendering info
//    m_renderingInfo.colorAttachmentCount = 1;
//    vk::Format formats[] = { m_colorFormat };
//    m_renderingInfo.pColorAttachmentFormats = &m_colorFormat;
//}
//
//ImGuiVulkanUtil::~ImGuiVulkanUtil() {
//    // Wait for device to finish operations before destroying resources
//    // NOTE: waitIdle() is acceptable in destructors/cleanup code but should NEVER be used
//    // in the main rendering loop as it causes severe performance issues. For frame
//    // synchronization, use fences and semaphores instead.
//    if (m_device) {
//        m_device->waitIdle();
//    }
//
//    // All resources are automatically cleaned up by their destructors
//    // No manual cleanup needed
//
//    // ImGui context is destroyed separately
//}
//
//void ImGuiVulkanUtil::Init(float width, float height) {
//    // Initialize ImGui context
//    IMGUI_CHECKVERSION();
//    ImGui::CreateContext();
//
//    // Configure ImGui
//    ImGuiIO &io = ImGui::GetIO();
//    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable keyboard controls
//    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable docking
//
//    // Inform ImGui that we support the new texture update protocol (v1.92+)
//    // This enables support for dynamic font textures and multiple texture atlases
//    io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
//
//    // Set display size
//    io.DisplaySize = ImVec2(width, height);
//    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
//
//    // Set up style
//    m_vulkanStyle = ImGui::GetStyle();
//    m_vulkanStyle.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
//    m_vulkanStyle.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
//    m_vulkanStyle.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
//    m_vulkanStyle.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
//    m_vulkanStyle.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
//
//    // Apply default style
//    SetStyle(0);
//}
//
//void ImGuiVulkanUtil::SetStyle(uint32_t index) {
//    ImGuiStyle &style = ImGui::GetStyle();
//
//    switch (index) {
//    case 0:
//        // Custom Vulkan style
//        style = m_vulkanStyle;
//        break;
//    case 1:
//        // Classic style
//        ImGui::StyleColorsClassic();
//        break;
//    case 2:
//        // Dark style
//        ImGui::StyleColorsDark();
//        break;
//    case 3:
//        // Light style
//        ImGui::StyleColorsLight();
//        break;
//    }
//}
//
//void ImGuiVulkanUtil::InitResources() {
//    // Configure texture sampling parameters for optimal text rendering
//    // These settings directly impact text quality and performance
//    vk::SamplerCreateInfo samplerInfo{};
//    samplerInfo.magFilter = vk::Filter::eLinear;                    // Smooth scaling when magnified
//    samplerInfo.minFilter = vk::Filter::eLinear;                    // Smooth scaling when minified
//    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;        // Smooth transitions between mip levels
//    samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;  // Prevent texture wrapping
//    samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;  // Clean edge handling
//    samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;  // 3D consistency
//    samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;   // White border for clamped areas
//
//    m_sampler = m_device->createSampler(samplerInfo);                   // Create the GPU sampler object
//
//    // Create descriptor pool for shader resource binding
//    // Descriptors provide the interface between shaders and GPU resources
//    vk::DescriptorPoolSize poolSize{ vk::DescriptorType::eCombinedImageSampler, 1 };
//
//    vk::DescriptorPoolCreateInfo poolInfo{};
//    poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;     // Allow individual descriptor set freeing
//    poolInfo.maxSets = 2;                                                      // Maximum number of descriptor sets
//    poolInfo.poolSizeCount = 1;                                                // Number of pool size specifications
//    poolInfo.pPoolSizes = &poolSize;                                           // Pool size configuration
//
//    m_descriptorPool = m_device->createDescriptorPool(poolInfo);                   // Create descriptor pool
//
//    // Create descriptor set layout defining shader resource interface
//    // This layout must match the binding declarations in the ImGui shaders
//    vk::DescriptorSetLayoutBinding binding{};
//    binding.descriptorType = vk::DescriptorType::eCombinedImageSampler;        // Combined texture and sampler
//    binding.descriptorCount = 1;                                               // Single texture binding
//    binding.stageFlags = vk::ShaderStageFlagBits::eFragment;                   // Used in fragment shader
//    binding.binding = 0;                                                       // Shader binding point 0
//
//    vk::DescriptorSetLayoutCreateInfo layoutInfo{};
//    layoutInfo.bindingCount = 1;                                               // Number of bindings in layout
//    layoutInfo.pBindings = &binding;                                           // Binding configuration array
//
//    m_descriptorSetLayout = m_device->createDescriptorSetLayout(layoutInfo);       // Create layout object
//
//    // Allocate descriptor set from pool using the defined layout
//    // This creates the actual binding that connects GPU resources to shaders
//    vk::DescriptorSetAllocateInfo allocInfo{};
//    allocInfo.descriptorPool = *m_descriptorPool;                                // Source pool for allocation
//    allocInfo.descriptorSetCount = 1;                                          // Number of sets to allocate
//    vk::DescriptorSetLayout layouts[] = { *m_descriptorSetLayout };                // Layout template array
//    allocInfo.pSetLayouts = layouts;                                           // Layout configuration
//
//    m_descriptorSet = std::move(m_device->allocateDescriptorSets(allocInfo).front()); // Allocate and store set
//
//    // Update descriptor set with actual font texture and sampler resources
//    // This final step connects the physical GPU resources to the shader binding points
//    vk::DescriptorImageInfo imageInfo{};
//    imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;           // Expected image layout
//    imageInfo.imageView = *m_fontImageView->GetView();                           // Font texture view
//    imageInfo.sampler = *m_sampler;                                              // Texture sampler
//
//    vk::WriteDescriptorSet writeSet{};
//    writeSet.dstSet = *m_descriptorSet;                                          // Target descriptor set
//    writeSet.descriptorCount = 1;                                              // Number of resources to bind
//    writeSet.descriptorType = vk::DescriptorType::eCombinedImageSampler;       // Resource type
//    writeSet.pImageInfo = &imageInfo;                                          // Image resource information
//    writeSet.dstBinding = 0;                                                   // Binding point in shader
//
//    m_device->updateDescriptorSets(writeSet, 0);                   // Execute the binding update
//
//    // Create pipeline cache
//    vk::PipelineCacheCreateInfo pipelineCacheInfo{};
//    m_pipelineCache = m_device->createPipelineCache(pipelineCacheInfo);
//
//    // Create pipeline layout
//    vk::PushConstantRange pushConstantRange{};
//    pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eVertex;
//    pushConstantRange.offset = 0;
//    pushConstantRange.size = sizeof(PushConstBlock);
//
//    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
//    pipelineLayoutInfo.setLayoutCount = 1;
//    vk::DescriptorSetLayout setLayouts[] = { *m_descriptorSetLayout };
//    pipelineLayoutInfo.pSetLayouts = setLayouts;
//    pipelineLayoutInfo.pushConstantRangeCount = 1;
//    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
//
//    m_pipelineLayout = m_device->createPipelineLayout(pipelineLayoutInfo);
//
//    // Create the graphics pipeline with dynamic rendering
//    // ... (shader loading, pipeline state setup, etc.)
//
//    // For brevity, we're omitting the full pipeline creation code here
//    // In a real implementation, you would:
//    // 1. Load the vertex and fragment shaders
//    // 2. Set up all the pipeline state (vertex input, input assembly, rasterization, etc.)
//    // 3. Include the renderingInfo in the pipeline creation to enable dynamic rendering
//}
//
//void ImGuiVulkanUtil::UpdateTexture(ImTextureData *tex) {
//    if (tex->Status == ImTextureStatus_WantCreate || tex->Status == ImTextureStatus_WantUpdates) {
//        int texWidth = tex->Width;
//        int texHeight = tex->Height;
//        unsigned char *fontData = (unsigned char *)tex->Pixels;
//
//        if (!fontData) return;
//
//        vk::DeviceSize uploadSize = texWidth * texHeight * tex->BytesPerPixel;
//        vk::Format format = (tex->BytesPerPixel == 4) ? vk::Format::eR8G8B8A8Unorm : vk::Format::eR8Unorm;
//
//        if (tex->Status == ImTextureStatus_WantCreate) {
//            // Create optimized GPU image for texture storage
//            vk::Extent3D extent{ static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1 };
//            m_fontImage = Image(*m_device, extent, format,
//                vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
//                vk::MemoryPropertyFlagBits::eDeviceLocal);
//
//            m_fontImageView = ImageView(*m_device, m_fontImage.getHandle(), format,
//                vk::ImageAspectFlagBits::eColor);
//        }
//
//        // Create staging buffer for efficient CPU-to-GPU data transfer
//        Buffer stagingBuffer(*device, uploadSize, vk::BufferUsageFlagBits::eTransferSrc,
//            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
//
//        // Copy data to staging buffer
//        void *data = stagingBuffer.map();
//        memcpy(data, fontData, uploadSize);
//        stagingBuffer.unmap();
//
//        // Transition image layout and copy data
//        TransitionImageLayout(fontImage.getHandle(), format,
//            vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
//        CopyBufferToImage(stagingBuffer.getHandle(), fontImage.getHandle(),
//            static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
//        TransitionImageLayout(fontImage.getHandle(), format,
//            vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
//
//        // Store descriptor set handle as the ImTextureID
//        // In this implementation, we use a single descriptor set for the font atlas
//        tex->SetTexID((ImTextureID)(intptr_t)(VkDescriptorSet)*m_descriptorSet);
//        tex->SetStatus(ImTextureStatus_OK);
//    }
//}
//
//void ImGuiVulkanUtil::NewFrame() 
//{
//    // Start a new ImGui frame
//    ImGui::NewFrame();
//
//    // Create your UI elements here
//    // For example:
//    ImGui::Begin("Vulkan ImGui Demo");
//    ImGui::Text("Hello, Vulkan!");
//    if (ImGui::Button("Click me!")) {
//        // Handle button click
//    }
//    ImGui::End();
//};
//
//bool ImGuiVulkanUtil::EndFrame()
//{
//    // End the frame
//    ImGui::EndFrame();
//
//    // Render to generate draw data
//    ImGui::Render();
//
//    // Check if buffers need updating
//    ImDrawData *drawData = ImGui::GetDrawData();
//    if (drawData && drawData->CmdListsCount > 0) {
//        if (drawData->TotalVtxCount > m_vertexCount || drawData->TotalIdxCount > m_indexCount) {
//            m_needsUpdateBuffers = true;
//            return true;
//        }
//    }
//
//    return false;
//}
//
//void ImGuiVulkanUtil::UpdateBuffers(
//    const vk::raii::Device &p_device, 
//    const vk::raii::PhysicalDevice &p_physicalDevice, 
//    const Droplet::Graphics::VK::CommandPool &p_commandPool,
//    const vk::raii::Queue &p_queue) 
//{
//    ImDrawData *drawData = ImGui::GetDrawData();
//    if (!drawData || drawData->CmdListsCount == 0) {
//        return;
//    }
//
//    // Calculate required buffer sizes
//    vk::DeviceSize vertexBufferSize = drawData->TotalVtxCount * sizeof(ImDrawVert);
//    vk::DeviceSize indexBufferSize = drawData->TotalIdxCount * sizeof(ImDrawIdx);
//
//    // Resize buffers if needed
//    if (drawData->TotalVtxCount > m_vertexCount) {
//        // Recreate vertex buffer with new size
//        m_vertexBuffer.reset();
//        m_vertexBuffer.emplace(p_device, m_physicalDevice, p_commandPool, p_queue, drawData, vertexBufferSize);
//        m_vertexCount = drawData->TotalVtxCount;
//    }
//
//    if (drawData->TotalIdxCount > m_indexCount) {
//        // Recreate index buffer with new size
//        m_indexBuffer = Buffer(*m_device, indexBufferSize,
//            vk::BufferUsageFlagBits::eIndexBuffer,
//            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
//        m_indexBuffer.emplace(p_device, p_physicalDevice, p_commandPool, p_queue, drawData.);
//        m_indexCount = drawData->TotalIdxCount;
//    }
//}
//
//void ImGuiVulkanUtil::DrawFrame(vk::raii::CommandBuffer &commandBuffer) {
//    ImDrawData *drawData = ImGui::GetDrawData();
//    if (!drawData || drawData->CmdListsCount == 0) {
//        return;
//    }
//
//    // Process dynamic texture updates (v1.92+ RendererHasTextures protocol)
//    // This handles font atlas regeneration and any user-provided textures
//    if (drawData->Textures) {
//        for (int n = 0; n < drawData->Textures->Size; n++) {
//            ImTextureData *tex = (*drawData->Textures)[n];
//            if (tex->Status != ImTextureStatus_OK) {
//                UpdateTexture(tex);
//            }
//        }
//    }
//
//    // Begin dynamic rendering
//    vk::RenderingAttachmentInfo colorAttachment{};
//    // Note: In a real implementation, you would set imageView, imageLayout,
//    // loadOp, storeOp, and clearValue based on your swapchain image
//
//    vk::RenderingInfo renderingInfo{};
//    renderingInfo.renderArea = vk::Rect2D{ {0, 0}, {static_cast<uint32_t>(drawData->DisplaySize.x),
//                                                   static_cast<uint32_t>(drawData->DisplaySize.y)} };
//    renderingInfo.layerCount = 1;
//    renderingInfo.colorAttachmentCount = 1;
//    renderingInfo.pColorAttachments = &colorAttachment;
//
//    commandBuffer.beginRendering(renderingInfo);
//
//    // Bind the pipeline used for ImGui
//    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_pipeline);
//
//    // Configure viewport for UI pixel coordinates
//    vk::Viewport viewport{};
//    viewport.width = drawData->DisplaySize.x;
//    viewport.height = drawData->DisplaySize.y;
//    viewport.minDepth = 0.0f;
//    viewport.maxDepth = 1.0f;
//    commandBuffer.setViewport(0, viewport);
//
//    // Convert from ImGui coordinates into NDC via a simple scale/translate
//    m_pushConstBlock.scale = glm::vec2(2.0f / drawData->DisplaySize.x, 2.0f / drawData->DisplaySize.y);
//    m_pushConstBlock.translate = glm::vec2(-1.0f);
//    commandBuffer.pushConstants(*m_pipelineLayout, vk::ShaderStageFlagBits::eVertex,
//        0, sizeof(PushConstBlock), &m_pushConstBlock);
//
//    // We already filled these buffers this frame
//    vk::Buffer vertexBuffers[] = { *m_vertexBuffer->GetVertexBuffer()};
//    vk::DeviceSize offsets[] = { 0 };
//    commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);
//    commandBuffer.bindIndexBuffer(*m_indexBuffer->GetIndexBuffer(), 0, vk::IndexType::eUint16);
//
//    int vertexOffset = 0;
//    int indexOffset = 0;
//
//    for (int i = 0; i < drawData->CmdListsCount; i++) {
//        const ImDrawList *cmdList = drawData->CmdLists[i];
//
//        for (int j = 0; j < cmdList->CmdBuffer.Size; j++) {
//            const ImDrawCmd *pcmd = &cmdList->CmdBuffer[j];
//
//            // Clip per draw call
//            vk::Rect2D scissor{};
//            scissor.offset.x = std::max(static_cast<int32_t>(pcmd->ClipRect.x), 0);
//            scissor.offset.y = std::max(static_cast<int32_t>(pcmd->ClipRect.y), 0);
//            scissor.extent.width = static_cast<uint32_t>(pcmd->ClipRect.z - pcmd->ClipRect.x);
//            scissor.extent.height = static_cast<uint32_t>(pcmd->ClipRect.w - pcmd->ClipRect.y);
//            commandBuffer.setScissor(0, scissor);
//
//            // Bind font (and any UI) textures for this draw
//            // The TexID now stores the actual descriptor set handle (VkDescriptorSet)
//            VkDescriptorSet texHandle = (VkDescriptorSet)pcmd->GetTexID();
//            if (texHandle) {
//                commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
//                    *m_pipelineLayout, 0, { vk::DescriptorSet(texHandle) }, {});
//            }
//            else {
//                // Fallback to default font if no specific texture is bound
//                commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
//                    *m_pipelineLayout, 0, { *m_descriptorSet }, {});
//            }
//
//            // Issue indexed draw for this UI batch
//            commandBuffer.drawIndexed(pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
//            indexOffset += pcmd->ElemCount;
//        }
//
//        vertexOffset += cmdList->VtxBuffer.Size;
//    }
//
//    // Close the rendering scope for the UI overlay
//    commandBuffer.endRendering();
//}
//
//void ImGuiVulkanUtil::HandleKey(int key, int scancode, int action, int mods) {
//    ImGuiIO &io = ImGui::GetIO();
//
//    // Map the platform-specific key action to a boolean state
//    // In GLFW: GLFW_RELEASE = 0, GLFW_PRESS = 1, GLFW_REPEAT = 2
//    // WE SHALL NOT USE GLFW, INSTEAD WE SHALL USE SDL3
//    bool pressed = (action != 0);
//
//    // Modern ImGui (v1.87+) uses AddKeyEvent to queue input events.
//    // This handles key states, modifiers, and repeat logic internally.
//    // Most backends can cast native key codes directly to ImGuiKey.
//    io.AddKeyEvent((ImGuiKey)key, pressed);
//}
//
//void ImGuiVulkanUtil::HandleMousePos(float x, float y) {
//    ImGuiIO &io = ImGui::GetIO();
//    // Modern event API for mouse position
//    io.AddMousePosEvent(x, y);
//}
//
//void ImGuiVulkanUtil::HandleMouseButton(int button, bool pressed) {
//    ImGuiIO &io = ImGui::GetIO();
//    // Modern event API for mouse buttons (0: Left, 1: Right, 2: Middle)
//    io.AddMouseButtonEvent(button, pressed);
//}
//
//bool ImGuiVulkanUtil::GetWantKeyCapture() {
//    return ImGui::GetIO().WantCaptureKeyboard;
//}
//
//void ImGuiVulkanUtil::CharPressed(uint32_t key) {
//    ImGuiIO &io = ImGui::GetIO();
//    io.AddInputCharacter(key);
//}