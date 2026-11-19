/**
 * @file Scene.cpp
 * @brief Game::Scene implementation: the camera matrices and CullViews, the sandbox's stand-in for an octree.
 */
#include "Integration/Game/Scene.hpp"

#include <glm/ext/matrix_clip_space.hpp>

#include <algorithm>

#include "Integration/Math/Frustum.hpp"

namespace Game
{
    CameraMatrices MakeCameraMatrices(const Node& cameraNode)
    {
        return MakeCameraMatrices(cameraNode.transform, *cameraNode.camera);
    }

    CameraMatrices MakeCameraMatrices(const Transform& transform, const CameraComponent& camera)
    {
        const glm::mat4 cameraToWorld = glm::translate(glm::mat4(1.0f), transform.position) *
            glm::mat4_cast(transform.rotation); // no scale on cameras
        return {
            .view = glm::inverse(cameraToWorld),
            .proj = glm::perspectiveRH_ZO(glm::radians(camera.fovYDegrees), camera.aspect, camera.nearZ, camera.farZ)
        };
    }

    glm::mat4 AdjustAspect(const glm::mat4& proj, float aspect)
    {
        // For a symmetric perspective projection proj[0][0] = 1 / (aspect * tan(fovY / 2)) and proj[1][1] = 1 / tan(fovY / 2).
        glm::mat4 out = proj;
        out[0][0] = proj[1][1] / aspect;
        return out;
    }

    std::vector<SceneView> Scene::CullViews() const
    {
        std::vector<SceneView> views;
        for (std::uint32_t cameraIndex = 0; cameraIndex < nodes.size(); ++cameraIndex)
        {
            if (!nodes[cameraIndex].camera)
                continue;
            // a camera does not see its own node's mesh (e.g. the marker showing where the camera is)
            views.push_back(SceneView{
                .cameraNode = cameraIndex,
                .visibleNodes = CullNodes(MakeCameraMatrices(nodes[cameraIndex]), cameraIndex)
            });
        }
        return views;
    }

    std::vector<std::uint32_t> Scene::CullNodes(const CameraMatrices& matrices, std::uint32_t skipNode) const
    {
        const Math::Frustum frustum = Math::ExtractFrustumFromViewProj(matrices.proj * matrices.view);
        std::vector<std::uint32_t> visible;
        for (std::uint32_t nodeIndex = 0; nodeIndex < nodes.size(); ++nodeIndex)
        {
            const Node& node = nodes[nodeIndex];
            if (!node.mesh || nodeIndex == skipNode)
                continue;
            const glm::mat4 model = node.transform.Matrix();
            const glm::vec3 scale = glm::abs(node.transform.scale);
            const glm::vec3 worldCenter(model * glm::vec4(node.mesh->boundsCenter, 1.0f));
            const float worldRadius = node.mesh->boundsRadius * std::max({scale.x, scale.y, scale.z});
            if (Math::IntersectsSphere(frustum, worldCenter, worldRadius))
                visible.push_back(nodeIndex);
        }
        return visible;
    }
}
