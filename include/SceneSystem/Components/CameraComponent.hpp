
#pragma once

#include <SceneSystem/Component.hpp>
#include <Graphics/VK/Camera.hpp>
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
        CameraComponent();

        void SetCamera(Droplet::Graphics::Camera *p_camera)
        {
            m_camera = p_camera;
        }

        /// @brief Gets the type name of the component.
        /// @return The type name of the component.
        [[nodiscard]] std::string_view GetTypeName() const override
        {
            return "CameraComponent";
        }

        /// @brief Sets the camera offset relative to the target.
        /// @param p_offset World-space position offset.
        void SetOffset(const glm::vec3 &p_offset)
        {
            m_offset = p_offset;
        }

        /// @brief Gets the camera offset relative to the target.
        /// @return The world-space position offset.
        [[nodiscard]] const glm::vec3 &GetOffset() const
        {
            return m_offset;
        }

    protected:
        void StartImpl() override;
        void UpdateImpl() override;
        void RenderUIImpl() override;

        nlohmann::json SerializeImpl() override;
        void DeserializeImpl(nlohmann::json p_compJson) override;

    private:
        Droplet::Graphics::Camera *m_camera = nullptr;
        // Node followed by the editor camera.
        std::weak_ptr<Node> m_target;

        // Offset from the target's world position.
        glm::vec3 m_offset{0.0f, 3.0f, 0.3f};

        glm::vec3 m_smoothedPosition{ 0.0f };
        float m_smoothedYaw = 0.0f;     // degrees
        float m_smoothedPitch = 0.0f;   // degrees
        bool  m_initialized = false;

        float m_rotationLag = 30.0f;     // higher = snappier, lower = laggier
        float m_positionLag = 60.0f;

    };
}
