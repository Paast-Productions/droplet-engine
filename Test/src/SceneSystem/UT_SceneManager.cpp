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
    EXPECT_NO_THROW(
        sceneManager.LoadScene("Game"));

    auto scene = sceneManager.GetScene("Game");

    ASSERT_NE(scene, nullptr);
    EXPECT_TRUE(scene->IsLoaded());
}


TEST_F(SceneManagerTest, PreventDuplicateScene)
{
    ASSERT_NO_THROW(
        sceneManager.LoadScene("Game"));

    EXPECT_THROW(
        sceneManager.LoadScene("Game"),
        std::runtime_error);
}


TEST_F(SceneManagerTest, GetUnknownSceneReturnsNull)
{
    EXPECT_EQ(
        sceneManager.GetScene("DoesNotExist"),
        nullptr);
}


TEST_F(SceneManagerTest, ActivateScene)
{
    ASSERT_NO_THROW(
        sceneManager.LoadScene("Game"));

    EXPECT_NO_THROW(
        sceneManager.ActivateScene("Game"));

    auto scene = sceneManager.GetScene("Game");

    ASSERT_NE(scene, nullptr);
    EXPECT_TRUE(scene->IsActive());
}


TEST_F(SceneManagerTest, CannotActivateUnknownScene)
{
    EXPECT_THROW(
        sceneManager.ActivateScene("DoesNotExist"),
        std::runtime_error);
}


TEST_F(SceneManagerTest, CannotActivateUnloadedScene)
{
    ASSERT_NO_THROW(
        sceneManager.LoadScene("Game"));

    auto scene = sceneManager.GetScene("Game");

    ASSERT_NE(scene, nullptr);

    scene->Unload();

    EXPECT_THROW(
        sceneManager.ActivateScene("Game"),
        std::runtime_error);
}


TEST_F(SceneManagerTest, CannotActivateAlreadyActiveScene)
{
    ASSERT_NO_THROW(
        sceneManager.LoadScene("Game"));

    ASSERT_NO_THROW(
        sceneManager.ActivateScene("Game"));

    EXPECT_THROW(
        sceneManager.ActivateScene("Game"),
        std::runtime_error);
}


TEST_F(SceneManagerTest, DeactivateScene)
{
    ASSERT_NO_THROW(
        sceneManager.LoadScene("Game"));

    ASSERT_NO_THROW(
        sceneManager.ActivateScene("Game"));

    EXPECT_NO_THROW(
        sceneManager.DeactivateScene("Game"));

    auto scene = sceneManager.GetScene("Game");

    ASSERT_NE(scene, nullptr);
    EXPECT_FALSE(scene->IsActive());
}


TEST_F(SceneManagerTest, CannotDeactivateUnknownScene)
{
    EXPECT_THROW(
        sceneManager.DeactivateScene("DoesNotExist"),
        std::runtime_error);
}


TEST_F(SceneManagerTest, CannotDeactivateInactiveScene)
{
    ASSERT_NO_THROW(
        sceneManager.LoadScene("Game"));

    EXPECT_THROW(
        sceneManager.DeactivateScene("Game"),
        std::runtime_error);
}


TEST_F(SceneManagerTest, UnloadScene)
{
    ASSERT_NO_THROW(
        sceneManager.LoadScene("Game"));

    EXPECT_NO_THROW(
        sceneManager.UnloadScene("Game"));

    EXPECT_EQ(
        sceneManager.GetScene("Game"),
        nullptr);
}


TEST_F(SceneManagerTest, UnloadActiveScene)
{
    ASSERT_NO_THROW(
        sceneManager.LoadScene("Game"));

    ASSERT_NO_THROW(
        sceneManager.ActivateScene("Game"));

    EXPECT_NO_THROW(
        sceneManager.UnloadScene("Game"));

    EXPECT_EQ(
        sceneManager.GetScene("Game"),
        nullptr);
}


TEST_F(SceneManagerTest, CannotUnloadUnknownScene)
{
    EXPECT_THROW(
        sceneManager.UnloadScene("DoesNotExist"),
        std::runtime_error);
}


TEST_F(SceneManagerTest, SceneInitiallyInactive)
{
    ASSERT_NO_THROW(
        sceneManager.LoadScene("Game"));

    auto scene = sceneManager.GetScene("Game");

    ASSERT_NE(scene, nullptr);

    EXPECT_FALSE(scene->IsActive());
}