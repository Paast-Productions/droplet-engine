#include <gtest/gtest.h>

#include "SceneSystem/Node.hpp"
#include "SceneSystem/Scene.hpp"
#include "SceneSystem/Component.hpp"

using namespace Droplet::Scene;

class TComponent : public Component
{
public:
    void Start() override
    {
        startCount++;
    }

    void Update(float p_deltaTime) override
    {
        updateCount++;
        lastDeltaTime = p_deltaTime;
    }

    int startCount = 0;
    int updateCount = 0;
    float lastDeltaTime = 0.0f;
};


class NodeTest : public ::testing::Test
{
protected:

    std::shared_ptr<Scene> scene;
    std::shared_ptr<Node> root;

    void SetUp() override
    {
		scene = std::make_shared<Scene>("Game");
		root = scene->AddNode("Root");
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
            scene->AddNode("Player"));

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


TEST_F(NodeTest, AddChildWithParent)
{
    auto parent1 =
        scene->AddNode("Parent1");

    auto parent2 =
        scene->AddNode("Parent2");

    auto child =
        scene->AddNode("Child");

    parent1->AddChild(child);

	EXPECT_EQ(child->GetParent(), parent1);

	parent2->AddChild(child);

	EXPECT_EQ(child->GetParent(), parent2);
	EXPECT_EQ(parent1->GetChildren().size(), 0);
	EXPECT_EQ(parent2->GetChildren().size(), 1);
	EXPECT_EQ(parent2->GetChildren()[0], child);
}


TEST_F(NodeTest, CannotAddNodeToItself)
{
    EXPECT_THROW(
        root->AddChild(root),
        std::runtime_error);
}


TEST_F(NodeTest, CannotSetGrandchildToParent)
{
	auto child =
		root->AddChild(
            scene->AddNode("Child"));

    EXPECT_THROW(
        child->AddChild(root),
        std::runtime_error);
}


TEST_F(NodeTest, ChildHasCorrectParent)
{
    auto player =
        root->AddChild(
            scene->AddNode("Player"));

    ASSERT_NE(player, nullptr);

    EXPECT_EQ(player->GetParent(), root);
}


TEST_F(NodeTest, RemoveChild)
{
    auto player =
        root->AddChild(
            scene->AddNode("Player"));

    ASSERT_NE(player, nullptr);

    EXPECT_NO_THROW(
        root->RemoveChild(player));

    EXPECT_TRUE(root->GetChildren().empty());
    EXPECT_EQ(player->GetParent(), nullptr);
}


TEST_F(NodeTest, RemoveNonExistentChildThrows)
{
    auto player =
        scene->AddNode("Player");

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


TEST_F(NodeTest, SceneReference)
{
    scene->Load();

    EXPECT_EQ(root->GetScene(), scene);
}


TEST_F(NodeTest, ChildInheritsSceneReference)
{
    scene->Load();

    auto player =
        root->AddChild(
            scene->AddNode("Player"));

    ASSERT_NE(player, nullptr);

    EXPECT_EQ(player->GetScene(), scene);
}


TEST_F(NodeTest, GrandchildInheritsSceneReference)
{
    scene->Load();

    auto player =
        root->AddChild(
            scene->AddNode("Player"));

    auto weapon =
        player->AddChild(
            scene->AddNode("Weapon"));

    ASSERT_NE(weapon, nullptr);

    EXPECT_EQ(weapon->GetScene(), scene);
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


TEST_F(NodeTest, RemoveNonExistentComponentThrows)
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
    scene->Load();

    auto component =
        root->AddComponent<TComponent>();

    ASSERT_NE(component, nullptr);

    EXPECT_EQ(component->startCount, 0);
}


TEST_F(NodeTest, ComponentStartsWhenNodeStarts)
{
    scene->Load();

    auto component =
        root->AddComponent<TComponent>();

    scene->SetActive(true);

    EXPECT_EQ(component->startCount, 1);
}


TEST_F(NodeTest, ComponentStartsOnlyOnce)
{
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

    EXPECT_FALSE(root->IsActive());

    root->SetActive(true);

    EXPECT_TRUE(root->IsActive());
}


TEST_F(NodeTest, ChildInheritsActiveState)
{
	auto child =
		root->AddChild(
			scene->AddNode("Child"));

	ASSERT_NE(child, nullptr);
	EXPECT_TRUE(child->IsActive());

	root->SetActive(false);

	EXPECT_FALSE(child->IsActive());
	EXPECT_TRUE(child->IsActiveSelf());

	root->SetActive(true);

	EXPECT_TRUE(child->IsActive());
	EXPECT_TRUE(child->IsActiveSelf());
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