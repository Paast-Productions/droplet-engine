#pragma once
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "Integration/Core/Guid.hpp"
#include "Integration/Common/RenderDesc.hpp"

/// namespace for spoof scene
namespace Game
{
    /// A simple Transform for an object
    struct Transform
    {
        glm::vec3 position{ 0.0f };
        glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
        glm::vec3 scale{ 1.0f };

        glm::mat4 Matrix() const
        {
            return glm::translate(glm::mat4(1.0f), position) * glm::mat4_cast(rotation) *
                glm::scale(glm::mat4(1.0f), scale);
        }
    };

    /// A camera in the scene
    struct CameraComponent
    {
        float fovYDegrees = 60.0f;
        float aspect = 1.0f;
        float nearZ = 0.1f;
        float farZ = 100.0f;
        Guid output; // id of a RenderTargetDesc declared in the Scene
        bool present = false;

        // parameters for animation
        glm::vec3 startPos = { 0.0f, 0.0f, 0.0f };
        glm::vec3 lookAtTarget = { 0.0f, 0.0f, 1.0f };
        glm::vec3 upDir = { 0.0f, 1.0f, 0.0f };
    };

    /// A component for a node to hold that holds a scene, a texture and a shader
    struct MeshComponent
    {
        Guid mesh; // MeshAsset id
        Common::ImageRef imageTexture; // texture asset OR a camera's render target
        Guid shader; // ShaderAsset id; invalid: the renderer's default scene shader
        glm::vec3 boundsCenter{0.0f}; ///< mesh-space bounding sphere, for culling
        float boundsRadius{0.8660254f}; ///< the default is a unit cube's, like the renderer's placeholder
    };

    /// Oscillates a node around its rest position:
    /// pos = rest + axis * amplitude * sin(2*pi*time/period).
    struct AnimationComponent
    {
        enum class Kind
        {
            Oscillate,
            Pan
        };

        float amplitude = 1.0f;
        float period = 1.0f; // seconds
        glm::vec3 axis{1.0f, 0.0f, 0.0f}; // direction of the swing
        Kind kind{Kind::Oscillate};
    };

    /// @brief Turns a node about `axis` on top of its rest rotation.
    struct SpinComponent
    {
        glm::vec3 axis{0.0f, 1.0f, 0.0f};
        float degreesPerSecond = 45.0f;
        glm::quat restRotation{1.0f, 0.0f, 0.0f, 0.0f};
    };

    /// @brief Effects applied, in order, to the output of the camera on the same node.
    struct PostProcessComponent
    {
        struct Effect
        {
            Common::PostEffect effect{Common::PostEffect::Grayscale};
            float amount{1.0f};
            bool enabled{true}; ///< disabled effects stay in the list but are skipped
        };

        std::vector<Effect> effects;
    };

    struct Node
    {
        std::string name;
        Transform transform;
        std::optional<AnimationComponent> animation;
        std::optional<SpinComponent> spin;
        std::optional<CameraComponent> camera;
        std::optional<MeshComponent> mesh;
        std::optional<PostProcessComponent> postProcess; ///< used with a camera on the same node
    };

    /// A camera's view and projection matrices
    struct CameraMatrices
    {
        glm::mat4 view{1.0f};
        glm::mat4 proj{1.0f};
    };

    /// @param cameraNode a node with a CameraComponent
    CameraMatrices MakeCameraMatrices(const Node& cameraNode);

    /// Replaces the aspect ratio of a symmetric perspective projection (as MakeCameraMatrices builds it), keeping the
    /// vertical field of view. Use it when the surface you draw into is not the size the view was made for, e.g. a
    /// resizable window.
    glm::mat4 AdjustAspect(const glm::mat4& proj, float aspect);

    /// One camera and the mesh nodes it can see: what the scene hands over for rendering
    struct SceneView
    {
        std::uint32_t cameraNode{0}; ///< index into Scene::nodes
        std::vector<std::uint32_t> visibleNodes; ///< indices into Scene::nodes, in node order; never the camera's own
    };

    struct Scene
    {
        std::vector<Node> nodes;
        std::vector<Common::RenderTargetDesc> renderTargets;

        const Common::RenderTargetDesc* FindTarget(Guid id) const
        {
            for (const auto& target : renderTargets)
                if (target.id == id) return &target;
            return nullptr;
        }
        Common::RenderTargetDesc* FindTarget(Guid id)
        {
            return const_cast<Common::RenderTargetDesc*>(std::as_const(*this).FindTarget(id));
        }

        /// A view per camera node with the mesh nodes inside its frustum. A stand-in for the scene's octree query
        std::vector<SceneView> CullViews() const;

        Node* FindNode(std::string_view name)
        {
            const auto nodeIt = std::ranges::find(nodes, name, &Game::Node::name);
            return nodeIt != nodes.end()
                       ? &*nodeIt
                       : nullptr;
        }

        void Animate(float time)
        {
            static constexpr float TWO_PI = 6.28318530718f;

            for (Node& node : nodes)
            {
                if (node.camera && node.animation && node.animation->period > 0.0f)
                {
                    const CameraComponent& cam = *node.camera;
                    const AnimationComponent& anim = *node.animation;

                    const float offset = anim.amplitude * std::sin(TWO_PI * time / anim.period);
                    const glm::vec3 pos = cam.startPos + anim.axis * offset;

                    node.transform.position = pos;

                    // orbit
                    if (anim.kind == AnimationComponent::Kind::Oscillate)
                    {
                        // move while looking at target
                        node.transform.rotation = glm::quatLookAtRH(glm::normalize(cam.lookAtTarget - pos), cam.upDir);
                    }
                    else // pan
                    {
                        // move while looking in direction
                        const glm::vec3 forward = glm::normalize(cam.lookAtTarget - cam.startPos);
                        node.transform.rotation = glm::quatLookAtRH(forward, cam.upDir);
                    }
                }
                if (node.spin)
                {
                    const SpinComponent& spin = *node.spin;
                    node.transform.rotation =
                        glm::angleAxis(glm::radians(spin.degreesPerSecond * time),
                                       glm::normalize(spin.axis)) * spin.restRotation;
                }
            }
        }
    };
}
