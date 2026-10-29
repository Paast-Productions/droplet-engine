#pragma once

#include <SceneSystem/Component.hpp>
#include <Graphics/VK/Camera.hpp>
#include <Graphics/DrawLayer.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <string_view>

namespace Droplet::Scene
{
    /// @brief Component representing a camera attached to a scene Node.
    ///
    /// Temporarily controls the editor camera by attaching to a Node.
    /// The editor camera is borrowed and is not owned by this component.
    /// TODO: Should handle multiple cameras and scene cameras not the editor camera like it is now.
    class CameraComponent : public Component
    {
    public:
        /// @brief Gets the type name of the component.
        /// @return The type name of the component.
        [[nodiscard]] std::string_view GetTypeName() const override
        {
            return "CameraComponent";
        }

        [[nodiscard]] Graphics::Camera &GetCamera() { return m_camera; }
        [[nodiscard]] const Graphics::Camera &GetCamera() const { return m_camera; }

        [[nodiscard]] Math::Frustum &GetFrustum() { return m_frustum; }
        [[nodiscard]] const Math::Frustum &GetFrustum() const { return m_frustum; }

		[[nodiscard]] Graphics::DrawLayer GetDrawLayerMask() const { return m_drawLayerMask; }

    protected:
        void StartImpl() override;
        void LateUpdateImpl() override;
        void QueueRenderImpl() override;
		void EditorUpdateImpl() override;
        void RenderUIImpl() override;

        nlohmann::json SerializeImpl() override;
        void DeserializeImpl(nlohmann::json p_compJson) override;

    private:
        Graphics::Camera m_camera{ };
		Math::Frustum m_frustum{ };
		Graphics::DrawLayer m_drawLayerMask{ Graphics::DrawLayer::All };

		bool m_debugDraw{ false };

        // TODO: Internal Draw List
    };
}
