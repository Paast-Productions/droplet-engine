#include <gtest/gtest.h>

#include "SceneSystem/Component.hpp"
#include "SceneSystem/Node.hpp"


class ComponentTest : public ::testing::Test
{
protected:

    std::shared_ptr<Node> node;

    void SetUp() override
    {
        node =
            std::make_shared<Node>("TestNode");
    }
};

class TestComponent : public Component
{
};

TEST_F(ComponentTest, OwnerInitiallyNull)
{
    auto component =
        std::make_shared<TestComponent>();

    EXPECT_EQ(component->GetOwner(), nullptr);
}

TEST_F(ComponentTest, OwnerAssignedWhenAdded)
{
    auto component =
        node->AddComponent<TestComponent>();

    ASSERT_NE(component, nullptr);

    EXPECT_EQ(component->GetOwner(), node);
}