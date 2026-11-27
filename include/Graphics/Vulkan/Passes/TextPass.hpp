#pragma once
#include "Context.hpp"
#include "PreparedFrame.hpp"
#include "RenderDevice.hpp"

namespace Droplet::Graphics::Vulkan
{
    class TextPass
    {
    public:
        static constexpr GUID SHADER_ID = GuidFromName("shader/built-in-slug-text");
    
        [[nodiscard]] static Common::ShaderDesc ShaderAsset();
    
        TextPass(
            const Context& p_context,
            RenderDevice& p_device,
            uint32_t p_framesInFlight
        );
    
        [[nodiscard]] GraphicsPipelineDesc MakePipelineDesc(
            vk::Format p_colorFormat,
            vk::Format p_depthFormat
        ) const;
        
        void BeginFrame(uint32_t p_frameSlot, const Render::PreparedFrame& p_frame);
    
        void Record(
            vk::CommandBuffer p_cmd,
            vk::Extent2D p_extent,
            const Render::DrawList& p_view,
            uint32_t p_viewIndex,
            const PipelineLibrary& p_pipelines
        );
    
    private:
        const Context* m_context = nullptr;
        GpuResourceManager* m_gpu = nullptr;
        TextureTable* m_textures = nullptr;
        
        vk::raii::DescriptorSetLayout m_viewLayout{nullptr};
        vk::raii::DescriptorSetLayout m_glyphLayout{nullptr};
        vk::raii::PipelineLayout m_pipelineLayout{nullptr};
        
        vk::raii::DescriptorPool m_descriptorPool{nullptr};
        
        std::vector<vk::raii::DescriptorSet> m_viewSets;
        std::vector<vk::raii::DescriptorSet> m_glyphSets;
        
        std::vector<Buffer> m_viewBuffers;
        std::vector<Buffer> m_glyphBuffers;
        
        vk::DeviceSize m_viewStride = 0;
        uint32_t m_frameSlot = 0;
    };
}
