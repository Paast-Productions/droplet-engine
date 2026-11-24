#pragma once

#include <memory>
#include <string_view>

#include <glm/glm.hpp>
#include <json/json.hpp>

#include <SceneSystem/Component.hpp>
#include <Common/RenderDesc.hpp>
#include <Common/Camera.hpp>
#include <Common/DrawLayer.hpp>

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

		~CameraComponent() final = default;

        [[nodiscard]] Common::Camera &GetCamera() { return m_camera; }
        [[nodiscard]] const Common::Camera &GetCamera() const { return m_camera; }

        [[nodiscard]] Math::Frustum &GetFrustum() { return m_frustum; }
        [[nodiscard]] const Math::Frustum &GetFrustum() const { return m_frustum; }

		[[nodiscard]] Common::DrawLayer GetDrawLayerMask() const { return m_drawLayerMask; }

		[[nodiscard]] int32_t GetPriority() const { return m_priority; }
		void SetPriority(int32_t p_priority) { m_priority = p_priority; }

		[[nodiscard]] glm::ivec2 GetResolution() const { return m_resolution; }
        void SetResolution(glm::ivec2 p_resolution);

		[[nodiscard]] bool IsAutoRenderEnabled() const { return m_autoRender; }
		void SetAutoRender(bool p_enabled) { m_autoRender = p_enabled; }
		void TriggerRender() { m_manualRender = true; }

        void CullRenderables(const std::vector<std::pair<std::shared_ptr<Node>, std::shared_ptr<Component>>> &p_renderables);

        [[nodiscard]] Common::CameraRenderData GetRenderData() const;
		void ClearRenderData() { m_renderData.clear(); }

        [[nodiscard]] const Common::RenderTargetDesc &GetTarget() const { return m_target; }
        void SetTarget(const Common::RenderTargetDesc &p_renderTarget) { m_target = p_renderTarget; }

		[[nodiscard]] glm::vec4 GetClearColor() const { return m_clearColor; }
		void SetClearColor(const glm::vec4 &p_clearColor) { m_clearColor = p_clearColor; }

    protected:
        void StartImpl() override;
        void LateUpdateImpl() override;
        void QueueRenderImpl() override;
		void EditorUpdateImpl() override;
        void RenderUIImpl() override;

        nlohmann::json SerializeImpl() override;
        void DeserializeImpl(nlohmann::json p_compJson) override;

    private:
        Common::Camera m_camera{ };
		Math::Frustum m_frustum{ };

		Common::DrawLayer m_drawLayerMask{ Common::DrawLayer::All };
		int32_t m_priority{ 0 };

        Common::RenderTargetDesc m_target { };
		glm::ivec2 m_resolution{ 256, 256 };
		bool m_dirty{ true };

		bool m_autoRender{ true };
		bool m_manualRender{ false };

		bool m_debugDraw{ false };
		bool m_uiSelected{ false };
		float m_uiScale{ 1.0f };

		glm::vec4 m_clearColor{ 0.0f, 0.0f, 0.3f, 1.0f };

		std::vector<Common::RenderData> m_renderData{ };
    };
}
