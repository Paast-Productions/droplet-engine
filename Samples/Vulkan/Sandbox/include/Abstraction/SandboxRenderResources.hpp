#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace Droplet::Sandbox
{
    struct Vertex;
}

namespace Droplet::Sandbox::Vulkan
{
    class SandboxRenderResources
    {
    public:

        void CreateVertexBuffer();
        void CreateIndexBuffer();
        void CreateUniformBuffers();
        void UpdateUniformBuffer(uint32_t currentImage) const;

        void CreateDescriptorPool();
        void CreateDescriptorSets();
    private:
        std::vector<Vertex> m_vertices;
        std::vector<uint16_t> m_indices;

        vk::raii::Buffer m_vertexBuffer{nullptr};
        vk::raii::Buffer m_indexBuffer{nullptr};

        std::vector<vk::raii::Buffer> m_uniformBuffers;
        std::vector<vk::raii::DeviceMemory> m_uniformBuffersMemory;
        std::vector<void*> m_uniformBuffersMapped;

        vk::raii::DescriptorPool m_descriptorPool{nullptr};
        std::vector<vk::raii::DescriptorSet> m_descriptorSets;
    };
}
