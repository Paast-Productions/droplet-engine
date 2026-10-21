#include <gtest/gtest.h>

#include "SceneSystem/Scene.hpp"
#include "SceneSystem/Node.hpp"

using namespace Droplet::Scene;

class SceneTest : public ::testing::Test
{
protected:

    std::shared_ptr<Scene> scene;

    void SetUp() override
    {
        scene = std::make_shared<Scene>("Game");
    }
};


TEST_F(SceneTest, HasCorrectName)
{
    EXPECT_EQ(scene->GetName(), "Game");
}


TEST_F(SceneTest, InitiallyHasNoRoots)
{
    EXPECT_TRUE(scene->GetRoots().empty());
}


TEST_F(SceneTest, AddNode)
{
    auto root =
        scene->AddNode("Root");

    ASSERT_NE(root, nullptr);

    EXPECT_EQ(scene->GetRoots().size(), 1);
    EXPECT_EQ(scene->GetRoots()[0], root);
}


TEST_F(SceneTest, RootHasCorrectName)
{
    auto root =
        scene->AddNode("Root");

    ASSERT_EQ(scene->GetRoots().size(), 1);

    EXPECT_EQ(
        scene->GetRoots()[0]->GetName(),
        "Root");
}


TEST_F(SceneTest, SupportsMultipleRoots)
{
    auto player =
        scene->AddNode("Player");

    auto enemy =
        scene->AddNode("Enemy");

    auto environment =
        scene->AddNode("Environment");

    ASSERT_EQ(scene->GetRoots().size(), 3);

    EXPECT_EQ(scene->GetRoots()[0], player);
    EXPECT_EQ(scene->GetRoots()[1], enemy);
    EXPECT_EQ(scene->GetRoots()[2], environment);
}


TEST_F(SceneTest, InitiallyNotLoaded)
{
    EXPECT_FALSE(scene->IsLoaded());
}


TEST_F(SceneTest, InitiallyInactive)
{
    EXPECT_FALSE(scene->IsActive());
}


TEST_F(SceneTest, LoadScene)
{
    scene->Load();

    EXPECT_TRUE(scene->IsLoaded());
}


TEST_F(SceneTest, LoadingAlreadyLoadedSceneThrows)
{
    scene->Load();

    EXPECT_THROW(
        scene->Load(),
        std::runtime_error);
}


TEST_F(SceneTest, RootReceivesSceneReference)
{
    auto root =
        scene->AddNode("Root");

    scene->Load();

    EXPECT_EQ(root->GetScene(), scene);
}


TEST_F(SceneTest, AllRootsReceiveSceneReference)
{
    auto root1 =
        scene->AddNode("Root1");

    auto root2 =
        scene->AddNode("Root2");

    scene->Load();

    EXPECT_EQ(root1->GetScene(), scene);
    EXPECT_EQ(root2->GetScene(), scene);
}


TEST_F(SceneTest, ActivateScene)
{
    scene->Load();
    scene->SetActive(true);

    EXPECT_TRUE(scene->IsActive());
}


TEST_F(SceneTest, ActivatingUnloadedSceneThrows)
{
    EXPECT_THROW(
        scene->SetActive(true),
        std::runtime_error);
}


TEST_F(SceneTest, DeactivateScene)
{
    scene->Load();
    scene->SetActive(true);
    scene->SetActive(false);

    EXPECT_FALSE(scene->IsActive());
}


TEST_F(SceneTest, UnloadScene)
{
    scene->Load();
    scene->SetActive(true);

    scene->Unload();

    EXPECT_FALSE(scene->IsLoaded());
    EXPECT_FALSE(scene->IsActive());
}


TEST_F(SceneTest, UnloadingUnloadedSceneThrows)
{
    EXPECT_THROW(
        scene->Unload(),
        std::runtime_error);
}


TEST_F(SceneTest, RemoveRoot)
{
    auto root =
        scene->AddNode("Root");

    scene->RemoveRoot(root);

    EXPECT_TRUE(scene->GetRoots().empty());
}


TEST_F(SceneTest, RemovingSpecificRootKeepsOtherRoots)
{
    auto root1 =
        scene->AddNode("Root1");

    auto root2 =
        scene->AddNode("Root2");

    scene->RemoveRoot(root1);

    ASSERT_EQ(scene->GetRoots().size(), 1);
    EXPECT_EQ(scene->GetRoots()[0], root2);
}


TEST_F(SceneTest, CanAddRootAfterSceneIsLoaded)
{
    scene->Load();

    auto root =
        scene->AddNode("Root");

    EXPECT_EQ(root->GetScene(), scene);
}


TEST_F(SceneTest, CanAddRootAfterSceneIsActive)
{
    scene->Load();
    scene->SetActive(true);

    auto root =
        scene->AddNode("Root");

    EXPECT_EQ(root->GetScene(), scene);
}
