#include <gtest/gtest.h>

#include "SceneSystem/SceneManager.hpp"
#include "SceneSystem/Scene.hpp"

class SceneManagerTest : public ::testing::Test
{
protected:

    SceneManager sceneManager;
};


TEST_F(SceneManagerTest, LoadScene)
{
    EXPECT_TRUE(sceneManager.LoadScene("Game"));

    auto scene = sceneManager.GetScene("Game");

    ASSERT_NE(scene, nullptr);
    EXPECT_TRUE(scene->IsLoaded());
}


TEST_F(SceneManagerTest, PreventDuplicateScene)
{
    EXPECT_TRUE(sceneManager.LoadScene("Game"));
    EXPECT_FALSE(sceneManager.LoadScene("Game"));
}


TEST_F(SceneManagerTest, GetUnknownSceneReturnsNull)
{
    EXPECT_EQ(
        sceneManager.GetScene("DoesNotExist"),
        nullptr);
}


TEST_F(SceneManagerTest, ActivateScene)
{
    ASSERT_TRUE(sceneManager.LoadScene("Game"));

    EXPECT_TRUE(
        sceneManager.ActivateScene("Game"));

    auto scene = sceneManager.GetScene("Game");

    ASSERT_NE(scene, nullptr);
    EXPECT_TRUE(scene->IsActive());
}


TEST_F(SceneManagerTest, CannotActivateUnknownScene)
{
    EXPECT_FALSE(
        sceneManager.ActivateScene("DoesNotExist"));
}


TEST_F(SceneManagerTest, DeactivateScene)
{
    ASSERT_TRUE(sceneManager.LoadScene("Game"));
    ASSERT_TRUE(sceneManager.ActivateScene("Game"));

    EXPECT_TRUE(
        sceneManager.DeactivateScene("Game"));

    auto scene = sceneManager.GetScene("Game");

    ASSERT_NE(scene, nullptr);
    EXPECT_FALSE(scene->IsActive());
}


TEST_F(SceneManagerTest, CannotDeactivateUnknownScene)
{
    EXPECT_FALSE(
        sceneManager.DeactivateScene("DoesNotExist"));
}


TEST_F(SceneManagerTest, UnloadScene)
{
    ASSERT_TRUE(sceneManager.LoadScene("Game"));

    sceneManager.UnloadScene("Game");

    EXPECT_EQ(
        sceneManager.GetScene("Game"),
        nullptr);
}


TEST_F(SceneManagerTest, UnloadActiveScene)
{
    ASSERT_TRUE(sceneManager.LoadScene("Game"));
    ASSERT_TRUE(sceneManager.ActivateScene("Game"));

    sceneManager.UnloadScene("Game");

    EXPECT_EQ(
        sceneManager.GetScene("Game"),
        nullptr);
}


TEST_F(SceneManagerTest, SceneInitiallyInactive)
{
    ASSERT_TRUE(sceneManager.LoadScene("Game"));

    auto scene = sceneManager.GetScene("Game");

    ASSERT_NE(scene, nullptr);

    EXPECT_FALSE(scene->IsActive());
}