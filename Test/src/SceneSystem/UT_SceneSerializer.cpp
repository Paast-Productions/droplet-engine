#include <gtest/gtest.h>

#include "SceneSystem/Scene.hpp"
#include "SceneSystem/Node.hpp"
#include "SceneSystem/SceneSerializer.hpp"
#include "Core/IoManager.hpp"
using namespace Droplet::Scene;



class SceneSerializerTest : public ::testing::Test
{
protected:
    SceneSerializer serializer;
    SceneManager sceneManager;


};

class PlayerComponent : public Component
{
public:
    std::string_view GetTypeName() override { return "PlayerComponent"; }

    void Start() override
    {
        std::cout << "PlayerComponent started\n";
    }

    void Update(float p_deltaTime) override
    {
        std::cout
            << "PlayerComponent updating: "
            << p_deltaTime
            << " seconds\n";
    }

    nlohmann::json SerializeImpl() override
    {
        nlohmann::json json;
        json["type"] = "PlayerComponent";
        return json;
    }
};

TEST_F(SceneSerializerTest, SerializeScene)
{
    sceneManager.LoadScene("testScene");
    auto testScene = sceneManager.GetScene("testScene");

    auto root = testScene->AddNode("Root");

    const nlohmann::json result = serializer.SerializeScene(testScene);

    Droplet::Core::JsonIO::Write("testSceneJson", result);

    EXPECT_EQ(result["name"], "testScene");
}

TEST_F(SceneSerializerTest, DeserializeScene)
{
    sceneManager.LoadScene("testScene");
    auto testScene = sceneManager.GetScene("testScene");

    auto root = testScene->AddNode("Root");

    const nlohmann::json result = serializer.SerializeScene(testScene);

    Droplet::Core::JsonIO::Write("testSceneJson", result);
    sceneManager.UnloadScene("testScene");
    const nlohmann::json readResult = Droplet::Core::JsonIO::Read("testSceneJson");

    EXPECT_TRUE(readResult.contains("name"));
    EXPECT_TRUE(readResult.contains("roots"));
    EXPECT_EQ(readResult["name"], "testScene");
    EXPECT_NE(readResult["roots"], nullptr);

    EXPECT_NO_THROW(serializer.DeserializeScene(readResult, sceneManager));
    
}

TEST_F(SceneSerializerTest, SerializeNode)
{
    sceneManager.LoadScene("testScene");
    auto testScene = sceneManager.GetScene("testScene");
    auto root = testScene->AddNode("Root");

    auto testNode = root->AddChild(testScene->AddNode("testNode"));
    auto testChild = testNode->AddChild(testScene->AddNode("testChild"));

    glm::vec3 position = glm::vec3(1.0f, 2.0f, 3.0f);
    testNode->GetTransform().SetPosition(glm::vec3(1.0f, 2.0f, 3.0f));

    testNode->AddComponent<PlayerComponent>();


    nlohmann::json json = serializer.SerializeScene(testScene);
    Droplet::Core::JsonIO::Write("testNodeJson", json);

    EXPECT_EQ(json["name"], "testScene");
    
    nlohmann::json rootJson = json["roots"][0];
    EXPECT_TRUE(rootJson.contains("name"));
    EXPECT_TRUE(rootJson.contains("children"));
    EXPECT_TRUE(rootJson.contains("transform"));


    nlohmann::json nodeJson = rootJson["children"][0];
    EXPECT_TRUE(nodeJson.contains("name"));
    EXPECT_TRUE(nodeJson.contains("children"));
    EXPECT_TRUE(nodeJson.contains("transform"));
    EXPECT_TRUE(nodeJson.contains("components"));

}

TEST_F(SceneSerializerTest, DeserializeNode)
{
    sceneManager.LoadScene("testScene");
    auto testScene = sceneManager.GetScene("testScene");
    auto root = testScene->AddNode("Root");

    auto testNode = root->AddChild(testScene->AddNode("testNode"));
    auto testChild = testNode->AddChild(testScene->AddNode("testChild"));

    glm::vec3 position = glm::vec3(1.0f, 2.0f, 3.0f);
    testNode->GetTransform().SetPosition(glm::vec3(1.0f, 2.0f, 3.0f));

    testNode->AddComponent<PlayerComponent>();


    nlohmann::json json = serializer.SerializeScene(testScene);
    Droplet::Core::JsonIO::Write("testNodeJson", json);
    

    const nlohmann::json readResult = Droplet::Core::JsonIO::Read("testNodeJson");
    SceneManager testManager;
    serializer.DeserializeScene(readResult, testManager);

    EXPECT_EQ(sceneManager.GetScene("testScene"), testManager.GetScene("testScene"));
    
    auto sceneOne = sceneManager.GetScene("testScene");
    auto sceneTwo = testManager.GetScene("testScene");

    EXPECT_EQ(sceneOne->GetName(), sceneTwo->GetName());


    auto rootOne = sceneOne->GetRoots();
    auto rootTwo = sceneTwo->GetRoots();
}
