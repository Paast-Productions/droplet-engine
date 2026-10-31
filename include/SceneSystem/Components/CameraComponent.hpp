#pragma once

#include <SceneSystem/Component.hpp>
#include <Graphics/VK/Camera.hpp>
#include <Graphics/GraphicsTypes.hpp>
#include <Graphics/DrawLayer.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <string_view>

namespace Droplet::Scene
{
    /// @brief Component representing a camera attached to a scene Node.e
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

		[[nodiscard]] int32_t GetPriority() const { return m_priority; }
		void SetPriority(int32_t p_priority) { m_priority = p_priority; }

		[[nodiscard]] glm::ivec2 GetResolution() const { return m_resolution; }
        void SetResolution(glm::ivec2 p_resolution);

		[[nodiscard]] bool IsAutoRenderEnabled() const { return m_autoRender; }
		void SetAutoRender(bool p_enabled) { m_autoRender = p_enabled; }
		void TriggerRender() { m_manualRender = true; }

        void CullRenderables(const std::vector<std::pair<std::shared_ptr<Node>, std::shared_ptr<Component>>> &p_renderables);

        [[nodiscard]] Graphics::CameraRenderData GetRenderData() const;
		void ClearRenderData() { m_renderData.clear(); }

        [[nodiscard]] std::uint32_t GetTargetIndex() const { return m_targetIndex; }
        void SetTargetIndex(std::uint32_t p_renderTarget) { m_targetIndex = p_renderTarget; }

        [[nodiscard]] std::uint32_t GetTextureIndex() const { return m_textureIndex; }
        void SetTextureIndex(std::uint32_t p_textureIndex) { m_textureIndex = p_textureIndex; }

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
		int32_t m_priority{ 0 };

        std::uint32_t m_targetIndex{ Graphics::VK::INVALID_INDEX }; // The index of the texture that the camera will render to
        std::uint32_t m_textureIndex{ Graphics::VK::INVALID_INDEX }; // The index of the texture that ImGui uses to render the camera view
		glm::ivec2 m_resolution{ 256, 256 };
		bool m_dirty{ true };

		bool m_autoRender{ true };
		bool m_manualRender{ false };

		bool m_debugDraw{ false };
		float m_uiScale{ 1.0f };

		std::vector<Graphics::RenderData> m_renderData{ };
    };
}
