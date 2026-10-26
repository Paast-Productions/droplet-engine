#include <SceneSystem/Components/SkeletonComponent.hpp>

#include <ImGui/imgui.h>

#include <Editor/ResourceSlot.hpp>
#include <Graphics/DebugDraw.hpp>
#include <SceneSystem/Scene.hpp>
#include <resource/ResourceManager.hpp>

#include <glm/glm.hpp>

#include <cmath>
#include <utility>
#include <vector>
#include <DefaultNodeBounds.hpp>

using namespace Droplet::Scene;

namespace
{
	/// @brief Checks if a 4x4 matrix is invertible by calculating its determinant.
	/// This is used to determine if a bone's offset matrix is valid for computing its bind pose.
	/// @param p_matrix The matrix to check.
	/// @return True if the matrix is invertible, false otherwise.
    bool IsInvertible(const glm::mat4 &p_matrix)
    {
        const float det = glm::determinant(p_matrix);
        return std::isfinite(det) && std::abs(det) > 1e-12f;
    }

    /// Resolves parent indices, treating out-of-range, self and cyclic parents as roots (-1).
    std::vector<int> ResolveParents(const std::vector<Droplet::SkinnedMeshResource::Bone> &p_bones)
    {
        const int count = static_cast<int>(p_bones.size());
        std::vector<int> parents(p_bones.size(), -1);

        for (int i = 0; i < count; ++i)
        {
            const int parent = p_bones[i].parentIndex;
            if (parent < 0 || parent >= count || parent == i)
            {
                continue;
            }

            // Walk up the chain; if it never terminates, the parent chain is cyclic.
            int current = parent;
            int steps = 0;
            while (current >= 0 && current < count && current != i && steps <= count)
            {
                current = p_bones[current].parentIndex;
                ++steps;
            }

            if (current < 0 || current >= count)
            {
                parents[i] = parent;
            }
        }

        return parents;
    }
}

void SkeletonComponent::SetSkinnedMeshResource(ResourceHandle<SkinnedMeshResource> p_handle)
{
    if (p_handle.GetGUID() != m_skinnedMesh.GetGUID())
    {
        ClearSkeleton();
    }

    m_skinnedMesh = std::move(p_handle);
    SyncWithResource();
}

void SkeletonComponent::SetSkinnedMeshGUID(Droplet::GUID p_guid)
{
    const std::weak_ptr<bool> alive = m_alive;
    ResourceHandle<SkinnedMeshResource> handle = ResourceManager::GetInstance().LoadResource<Droplet::SkinnedMeshResource>(
        p_guid,
        [this, alive](ResourceHandle<SkinnedMeshResource> p_loaded)
        {
            // The component may have been destroyed or retargeted before the load completed
            if (alive.expired() || p_loaded.GetGUID() != m_skinnedMesh.GetGUID())
            {
                return;
            }
            SyncWithResource();
        });
    SetSkinnedMeshResource(std::move(handle));
}

std::shared_ptr<Node> SkeletonComponent::GetBoneNode(std::size_t p_boneIndex) const
{
    return p_boneIndex < m_boneNodes.size() ? m_boneNodes[p_boneIndex] : nullptr;
}

void SkeletonComponent::ClearSkeleton()
{
    const auto scene = GetOwner() ? GetOwner()->GetScene() : nullptr;

    // Detach in reverse so children are removed before their parents.
    for (auto it = m_boneNodes.rbegin(); it != m_boneNodes.rend(); ++it)
    {
        const std::shared_ptr<Node> &node = *it;
        if (!node)
        {
            continue;
        }

        if (const auto parent = node->GetParent())
        {
            parent->RemoveChild(node); // Re-adds the node as a scene root
        }

        if (scene)
        {
            scene->RemoveRoot(node);
        }
    }

    m_boneNodes.clear();
    m_builtFrom = nullptr;
}

