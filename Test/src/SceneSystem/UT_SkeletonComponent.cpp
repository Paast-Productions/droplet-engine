#include <gtest/gtest.h>

#include "SceneSystem/Components/SkeletonComponent.hpp"
#include "SceneSystem/Node.hpp"
#include "SceneSystem/Scene.hpp"
#include "resource/types/SkinnedMeshResource.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace Droplet::Scene;

namespace
{
    void ExpectMatNear(const glm::mat4 &p_a, const glm::mat4 &p_b, float p_eps = 1e-4f)
    {
        for (int c = 0; c < 4; ++c)
        {
            for (int r = 0; r < 4; ++r)
            {
                EXPECT_NEAR(p_a[c][r], p_b[c][r], p_eps) << "at [" << c << "][" << r << "]";
            }
        }
    }

    glm::mat4 Offset(const glm::vec3 &p_bindPosition)
    {
        // Mesh space -> bone space for a bone located at p_bindPosition in mesh space
        return glm::inverse(glm::translate(glm::mat4(1.0f), p_bindPosition));
    }
}

class SkeletonComponentTest : public ::testing::Test
{
protected:
    std::shared_ptr<Scene> scene;
    std::shared_ptr<Node> node;
    SkeletonComponent *skeleton = nullptr;

    void SetUp() override
    {
        scene = std::make_shared<Scene>("Game");
        scene->Load();
        node = scene->AddNode("Owner");

        std::unique_ptr<Component> component = std::make_unique<SkeletonComponent>();
        skeleton = static_cast<SkeletonComponent *>(component.get());
        node->AttachComponent(component);
    }
};

TEST_F(SkeletonComponentTest, ParentChildHierarchy)
{
    Droplet::SkinnedMeshResource mesh;
    const int root = mesh.AddBone("Root", -1, Offset({0.0f, 1.0f, 0.0f}));
    const int child = mesh.AddBone("Child", root, Offset({0.0f, 3.0f, 0.0f}));

    skeleton->BuildSkeleton(&mesh);

    ASSERT_EQ(skeleton->GetBoneCount(), 2u);
    auto rootNode = skeleton->GetBoneNode(root);
    auto childNode = skeleton->GetBoneNode(child);
    ASSERT_NE(rootNode, nullptr);
    ASSERT_NE(childNode, nullptr);

    EXPECT_EQ(rootNode->GetName(), "Root");
    EXPECT_EQ(childNode->GetName(), "Child");
    EXPECT_EQ(rootNode->GetParent(), node);
    EXPECT_EQ(childNode->GetParent(), rootNode);
    ASSERT_EQ(node->GetChildren().size(), 1u);
    ASSERT_EQ(rootNode->GetChildren().size(), 1u);
}

TEST_F(SkeletonComponentTest, MultipleRoots)
{
    Droplet::SkinnedMeshResource mesh;
    const int a = mesh.AddBone("A", -1, glm::mat4(1.0f));
    const int b = mesh.AddBone("B", -1, glm::mat4(1.0f));
    const int c = mesh.AddBone("C", b, glm::mat4(1.0f));

    skeleton->BuildSkeleton(&mesh);

    EXPECT_EQ(node->GetChildren().size(), 2u);
    EXPECT_EQ(skeleton->GetBoneNode(a)->GetParent(), node);
    EXPECT_EQ(skeleton->GetBoneNode(b)->GetParent(), node);
    EXPECT_EQ(skeleton->GetBoneNode(c)->GetParent(), skeleton->GetBoneNode(b));
}

TEST_F(SkeletonComponentTest, BindTransformsAreInverseOffsetRelativeToParent)
{
    Droplet::SkinnedMeshResource mesh;
    const int root = mesh.AddBone("Root", -1, Offset({0.0f, 1.0f, 0.0f}));
    const int child = mesh.AddBone("Child", root, Offset({0.0f, 3.0f, 0.0f}));

    skeleton->BuildSkeleton(&mesh);

    auto rootNode = skeleton->GetBoneNode(root);
    auto childNode = skeleton->GetBoneNode(child);

    ExpectMatNear(rootNode->GetTransform().GetMatrix(), glm::translate(glm::mat4(1.0f), {0.0f, 1.0f, 0.0f}));
    ExpectMatNear(childNode->GetTransform().GetMatrix(), glm::translate(glm::mat4(1.0f), {0.0f, 2.0f, 0.0f}));
    ExpectMatNear(childNode->GetTransform().GetMatrix(Transform::Space::World), glm::translate(glm::mat4(1.0f), {0.0f, 3.0f, 0.0f}));
}

TEST_F(SkeletonComponentTest, RebuildDoesNotDuplicateNodes)
{
    Droplet::SkinnedMeshResource mesh;
    const int root = mesh.AddBone("Root", -1, glm::mat4(1.0f));
    const int child = mesh.AddBone("Child", root, glm::mat4(1.0f));
    (void)child;

    auto unrelated = scene->AddNode("Unrelated");
    node->AddChild(unrelated);

    skeleton->BuildSkeleton(&mesh);
    skeleton->BuildSkeleton(&mesh);

    EXPECT_EQ(skeleton->GetBoneCount(), 2u);
    EXPECT_EQ(node->GetChildren().size(), 2u); // Unrelated + Root
    EXPECT_EQ(scene->GetRoots().size(), 1u);   // Only the owner is a scene root

    skeleton->BuildSkeleton(nullptr);

    EXPECT_EQ(skeleton->GetBoneCount(), 0u);
    ASSERT_EQ(node->GetChildren().size(), 1u);
    EXPECT_EQ(node->GetChildren()[0], unrelated);
    EXPECT_EQ(scene->GetRoots().size(), 1u);
}

TEST_F(SkeletonComponentTest, OutOfRangeBoneLookupReturnsNull)
{
    Droplet::SkinnedMeshResource mesh;
    const int a = mesh.AddBone("A", -1, glm::mat4(1.0f));
    const int b = mesh.AddBone("B", a, glm::mat4(1.0f));
    (void)b;

    skeleton->BuildSkeleton(&mesh);
    EXPECT_EQ(skeleton->GetBoneNode(a)->GetParent(), node);
    EXPECT_EQ(skeleton->GetBoneNode(static_cast<std::size_t>(99)), nullptr);
}

TEST_F(SkeletonComponentTest, UnavailableResourceBuildsNothing)
{
    skeleton->SetSkinnedMeshResource({});
    skeleton->Start();
    skeleton->Update();

    EXPECT_EQ(skeleton->GetBoneCount(), 0u);
    EXPECT_TRUE(node->GetChildren().empty());
    EXPECT_EQ(skeleton->GetSkinnedMeshGUID(), Droplet::C_INVALID_GUID);
}
