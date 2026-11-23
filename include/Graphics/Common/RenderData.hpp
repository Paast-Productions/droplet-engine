#pragma once

#include <glm/glm.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "Graphics/Common/Camera.hpp"
#include <Graphics/Common/RenderDesc.hpp>

namespace Droplet::Graphics
{
    struct PushConstantData
    {
        glm::mat4 world{};
        float cameraDir[4]{};
        float cameraPos[4]{};
        glm::float32_t time{0};
        std::uint32_t textureID{0};
    };

    struct ShaderData
    {
        std::uint64_t GUID{0};
    };

    struct PipelineData
    {
        ShaderData vsData{};
        ShaderData tcsData{};
        ShaderData tesData{};
        ShaderData gsData{};
        ShaderData fsData{};
    };

    struct MeshData
    {
        //Extra mesh rendering options could possibly be added here
        std::uint64_t GUID{0};
    };

    struct TextureData
    {
        //Texture sampling options could be added here
        std::uint64_t GUID{0};
        bool isRenderTarget{false};
    };

    // == Dynamic meshes ==

    /// @brief How one vertex attribute is stored
    enum class VertexFormat : std::uint8_t
    {
        Float1,
        Float2,
        Float3,
        Float4,
        UNorm8x4 ///< four bytes read as floats 0..1 (colours)
    };

    /// @brief How a mesh's vertices or indices form primitives
    enum class MeshTopology : std::uint8_t
    {
        Triangles,
        Lines
    };

    /// @brief One attribute of a vertex: where the shader reads it and where it sits in the vertex
    struct VertexAttribute
    {
        std::uint32_t location{ 0 };
        VertexFormat format{ VertexFormat::Float3 };
        std::uint32_t offset{ 0 }; ///< bytes from the start of the vertex
    };

    /// @brief Geometry built by the game side; never changed once handed over (build a new one instead)
    struct DynamicMeshData
    {
        GUID id{ C_INVALID_GUID };          ///< invalid: sent every frame; valid: kept on the GPU by id
        std::uint64_t revision{ 0 };        ///< kept meshes: a higher revision replaces the GPU copy
        std::uint32_t vertexStride{ 0 };    ///< bytes per vertex
        std::vector<VertexAttribute> attributes;
        std::vector<std::byte> vertexData;
        std::vector<std::uint32_t> indices; ///< empty: drawn in vertex order
        MeshTopology topology{ MeshTopology::Triangles };
        bool depthTest{ true };             ///< false: drawn over everything
    };

    struct RenderData
    {
        ShaderData vsData{};
        ShaderData tcsData{};
        ShaderData tesData{};
        ShaderData gsData{};
        ShaderData fsData{};

        MeshData meshData{};
        TextureData texData{};

        glm::mat4 transform{};
        
        // Settings
        bool transparent{false};

        /// drawn instead of meshData when set
        std::shared_ptr<const DynamicMeshData> dynamicMesh{};

        // HACK: Sort based on transparency
        bool operator<(const RenderData& p_other) const
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
        Camera camera{};
        Common::RenderDesc renderTarget{};
        std::int32_t priority{};
        std::vector<RenderData> renderData{};
        glm::vec4 clearColor{glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)};

        // DEBUG
        bool debug = false;
    };

#ifdef _EDITOR
    // == ImGui ==

    /// The ImGui texture id that shows render target or texture p_image, resolved by renderer each frame
    constexpr std::uint64_t ImGuiTextureId(GUID p_image) noexcept { return p_image; }

#endif
}
