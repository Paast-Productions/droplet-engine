#pragma once

#include <glm/glm.hpp>

#include "Graphics/Common/Camera.hpp"
#include <Graphics/Common/RenderTargetDesc.hpp>

namespace Droplet::Graphics
{
    struct PushConstantData
    {
        glm::mat4 world{};
        float cameraDir[4]{};
        float cameraPos[4]{};
        glm::float32_t time{ 0 };
        std::uint32_t textureID{ 0 };
    };

    struct ShaderData
    {
        std::uint64_t GUID {0};
    };
    
    struct PipelineData
    {
        ShaderData vsData {};
        ShaderData tcsData {};
        ShaderData tesData {};
        ShaderData gsData {};
        ShaderData fsData {};
    };

    struct MeshData
    {
        //Extra mesh rendering options could possibly be added here

        std::uint64_t GUID {0};
    };

    struct TextureData
    {
        //Texture sampling options could be added here
        std::uint64_t GUID {0};
    };
    
    // TEMP TEMP TEMP TEMP
    /// Describes a render target the camera would output to
    struct TargetData
    {
        //TODO fill in
    };
    // TEMP TEMP TEMP TEMP
    
    struct RenderData
    {
        ShaderData vsData {};
        ShaderData tcsData {};
        ShaderData tesData {};
        ShaderData gsData {};
        ShaderData fsData {};
        
        MeshData meshData{};
        TextureData texData{};

        glm::mat4 transform{};


        // Settings
		bool transparent { false };
       
        // HACK: (RENDERER) Sort based on transparency
        bool operator<(const RenderData &p_other) const
        {
            return transparent < p_other.transparent;
        }
    };

    namespace Vulkan
    {
        struct ViewProjectionBuffer
        {
            glm::mat4 view{};
            glm::mat4 proj{};
        };
    }
    
    struct CameraRenderData
    {
        Camera camera {};
        Common::RenderTargetDesc renderTarget {};
        std::int32_t priority {};
        std::vector<RenderData> renderData {};
        glm::vec4 clearColor { glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) };
		
        // DEBUG
        bool debug = false;
    };
}

