#pragma once
#include "Context.hpp"
#include "PreparedFrame.hpp"
#include "RenderDevice.hpp"

class TextPass
{
public:
    TextPass(
        const Droplet::Graphics::Vulkan::Context& p_context,
        const Droplet::Graphics::Vulkan::RenderDevice& p_device.
        uint32_t p_framesInFlight
    );
    
    void BeginFrame(uint32_t frameSlot, const Droplet::Graphics::Render::PreparedFrame& frame);
    
    void Record(
        vk::raii::CommandBuffer p_cmd,
        vk::Extent2D p_extent,
        const Droplet::Graphics::Render::DrawList& p_view,
        uint32_t p_viewIndex,
        const Droplet::Graphics::Vulkan::PipelineLibrary& p_pipelines
    );
    
    Droplet::Graphics::Vulkan::GraphicsPipelineDesc MakePipelineDesc(
        Droplet::GUID p_vertexShader,
        Droplet::GUID p_fragmentShader,
        vk::Format colorFormat,
        vk::Format depthFormat
    ) const;
    
private:
    
};
