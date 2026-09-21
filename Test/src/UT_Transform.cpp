#include <gtest/gtest.h>
#include "Transform.hpp"

using namespace Droplet::Scene;

class Droplet::Scene::Node
{
public:
	Node()
	{
		m_parent = nullptr;
		std::shared_ptr<Node> self = (std::shared_ptr<Node>)(this);
		m_transform = std::make_unique<Transform>(self);
	}

	std::shared_ptr<Node> GetParent() const
	{
		return m_parent;
	}

	std::unique_ptr<Transform> &GetTransform()
	{
		return m_transform;
	}

	void SetParent(std::shared_ptr<Node> parent)
	{
		m_parent = parent;
	}

	void SetTransform(std::unique_ptr<Transform> transform)
	{
		m_transform = std::move(transform);
	}

private:
	std::shared_ptr<Node> m_parent;
	std::unique_ptr<Transform> m_transform;
};

class TransformTest : public ::testing::Test
{
protected:

	std::unique_ptr<Node> m_node;

    void SetUp() override
    {
		m_node = std::make_unique<Node>();
    }
};

TEST_F(TransformTest, Construct)
{
	EXPECT_NE(m_node, nullptr);

	EXPECT_NE(m_node->GetTransform(), nullptr);
}
