/**
 * @file TestDynamicMeshComponent.hpp
 * @brief Test only: draws a dynamic mesh, to check the renderer's dynamic mesh path. Remove once the game side has its own
 */
#pragma once

#include <SceneSystem/Component.hpp>
#include <SceneSystem/DefaultNodeBounds.hpp>
#include <Graphics/Common/RenderData.hpp>
#include <Resource/StaticResourceHandle.hpp>
#include <Resource/Types/Texture2DResource.hpp>

#include <cstdint>
#include <memory>
#include <string>

namespace Droplet::Scene
{
    /// @brief Test only: a streamed star rebuilt every frame, or a quad kept on the GPU
    class TestDynamicMeshComponent : public Component
    {
    public:
        /// @brief Which kind of dynamic mesh the component draws
        enum class Mode : std::int32_t
        {
            Streamed, ///< vertex colours, rebuilt every frame
            Kept      ///< the scene layout, uploaded once; Rebuild raises its revision
        };

        /**
         * @brief Sets the texture the kept quad shows
         * @param p_guid The texture's GUID; invalid: white
         */
        void SetTextureGUID(GUID p_guid);

        TestDynamicMeshComponent();

        [[nodiscard]] std::string_view GetTypeName() const override { return "TestDynamicMeshComponent"; }

        [[nodiscard]] bool HasBoundsOverride() const override { return true; }
        [[nodiscard]] const NodeBounds *GetBounds() const override { return &m_bounds; }

    protected:
        void UpdateImpl() override;
        void EditorUpdateImpl() override;
        void QueueRenderImpl() override;
        [[nodiscard]] Droplet::Graphics::RenderData GetRenderDataImpl() override;
        void RenderUIImpl() override;

        [[nodiscard]] nlohmann::json SerializeImpl() override;
        void DeserializeImpl(nlohmann::json p_compJson) override;

    private:
        /// @brief Rebuilds the star at the current angle
        void BuildStar();
        /// @brief Rebuilds the quad as a new revision
        void BuildQuad();

        Mode m_mode = Mode::Streamed;
        float m_angle = 0.0f;                         ///< degrees, the star's spin
        GUID m_quadId = C_INVALID_GUID;               ///< the kept quad's id, made on first build
        std::uint64_t m_quadRevision = 0;
        std::shared_ptr<const Droplet::Graphics::DynamicMeshData> m_star{};
        std::shared_ptr<const Droplet::Graphics::DynamicMeshData> m_quad{};
        StaticResourceHandle<Texture2DResource> m_texture{}; ///< the kept quad's texture
        std::string m_textureDisplayName;
        DefaultNodeBounds m_bounds{ glm::vec3(0.0f), glm::vec3(1.0f) };
    };
}
