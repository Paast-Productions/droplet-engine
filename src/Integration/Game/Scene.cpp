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
        const CameraComponent& camera = *cameraNode.camera;
        const glm::mat4 cameraToWorld = glm::translate(glm::mat4(1.0f), cameraNode.transform.position) *
            glm::mat4_cast(cameraNode.transform.rotation); // no scale on cameras

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

            const CameraMatrices matrices = MakeCameraMatrices(nodes[cameraIndex]);
            const Math::Frustum frustum = Math::ExtractFrustumFromViewProj(matrices.proj * matrices.view);

            SceneView& view = views.emplace_back(SceneView{.cameraNode = cameraIndex});
            for (std::uint32_t nodeIndex = 0; nodeIndex < nodes.size(); ++nodeIndex)
            {
                // a camera does not see its own node's mesh (e.g. the marker showing where the camera is)
                const Node& node = nodes[nodeIndex];
                if (!node.mesh || nodeIndex == cameraIndex)
                    continue;

                const glm::mat4 model = node.transform.Matrix();
                const glm::vec3 scale = glm::abs(node.transform.scale);
                const glm::vec3 worldCenter(model * glm::vec4(node.mesh->boundsCenter, 1.0f));
                const float worldRadius = node.mesh->boundsRadius * std::max({scale.x, scale.y, scale.z});

                if (Math::IntersectsSphere(frustum, worldCenter, worldRadius))
                    view.visibleNodes.push_back(nodeIndex);
            }
        }
        return views;
    }
}
