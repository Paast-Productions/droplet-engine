#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <vector>

#include "Camera.hpp"
#include "RenderTarget.hpp"

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
    };

    namespace VK
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
        RenderTarget renderTarget {};
        std::int32_t priority = 0;
        std::vector<RenderData> renderData {};
        bool debug = false;
        glm::vec4 clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    };

}