void SkeletonComponent::BuildSkeleton(const SkinnedMeshResource *p_mesh)
{
    ClearSkeleton();

    const auto owner = GetOwner();
    const auto scene = owner ? owner->GetScene() : nullptr;
    if (!p_mesh || !scene)
    {
        return;
    }

    const auto &bones = p_mesh->GetBones();
    const std::vector<int> parents = ResolveParents(bones);

    // Create all nodes first, so parent order in the resource does not matter.
    m_boneNodes.reserve(bones.size());
    for (const auto &bone : bones)
    {
        m_boneNodes.push_back(scene->AddNode(bone.name));
    }

    // offsetMat maps mesh space to bone space, so a bone's bind-pose global transform (relative to the owner) is
    // inverse(offsetMat). Its local transform is inverse(parentGlobal) * global. Bones are children of the owner,
    // so root bones use their global transform directly and the owner's transform is applied on top.
    std::vector<glm::mat4> globals(bones.size(), glm::mat4(1.0f));
    for (std::size_t i = 0; i < bones.size(); ++i)
    {
        if (IsInvertible(bones[i].offsetMat))
        {
            globals[i] = glm::inverse(bones[i].offsetMat);
        }
    }

    for (std::size_t i = 0; i < bones.size(); ++i)
    {
		const auto &bone = bones[i];
        const int parent = parents[i];
        const std::shared_ptr<Node> &parentNode = parent >= 0 ? m_boneNodes[parent] : owner;
        parentNode->AddChild(m_boneNodes[i]);

        glm::mat4 local = globals[i];
        if (parent >= 0)
        {
            local = glm::inverse(globals[parent]) * globals[i];
        }

        m_boneNodes[i]->GetTransform().SetMatrix(local);

		// Get the bone's bounds from the mesh resource and generate a default bounding box with it.
        std::shared_ptr<DefaultNodeBounds> bounds = std::make_shared<DefaultNodeBounds>(bone.bounds);
        m_boneNodes[i]->SetBounds(bounds);
    }

    m_builtFrom = p_mesh;
}

void SkeletonComponent::SyncWithResource()
{
    if (!GetOwner())
    {
        return;
    }

    if (m_skinnedMesh.IsReady())
    {
        const SkinnedMeshResource *mesh = m_skinnedMesh.Get();
        if (mesh != m_builtFrom)
        {
            BuildSkeleton(mesh);
        }
    }
    else if (m_skinnedMesh.HasFailed() || !m_skinnedMesh.IsValid())
    {
        if (m_builtFrom)
        {
            ClearSkeleton();
        }
    }
}

void SkeletonComponent::DrawDebugSkeleton() const
{
    using Droplet::Graphics::Debug::DebugDraw;

    const auto owner = GetOwner();
    const glm::vec4 boneColor(0.2f, 1.0f, 0.3f, 1.0f);
    const glm::vec4 jointColor(1.0f, 0.8f, 0.1f, 1.0f);
    const glm::vec4 rootLinkColor(0.3f, 0.6f, 1.0f, 1.0f);

    for (const auto &bone : m_boneNodes)
    {
        if (!bone)
        {
            continue;
        }

        const glm::vec3 position = bone->GetTransform().GetPosition(Transform::Space::World);
        const auto parent = bone->GetParent();

        if (parent)
        {
            const bool toOwner = parent == owner;
            const glm::vec3 parentPosition = parent->GetTransform().GetPosition(Transform::Space::World);
            DebugDraw::DrawLine(parentPosition, position, toOwner ? rootLinkColor : boneColor, false);
        }

        DebugDraw::DrawSphere(Droplet::Math::Sphere(position, 0.02f), jointColor, 1, false);
    }
}

void SkeletonComponent::StartImpl()
{
    SyncWithResource();
}

void SkeletonComponent::EditorUpdateImpl()
{
    if (m_debugDraw)
    {
        DrawDebugSkeleton();
    }
}

void SkeletonComponent::RenderUIImpl()
{
    using namespace ImGui;

    PushID(this);

	Checkbox("Debug Draw", &m_debugDraw);

    if (BeginTable("SkeletonProps", 2, ImGuiTableFlags_SizingStretchProp))
    {
        TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

        Droplet::GUID newGUID = C_INVALID_GUID;
        std::string newName;

        if (ResourceSlot(
            "Skinned Mesh",
            m_skinnedMesh.GetGUID(),
            m_meshDisplayName,
            newGUID,
            newName,
            Droplet::ResourceType::SkinnedMesh))
        {
            SetSkinnedMeshGUID(newGUID);
            m_meshDisplayName = newName;
        }

        EndTable();
    }

    Text("Bones: %zu", m_boneNodes.size());

    PopID();
}

nlohmann::json SkeletonComponent::SerializeImpl()
{
    nlohmann::json json;

    json["skinnedMeshGUID"] = GetSkinnedMeshGUID();
    json["skinnedMeshDisplayName"] = m_meshDisplayName;

    return json;
}

void SkeletonComponent::DeserializeImpl(const nlohmann::json p_compJson)
{
    if (p_compJson.contains("skinnedMeshGUID"))
    {
        SetSkinnedMeshGUID(p_compJson["skinnedMeshGUID"].get<Droplet::GUID>());
    }

    if (p_compJson.contains("skinnedMeshDisplayName"))
    {
        m_meshDisplayName = p_compJson["skinnedMeshDisplayName"].get<std::string>();
    }
}
