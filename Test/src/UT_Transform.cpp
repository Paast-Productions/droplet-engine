#include <gtest/gtest.h>
#include <cmath>
#include "Transform.hpp"

using namespace Droplet::Scene;

class Droplet::Scene::Node
{
public:
	Node()
	{
		m_parent = nullptr;
        std::shared_ptr<Node> self(this, [](Node*) {});
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

	std::shared_ptr<Node> m_node;

    void SetUp() override
    {
		m_node = std::make_shared<Node>();
    }
};

namespace
{
	constexpr float kEpsilon = 1e-5f;
	constexpr float kHalfPi = 1.57079632679f;

	std::shared_ptr<Node> CreateNode()
	{
		return std::make_shared<Node>();
	}

	void ExpectVec3Near(const glm::vec3 &actual, const glm::vec3 &expected)
	{
		EXPECT_NEAR(actual.x, expected.x, kEpsilon);
		EXPECT_NEAR(actual.y, expected.y, kEpsilon);
		EXPECT_NEAR(actual.z, expected.z, kEpsilon);
	}

	void ExpectQuatNear(const glm::quat &actual, const glm::quat &expected)
	{
		const bool directMatch =
			std::fabs(actual.x - expected.x) <= kEpsilon &&
			std::fabs(actual.y - expected.y) <= kEpsilon &&
			std::fabs(actual.z - expected.z) <= kEpsilon &&
			std::fabs(actual.w - expected.w) <= kEpsilon;

		const bool negatedMatch =
			std::fabs(actual.x + expected.x) <= kEpsilon &&
			std::fabs(actual.y + expected.y) <= kEpsilon &&
			std::fabs(actual.z + expected.z) <= kEpsilon &&
			std::fabs(actual.w + expected.w) <= kEpsilon;

		EXPECT_TRUE(directMatch || negatedMatch);
	}

	void ExpectMat4Near(const glm::mat4 &actual, const glm::mat4 &expected)
	{
		for (int column = 0; column < 4; ++column)
		{
			for (int row = 0; row < 4; ++row)
			{
				EXPECT_NEAR(actual[column][row], expected[column][row], kEpsilon);
			}
		}
	}
}

// Base

TEST_F(TransformTest, Construct)
{
	EXPECT_NE(m_node, nullptr);

	EXPECT_NE(m_node->GetTransform(), nullptr);
}

// Getters Local

TEST_F(TransformTest, GetPositionLocal)
{
	// TODO
}

TEST_F(TransformTest, GetRotationLocal)
{
	// TODO
}

TEST_F(TransformTest, GetScaleLocal)
{
	// TODO
}

TEST_F(TransformTest, GetEulerLocal)
{
	// TODO
}

TEST_F(TransformTest, GetAxesLocal)
{
	// TODO
}

TEST_F(TransformTest, GetMatrixLocal)
{
	// TODO
}

// Setters Local

TEST_F(TransformTest, SetPositionLocal)
{
	// TODO
}

TEST_F(TransformTest, SetRotationLocal)
{
	// TODO
}

TEST_F(TransformTest, SetScaleLocal)
{
	// TODO
}

TEST_F(TransformTest, SetEulerLocal)
{
	// TODO
}

TEST_F(TransformTest, SetMatrixLocal)
{
	// TODO
}

TEST_F(TransformTest, SettersCombinedLocal)
{
	// TODO
}

// Utility Local

TEST_F(TransformTest, MoveLocal)
{
	// TODO
}

TEST_F(TransformTest, RotateLocal)
{
	// TODO
}

TEST_F(TransformTest, RotateEulerLocal)
{
	// TODO
}

TEST_F(TransformTest, AddScaleLocal)
{
	// TODO
}

TEST_F(TransformTest, RotateAxisLocal)
{
	// TODO
}

TEST_F(TransformTest, LookAtLocal)
{
	// TODO
}

// Dirty Tracking

TEST_F(TransformTest, MarkDirtyAfterChange)
{
	// TODO
}

TEST_F(TransformTest, CleanAfterRecalcMatrices)
{
	// TODO
}

// Parenting

TEST_F(TransformTest, SetParent)
{
	std::shared_ptr<Node> parentNode = CreateNode();

	EXPECT_NE(parentNode, nullptr);

	m_node->SetParent(parentNode);

	EXPECT_EQ(m_node->GetParent(), parentNode);
}

TEST_F(TransformTest, InheritParentTransform)
{
	// TODO
}

TEST_F(TransformTest, ChangeParent)
{
	// TODO
}

// Getters World

TEST_F(TransformTest, GetPositionWorld)
{
	std::shared_ptr<Node> parentNode = CreateNode();
	m_node->SetParent(parentNode);

	// TODO
}

TEST_F(TransformTest, GetRotationWorld)
{
	// TODO
}

TEST_F(TransformTest, GetEulerWorld)
{
	// TODO
}

TEST_F(TransformTest, GetAxesWorld)
{
	// TODO
}

TEST_F(TransformTest, GetMatrixWorld)
{
	// TODO
}

// Setters World

TEST_F(TransformTest, SetPositionWorld)
{
	// TODO
}

TEST_F(TransformTest, SetRotationWorld)
{
	// TODO
}

TEST_F(TransformTest, SetEulerWorld)
{
	// TODO
}

TEST_F(TransformTest, SetMatrixWorld)
{
	// TODO
}

TEST_F(TransformTest, SettersCombinedWorld)
{
	// TODO
}

// Utility World

TEST_F(TransformTest, MoveWorld)
{
	// TODO
}

TEST_F(TransformTest, RotateWorld)
{
	// TODO
}

TEST_F(TransformTest, RotateEulerWorld)
{
	// TODO
}

TEST_F(TransformTest, RotateAxisWorld)
{
	// TODO
}

TEST_F(TransformTest, LookAtWorld)
{
	// TODO
}
