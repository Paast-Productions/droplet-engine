#include <gtest/gtest.h>

#include "SceneSystem/Scene.hpp"
#include "SceneSystem/Node.hpp"


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


TEST_F(SceneTest, AddRoot)
{
    auto root =
        std::make_shared<Node>("Root");

    auto result =
        scene->AddRoot(root);

    ASSERT_NE(result, nullptr);

    EXPECT_EQ(result, root);
    EXPECT_EQ(scene->GetRoots().size(), 1);
    EXPECT_EQ(scene->GetRoots()[0], root);
}


TEST_F(SceneTest, RootHasCorrectName)
{
    auto root =
        std::make_shared<Node>("Root");

    scene->AddRoot(root);

    ASSERT_EQ(scene->GetRoots().size(), 1);

    EXPECT_EQ(
        scene->GetRoots()[0]->GetName(),
        "Root");
}


TEST_F(SceneTest, SupportsMultipleRoots)
{
    auto player =
        std::make_shared<Node>("Player");

    auto enemy =
        std::make_shared<Node>("Enemy");

    auto environment =
        std::make_shared<Node>("Environment");

    scene->AddRoot(player);
    scene->AddRoot(enemy);
    scene->AddRoot(environment);

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
        std::make_shared<Node>("Root");

    scene->AddRoot(root);
    scene->Load();

    EXPECT_EQ(root->GetScene(), scene);
}


TEST_F(SceneTest, AllRootsReceiveSceneReference)
{
    auto root1 =
        std::make_shared<Node>("Root1");

    auto root2 =
        std::make_shared<Node>("Root2");

    scene->AddRoot(root1);
    scene->AddRoot(root2);

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
        std::make_shared<Node>("Root");

    scene->AddRoot(root);

    scene->RemoveRoot(root);

    EXPECT_TRUE(scene->GetRoots().empty());
}


TEST_F(SceneTest, RemovingRootClearsSceneReference)
{
    auto root =
        std::make_shared<Node>("Root");

    scene->AddRoot(root);
    scene->Load();

    ASSERT_EQ(root->GetScene(), scene);

    scene->RemoveRoot(root);

    EXPECT_EQ(root->GetScene(), nullptr);
}


TEST_F(SceneTest, RemovingSpecificRootKeepsOtherRoots)
{
    auto root1 =
        std::make_shared<Node>("Root1");

    auto root2 =
        std::make_shared<Node>("Root2");

    scene->AddRoot(root1);
    scene->AddRoot(root2);

    scene->RemoveRoot(root1);

    ASSERT_EQ(scene->GetRoots().size(), 1);
    EXPECT_EQ(scene->GetRoots()[0], root2);
}


TEST_F(SceneTest, AddingNullRootThrows)
{
    EXPECT_THROW(
        scene->AddRoot(nullptr),
        std::invalid_argument);
}


TEST_F(SceneTest, CannotAddChildAsRoot)
{
    auto parent =
        std::make_shared<Node>("Parent");

    auto child =
        std::make_shared<Node>("Child");

    parent->AddChild(child);

    EXPECT_THROW(
        scene->AddRoot(child),
        std::runtime_error);
}


TEST_F(SceneTest, CanAddRootAfterSceneIsLoaded)
{
    scene->Load();

    auto root =
        std::make_shared<Node>("Root");

    scene->AddRoot(root);

    EXPECT_EQ(root->GetScene(), scene);
}


TEST_F(SceneTest, CanAddRootAfterSceneIsActive)
{
    scene->Load();
    scene->SetActive(true);

    auto root =
        std::make_shared<Node>("Root");

    scene->AddRoot(root);

    EXPECT_EQ(root->GetScene(), scene);
}