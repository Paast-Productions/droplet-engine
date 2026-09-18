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


TEST_F(SceneTest, HasRoot)
{
    EXPECT_NE(scene->GetRoot(), nullptr);
}


TEST_F(SceneTest, RootHasCorrectName)
{
    auto root = scene->GetRoot();

    ASSERT_NE(root, nullptr);

    EXPECT_EQ(root->GetName(), "Root");
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


TEST_F(SceneTest, LoadSceneOnlyOnce)
{
    scene->Load();
    scene->Load();

    EXPECT_TRUE(scene->IsLoaded());
}


TEST_F(SceneTest, RootReceivesSceneReference)
{
    scene->Load();

    auto root = scene->GetRoot();

    ASSERT_NE(root, nullptr);

    EXPECT_EQ(root->GetScene(), scene);
}


TEST_F(SceneTest, ActivateScene)
{
    scene->Load();
    scene->SetActive(true);

    EXPECT_TRUE(scene->IsActive());
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