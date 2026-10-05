#include <gtest/gtest.h>

#include "SceneSystem/Scene.hpp"
#include "SceneSystem/Node.hpp"
#include "SceneSystem/SceneSerializer.hpp"
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

class SceneSerializerTest : public ::testing::Test
{
protected:
    SceneSerializer serializer;
    SceneManager sceneManager;
};


TEST_F(SceneSerializerTest, SerializeScene)
{
    
}

TEST_F(SceneSerializerTest, DeserializeScene)
{
    
}
