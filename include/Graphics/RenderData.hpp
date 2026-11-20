#pragma once

#include <glm/glm.hpp>

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

    // struct TextData
    // {
    //     std::uint64_t GUID {0};
    //     
    //     std::shared_ptr<const std::vector<GlyphInstance>> glyphInstances;
    // };
    
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
       
        // HACK: Sort based on transparency
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

}

