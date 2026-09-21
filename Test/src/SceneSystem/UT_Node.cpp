#include <gtest/gtest.h>

#include "SceneSystem/Node.hpp"
#include "SceneSystem/Scene.hpp"
#include "SceneSystem/Component.hpp"


class TComponent : public Component
{
public:

    void Initialize() override
    {
        initialized = true;
    }

    void Start() override
    {
        startCount++;
    }

    void Update(float p_deltaTime) override
    {
        updateCount++;
        lastDeltaTime = p_deltaTime;
    }

    bool initialized = false;
    int startCount = 0;
    int updateCount = 0;
    float lastDeltaTime = 0.0f;
};


class NodeTest : public ::testing::Test
{
protected:

    std::shared_ptr<Node> root;

    void SetUp() override
    {
        root = std::make_shared<Node>("Root");
    }
};


TEST_F(NodeTest, HasCorrectName)
{
    EXPECT_EQ(root->GetName(), "Root");
}


TEST_F(NodeTest, AddChild)
{
    auto player =
        root->AddChild(
            std::make_shared<Node>("Player"));

    ASSERT_NE(player, nullptr);

    EXPECT_EQ(root->GetChildren().size(), 1);
    EXPECT_EQ(root->GetChildren()[0], player);
}


TEST_F(NodeTest, AddNullChildThrows)
{
    EXPECT_THROW(
        root->AddChild(nullptr),
        std::invalid_argument);
}


TEST_F(NodeTest, CannotAddChildWithParent)
{
    auto parent1 =
        std::make_shared<Node>("Parent1");

    auto parent2 =
        std::make_shared<Node>("Parent2");

    auto child =
        std::make_shared<Node>("Child");

    parent1->AddChild(child);

    EXPECT_THROW(
        parent2->AddChild(child),
        std::runtime_error);
}


TEST_F(NodeTest, CannotAddNodeToItself)
{
    EXPECT_THROW(
        root->AddChild(root),
        std::runtime_error);
}


TEST_F(NodeTest, CannotAddNodeThatBelongsToScene)
{
    auto scene =
        std::make_shared<Scene>("Game");

    scene->AddRoot(root);
    scene->Load();

    auto child =
        std::make_shared<Node>("Child");

    scene->AddRoot(child);

    EXPECT_THROW(
        root->AddChild(child),
        std::runtime_error);
}


TEST_F(NodeTest, ChildHasCorrectParent)
{
    auto player =
        root->AddChild(
            std::make_shared<Node>("Player"));

    ASSERT_NE(player, nullptr);

    EXPECT_EQ(player->GetParent(), root);
}


TEST_F(NodeTest, RemoveChild)
{
    auto player =
        root->AddChild(
            std::make_shared<Node>("Player"));

    ASSERT_NE(player, nullptr);

    EXPECT_NO_THROW(
        root->RemoveChild(player));

    EXPECT_TRUE(root->GetChildren().empty());
    EXPECT_EQ(player->GetParent(), nullptr);
}


TEST_F(NodeTest, RemoveNonexistentChildThrows)
{
    auto player =
        std::make_shared<Node>("Player");

    EXPECT_THROW(
        root->RemoveChild(player),
        std::runtime_error);
}


TEST_F(NodeTest, RemoveNullChildThrows)
{
    EXPECT_THROW(
        root->RemoveChild(nullptr),
        std::invalid_argument);
}


TEST_F(NodeTest, RemovingChildClearsSceneReference)
{
    auto scene =
        std::make_shared<Scene>("Game");

    scene->AddRoot(root);
    scene->Load();

    auto player =
        root->AddChild(
            std::make_shared<Node>("Player"));

    ASSERT_NE(player, nullptr);
    ASSERT_EQ(player->GetScene(), scene);

    root->RemoveChild(player);

    EXPECT_EQ(player->GetScene(), nullptr);
}


TEST_F(NodeTest, SceneReference)
{
    auto scene =
        std::make_shared<Scene>("Game");

    scene->AddRoot(root);
    scene->Load();

    EXPECT_EQ(root->GetScene(), scene);
}


TEST_F(NodeTest, ChildInheritsSceneReference)
{
    auto scene =
        std::make_shared<Scene>("Game");

    scene->AddRoot(root);
    scene->Load();

    auto player =
        root->AddChild(
            std::make_shared<Node>("Player"));

    ASSERT_NE(player, nullptr);

    EXPECT_EQ(player->GetScene(), scene);
}


TEST_F(NodeTest, GrandchildInheritsSceneReference)
{
    auto scene =
        std::make_shared<Scene>("Game");

    scene->AddRoot(root);
    scene->Load();

    auto player =
        root->AddChild(
            std::make_shared<Node>("Player"));

    auto weapon =
        player->AddChild(
            std::make_shared<Node>("Weapon"));

    ASSERT_NE(weapon, nullptr);

    EXPECT_EQ(weapon->GetScene(), scene);
}


TEST_F(NodeTest, AddComponent)
{
    auto component =
        root->AddComponent<TComponent>();

    ASSERT_NE(component, nullptr);

    EXPECT_TRUE(component->initialized);
}


TEST_F(NodeTest, ComponentHasOwner)
{
    auto component =
        root->AddComponent<TComponent>();

    ASSERT_NE(component, nullptr);

    EXPECT_EQ(component->GetOwner(), root);
}


TEST_F(NodeTest, GetComponents)
{
    root->AddComponent<TComponent>();

    auto components =
        root->GetComponents<TComponent>();

    ASSERT_EQ(components.size(), 1);
}


TEST_F(NodeTest, SupportsMultipleComponents)
{
    root->AddComponent<TComponent>();
    root->AddComponent<TComponent>();

    auto components =
        root->GetComponents<TComponent>();

    EXPECT_EQ(components.size(), 2);
}


TEST_F(NodeTest, RemoveSpecificComponent)
{
    auto component1 =
        root->AddComponent<TComponent>();

    auto component2 =
        root->AddComponent<TComponent>();

    ASSERT_NE(component1, nullptr);
    ASSERT_NE(component2, nullptr);

    EXPECT_NO_THROW(
        root->RemoveComponent(component1));

    auto components =
        root->GetComponents<TComponent>();

    ASSERT_EQ(components.size(), 1);
    EXPECT_EQ(components[0], component2);
}


TEST_F(NodeTest, RemoveNonexistentComponentThrows)
{
    auto component =
        std::make_shared<TComponent>();

    EXPECT_THROW(
        root->RemoveComponent(component),
        std::runtime_error);
}


TEST_F(NodeTest, RemoveNullComponentThrows)
{
    EXPECT_THROW(
        root->RemoveComponent(nullptr),
        std::invalid_argument);
}


TEST_F(NodeTest, ComponentDoesNotStartBeforeSceneActivation)
{
    auto scene =
        std::make_shared<Scene>("Game");

    scene->AddRoot(root);
    scene->Load();

    auto component =
        root->AddComponent<TComponent>();

    ASSERT_NE(component, nullptr);

    EXPECT_EQ(component->startCount, 0);
}


TEST_F(NodeTest, ComponentStartsWhenNodeStarts)
{
    auto scene =
        std::make_shared<Scene>("Game");

    scene->AddRoot(root);
    scene->Load();

    auto component =
        root->AddComponent<TComponent>();

    scene->SetActive(true);

    EXPECT_EQ(component->startCount, 1);
}


TEST_F(NodeTest, ComponentStartsOnlyOnce)
{
    auto scene =
        std::make_shared<Scene>("Game");

    scene->AddRoot(root);
    scene->Load();

    auto component =
        root->AddComponent<TComponent>();

    scene->SetActive(true);
    scene->SetActive(false);
    scene->SetActive(true);

    EXPECT_EQ(component->startCount, 1);
}


TEST_F(NodeTest, ComponentAddedAfterStartStartsImmediately)
{
    auto scene =
        std::make_shared<Scene>("Game");

    scene->AddRoot(root);
    scene->Load();
    scene->SetActive(true);

    auto component =
        root->AddComponent<TComponent>();

    ASSERT_NE(component, nullptr);

    EXPECT_EQ(component->startCount, 1);
}


TEST_F(NodeTest, ComponentUpdates)
{
    auto component =
        root->AddComponent<TComponent>();

    root->Update(0.016f);

    EXPECT_EQ(component->updateCount, 1);
    EXPECT_FLOAT_EQ(component->lastDeltaTime, 0.016f);
}


TEST_F(NodeTest, ComponentUpdatesEveryFrame)
{
    auto component =
        root->AddComponent<TComponent>();

    root->Update(0.016f);
    root->Update(0.016f);
    root->Update(0.016f);

    EXPECT_EQ(component->updateCount, 3);
}


TEST_F(NodeTest, NodeInitiallyActive)
{
    EXPECT_TRUE(root->IsActive());
}


TEST_F(NodeTest, DeactivateNode)
{
    root->SetActive(false);

    EXPECT_FALSE(root->IsActive());
}


TEST_F(NodeTest, ReactivateNode)
{
    root->SetActive(false);
    root->SetActive(true);

    EXPECT_TRUE(root->IsActive());
}


TEST_F(NodeTest, InactiveNodeDoesNotUpdate)
{
    auto component =
        root->AddComponent<TComponent>();

    root->SetActive(false);

    root->Update(0.016f);

    EXPECT_EQ(component->updateCount, 0);
}


TEST_F(NodeTest, TestComponentInitializesCorrectly)
{
    auto component =
        std::make_shared<TComponent>();

    ASSERT_NE(component, nullptr);

    EXPECT_FALSE(component->initialized);
    EXPECT_EQ(component->startCount, 0);
    EXPECT_EQ(component->updateCount, 0);
    EXPECT_FLOAT_EQ(component->lastDeltaTime, 0.0f);
}


class PlainTestObject
{
public:
    bool initialized = false;
    int startCount = 0;
    int updateCount = 0;
    float lastDeltaTime = 0.0f;
};


TEST_F(NodeTest, PlainTestObjectInitializesCorrectly)
{
    auto object =
        std::make_shared<PlainTestObject>();

    ASSERT_NE(object, nullptr);

    EXPECT_FALSE(object->initialized);
    EXPECT_EQ(object->startCount, 0);
    EXPECT_EQ(object->updateCount, 0);
    EXPECT_FLOAT_EQ(object->lastDeltaTime, 0.0f);
}