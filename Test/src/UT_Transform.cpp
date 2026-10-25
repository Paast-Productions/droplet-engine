#include <gtest/gtest.h>
#include <Transform.hpp>
#include <SceneSystem/Node.hpp>
#include <SceneSystem/Scene.hpp>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <glm/gtx/quaternion.hpp>

using namespace Droplet::Scene;

class TransformTest : public ::testing::Test
{
protected:

	std::shared_ptr<Scene> m_scene;
	std::shared_ptr<Node> m_node;

    void SetUp() override
    {
		m_scene = std::make_shared<Scene>("Game");
		m_scene->Load();
		m_node = m_scene->AddNode("Node");
    }
};

namespace
{
	constexpr float kEpsilon = 1e-5f;
	constexpr float kHalfPi = 1.57079632679f;

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
}

TEST(TransformConstructionTest, RejectsNullOwner)
{
	EXPECT_THROW(Transform(nullptr), std::invalid_argument);
}

// Getters Local

TEST_F(TransformTest, GetPositionLocal)
{
	ExpectVec3Near(m_node->GetTransform().GetPosition(), glm::vec3(0.f, 0.f, 0.f));
}

TEST_F(TransformTest, GetRotationLocal)
{
	ExpectQuatNear(m_node->GetTransform().GetRotation(), glm::quat(glm::vec3(0.f, 0.f, 0.f)));
}

TEST_F(TransformTest, GetScaleLocal)
{
	ExpectVec3Near(m_node->GetTransform().GetScale(), glm::vec3(1.f, 1.f, 1.f));
}

TEST_F(TransformTest, GetEulerLocal)
{
	ExpectVec3Near(m_node->GetTransform().GetEuler(), glm::vec3(0.f, 0.f, 0.f));
}

TEST_F(TransformTest, GetAxesLocal)
{
	ExpectVec3Near(m_node->GetTransform().GetRight(), glm::vec3(1.f, 0.f, 0.f));
	ExpectVec3Near(m_node->GetTransform().GetUp(), glm::vec3(0.f, 1.f, 0.f));
	ExpectVec3Near(m_node->GetTransform().GetForward(), glm::vec3(0.f, 0.f, 1.f));
}

TEST_F(TransformTest, GetMatrixLocal)
{
	const glm::vec3 pos(2.f, 3.f, -4.f);
	const glm::quat rot = glm::angleAxis(kHalfPi, glm::vec3(0.f, 1.f, 0.f));
	const glm::vec3 scale(1.5f, 2.f, 0.5f);

	m_node->GetTransform().SetPosition(pos);
	m_node->GetTransform().SetRotation(rot);
	m_node->GetTransform().SetScale(scale);
	m_node->GetTransform().RecalculateMatrices();

	const glm::mat4 expected =
		glm::translate(glm::mat4(1.f), pos) *
		glm::toMat4(rot) *
		glm::scale(glm::mat4(1.f), scale);

	ExpectMat4Near(m_node->GetTransform().GetMatrix(), expected);
}

// Setters Local

TEST_F(TransformTest, SetPositionLocal)
{
	const glm::vec3 target(1.f, -2.f, 3.f);
	m_node->GetTransform().SetPosition(target);
	ExpectVec3Near(m_node->GetTransform().GetPosition(), target);
}

TEST_F(TransformTest, SetRotationLocal)
{
	const glm::quat target = glm::angleAxis(kHalfPi, glm::vec3(0.f, 0.f, 1.f));
	m_node->GetTransform().SetRotation(target);
	ExpectQuatNear(m_node->GetTransform().GetRotation(), target);
}

TEST_F(TransformTest, SetScaleLocal)
{
	const glm::vec3 target(2.f, 3.f, 4.f);
	m_node->GetTransform().SetScale(target);
	ExpectVec3Near(m_node->GetTransform().GetScale(), target);
}

TEST_F(TransformTest, SetEulerLocal)
{
	const glm::vec3 euler(0.f, kHalfPi, 0.f);
	m_node->GetTransform().SetEuler(euler);
	ExpectQuatNear(m_node->GetTransform().GetRotation(), glm::quat(euler));
}

TEST_F(TransformTest, SetMatrixLocal)
{
	const glm::vec3 pos(2.f, -1.f, 3.f);
	const glm::quat rot = glm::angleAxis(kHalfPi, glm::vec3(1.f, 0.f, 0.f));
	const glm::vec3 scale(2.f, 1.5f, 0.5f);
	const glm::mat4 matrix =
		glm::translate(glm::mat4(1.f), pos) *
		glm::toMat4(rot) *
		glm::scale(glm::mat4(1.f), scale);

	m_node->GetTransform().SetMatrix(matrix);
	m_node->GetTransform().RecalculateMatrices();

	ExpectVec3Near(m_node->GetTransform().GetPosition(), pos);
	ExpectQuatNear(m_node->GetTransform().GetRotation(), rot);
	ExpectVec3Near(m_node->GetTransform().GetScale(), scale);
	ExpectMat4Near(m_node->GetTransform().GetMatrix(), matrix);
}

TEST_F(TransformTest, SetMatrixRejectsInvalidOrShearedInputWithoutChangingState)
{
	auto &transform = m_node->GetTransform();
	const glm::vec3 position(1.f, 2.f, 3.f);
	const glm::quat rotation = glm::angleAxis(kHalfPi, glm::vec3(0.f, 1.f, 0.f));
	const glm::vec3 scale(2.f, 3.f, 4.f);
	transform.SetPosition(position);
	transform.SetRotation(rotation);
	transform.SetScale(scale);
	transform.RecalculateMatrices();

	glm::mat4 invalidMatrix(1.f);
	invalidMatrix[0][0] = 0.f; 
	transform.SetMatrix(invalidMatrix);
	ExpectVec3Near(transform.GetPosition(), position);
	ExpectQuatNear(transform.GetRotation(), rotation);
	ExpectVec3Near(transform.GetScale(), scale);
	EXPECT_FALSE(transform.IsDirty());

	invalidMatrix[0][0] = std::numeric_limits<float>::quiet_NaN();
	transform.SetMatrix(invalidMatrix);
	ExpectVec3Near(transform.GetPosition(), position);
	ExpectQuatNear(transform.GetRotation(), rotation);
	ExpectVec3Near(transform.GetScale(), scale);
	EXPECT_FALSE(transform.IsDirty());

	glm::mat4 shearedMatrix(1.f);
	shearedMatrix[1][0] = 0.5f;
	transform.SetMatrix(shearedMatrix);
	ExpectVec3Near(transform.GetPosition(), position);
	ExpectQuatNear(transform.GetRotation(), rotation);
	ExpectVec3Near(transform.GetScale(), scale);
	EXPECT_FALSE(transform.IsDirty());
}

TEST_F(TransformTest, SettersCombinedLocal)
{
	const glm::vec3 pos(-3.f, 2.f, 1.f);
	const glm::vec3 euler(0.f, 0.f, kHalfPi);
	const glm::vec3 scale(3.f, 2.f, 1.f);

	m_node->GetTransform().SetPosition(pos);
	m_node->GetTransform().SetEuler(euler);
	m_node->GetTransform().SetScale(scale);
	m_node->GetTransform().RecalculateMatrices();

	ExpectVec3Near(m_node->GetTransform().GetPosition(), pos);
	ExpectQuatNear(m_node->GetTransform().GetRotation(), glm::quat(euler));
	ExpectVec3Near(m_node->GetTransform().GetScale(), scale);
}

// Utility Local

TEST_F(TransformTest, MoveLocal)
{
	m_node->GetTransform().SetPosition(glm::vec3(1.f, 2.f, 3.f));
	m_node->GetTransform().Move(glm::vec3(4.f, -1.f, 0.5f));
	ExpectVec3Near(m_node->GetTransform().GetPosition(), glm::vec3(5.f, 1.f, 3.5f));
}

TEST_F(TransformTest, RotateLocal)
{
	m_node->GetTransform().SetRotation(glm::quat(glm::vec3(0.f, 0.f, 0.f)));
	const glm::quat delta = glm::angleAxis(kHalfPi, glm::vec3(0.f, 1.f, 0.f));
	m_node->GetTransform().Rotate(delta);
	ExpectQuatNear(m_node->GetTransform().GetRotation(), delta);
}

TEST_F(TransformTest, RotateEulerLocal)
{
	m_node->GetTransform().SetRotation(glm::quat(glm::vec3(0.f, 0.f, 0.f)));
	const glm::vec3 delta(0.f, kHalfPi, 0.f);
	m_node->GetTransform().RotateEuler(delta);
	ExpectQuatNear(m_node->GetTransform().GetRotation(), glm::quat(delta));
}

TEST_F(TransformTest, AddScaleLocal)
{
	m_node->GetTransform().SetScale(glm::vec3(1.f, 2.f, 3.f));
	m_node->GetTransform().AddScale(glm::vec3(0.5f, 1.f, -1.f));

	ExpectVec3Near(m_node->GetTransform().GetScale(), glm::vec3(1.5f, 3.f, 2.f));
}

TEST_F(TransformTest, RotateAxisLocal)
{
	m_node->GetTransform().SetRotation(glm::quat(glm::vec3(0.f, 0.f, 0.f)));
	m_node->GetTransform().RotateAxis(kHalfPi, glm::vec3(0.f, 0.f, 1.f));
	ExpectQuatNear(m_node->GetTransform().GetRotation(), glm::angleAxis(kHalfPi, glm::vec3(0.f, 0.f, 1.f)));
}

TEST_F(TransformTest, LookAtLocal)
{
	m_node->GetTransform().SetPosition(glm::vec3(0.f, 0.f, 0.f));
	const glm::vec3 target(1.f, 0.f, 0.f);

	m_node->GetTransform().LookAt(target);
	ExpectQuatNear(
		m_node->GetTransform().GetRotation(),
		glm::quat_cast(glm::inverse(glm::lookAt(glm::vec3(0.f), -target, glm::vec3(0.f, 1.f, 0.f)))));
	ExpectVec3Near(m_node->GetTransform().GetForward(), glm::normalize(target));
}

TEST_F(TransformTest, LookAtDegenerateInputsPreserveRotation)
{
	auto &transform = m_node->GetTransform();
	const glm::quat initialRotation = glm::angleAxis(0.7f, glm::normalize(glm::vec3(1.f, 2.f, 3.f)));
	transform.SetRotation(initialRotation);

	transform.LookAt(transform.GetPosition());
	ExpectQuatNear(transform.GetRotation(), initialRotation);

	transform.LookAt(glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 0.f, 2.f));
	ExpectQuatNear(transform.GetRotation(), initialRotation);
}

// Parenting

TEST_F(TransformTest, SetParent)
{
	std::shared_ptr<Node> parentNode = m_scene->AddNode("Node");

	EXPECT_NE(parentNode, nullptr);

	parentNode.get()->AddChild(m_node);

	EXPECT_EQ(m_node->GetParent(), parentNode);
}

TEST_F(TransformTest, InheritParentTransform)
{
	std::shared_ptr<Node> parentNode = m_scene->AddNode("Node");
	parentNode->GetTransform().SetPosition(glm::vec3(10.f, 0.f, 0.f));
	parentNode->GetTransform().RecalculateMatrices();

	parentNode.get()->AddChild(m_node);
	m_node->GetTransform().SetPosition(glm::vec3(1.f, 2.f, 3.f));
	m_node->GetTransform().RecalculateMatrices();

	ExpectVec3Near(m_node->GetTransform().GetPosition(Transform::Space::World), glm::vec3(11.f, 2.f, 3.f));
}

TEST_F(TransformTest, ChangeParent)
{
	std::shared_ptr<Node> parentA = m_scene->AddNode("Node");
	std::shared_ptr<Node> parentB = m_scene->AddNode("Node");

	parentA->GetTransform().SetPosition(glm::vec3(2.f, 0.f, 0.f));
	parentB->GetTransform().SetPosition(glm::vec3(5.f, 0.f, 0.f));
	parentA->GetTransform().RecalculateMatrices();
	parentB->GetTransform().RecalculateMatrices();

	parentA.get()->AddChild(m_node);
	m_node->GetTransform().SetPosition(glm::vec3(1.f, 0.f, 0.f));
	m_node->GetTransform().RecalculateMatrices();
	ExpectVec3Near(m_node->GetTransform().GetPosition(Transform::Space::World), glm::vec3(3.f, 0.f, 0.f));

	parentB.get()->AddChild(m_node);
	m_node->GetTransform().RecalculateMatrices();
	ExpectVec3Near(m_node->GetTransform().GetPosition(Transform::Space::World), glm::vec3(6.f, 0.f, 0.f));
}

// Getters World

TEST_F(TransformTest, GetPositionWorld)
{
	std::shared_ptr<Node> parentNode = m_scene->AddNode("Node");
	parentNode.get()->AddChild(m_node);

	parentNode->GetTransform().SetPosition(glm::vec3(10.f, -2.f, 5.f));
	parentNode->GetTransform().RecalculateMatrices();

	m_node->GetTransform().SetPosition(glm::vec3(1.f, 2.f, 3.f));
	ExpectVec3Near(m_node->GetTransform().GetPosition(Transform::Space::World), glm::vec3(11.f, 0.f, 8.f));
}

TEST_F(TransformTest, GetRotationWorld)
{
	std::shared_ptr<Node> parentNode = m_scene->AddNode("Node");
	parentNode.get()->AddChild(m_node);

	const glm::quat parentRot = glm::quat(glm::vec3(0.3f, 0.4f, 0.2f));
	const glm::quat localRot = glm::angleAxis(kHalfPi, glm::vec3(1.f, 0.f, 0.f));
	parentNode->GetTransform().SetRotation(parentRot);
	parentNode->GetTransform().RecalculateMatrices();

	m_node->GetTransform().SetRotation(localRot);
	ExpectQuatNear(m_node->GetTransform().GetRotation(Transform::Space::World), parentRot * localRot);
}

TEST_F(TransformTest, WorldRotationIgnoresParentScale)
{
	auto parentNode = m_scene->AddNode("Parent");
	parentNode->AddChild(m_node);

	const glm::quat parentRotation = glm::angleAxis(0.6f, glm::normalize(glm::vec3(1.f, 2.f, 0.5f)));
	const glm::quat localRotation = glm::angleAxis(-0.4f, glm::vec3(0.f, 1.f, 0.f));
	const glm::quat desiredWorldRotation = glm::angleAxis(0.9f, glm::vec3(1.f, 0.f, 0.f));
	parentNode->GetTransform().SetRotation(parentRotation);
	parentNode->GetTransform().SetScale(glm::vec3(2.f, 0.5f, 3.f));
	parentNode->GetTransform().RecalculateMatrices();

	auto &transform = m_node->GetTransform();
	transform.SetRotation(localRotation);
	ExpectQuatNear(transform.GetRotation(Transform::Space::World), parentRotation * localRotation);

	transform.SetRotation(desiredWorldRotation, Transform::Space::World);
	ExpectQuatNear(transform.GetRotation(Transform::Space::World), desiredWorldRotation);

	const glm::quat worldDelta = glm::angleAxis(0.3f, glm::vec3(0.f, 0.f, 1.f));
	transform.Rotate(worldDelta, Transform::Space::World);
	ExpectQuatNear(transform.GetRotation(Transform::Space::World), worldDelta * desiredWorldRotation);

	const glm::vec3 worldEuler(0.2f, -0.4f, 0.7f);
	transform.SetEuler(worldEuler, Transform::Space::World);
	ExpectQuatNear(transform.GetRotation(Transform::Space::World), glm::quat(worldEuler));

	const glm::vec3 eulerDelta(-0.1f, 0.3f, 0.2f);
	transform.RotateEuler(eulerDelta, Transform::Space::World);
	ExpectQuatNear(
		transform.GetRotation(Transform::Space::World),
		glm::quat(eulerDelta) * glm::quat(worldEuler));

	const glm::quat axisDelta = glm::angleAxis(-0.2f, glm::vec3(0.f, 1.f, 0.f));
	transform.RotateAxis(-0.2f, glm::vec3(0.f, 1.f, 0.f), Transform::Space::World);
	ExpectQuatNear(
		transform.GetRotation(Transform::Space::World),
		axisDelta * glm::quat(eulerDelta) * glm::quat(worldEuler));
}

TEST_F(TransformTest, GetEulerWorld)
{
	std::shared_ptr<Node> parentNode = m_scene->AddNode("Node");
	parentNode.get()->AddChild(m_node);

	const glm::quat parentRot = glm::quat(glm::vec3(0.3f, 0.4f, 0.2f));
	parentNode->GetTransform().SetRotation(parentRot);
	parentNode->GetTransform().RecalculateMatrices();

	ExpectVec3Near(
		m_node->GetTransform().GetEuler(Transform::Space::World),
		glm::eulerAngles(m_node->GetTransform().GetRotation(Transform::Space::World)));
}

TEST_F(TransformTest, GetAxesWorld)
{
	std::shared_ptr<Node> parentNode = m_scene->AddNode("Node");
	parentNode.get()->AddChild(m_node);

	const glm::quat parentRot = glm::angleAxis(kHalfPi, glm::vec3(0.f, 0.f, 1.f));
	parentNode->GetTransform().SetRotation(parentRot);
	parentNode->GetTransform().RecalculateMatrices();

	ExpectVec3Near(m_node->GetTransform().GetRight(Transform::Space::World), glm::normalize(parentRot * glm::vec3(1.f, 0.f, 0.f)));
	ExpectVec3Near(m_node->GetTransform().GetUp(Transform::Space::World), glm::normalize(parentRot * glm::vec3(0.f, 1.f, 0.f)));
	ExpectVec3Near(m_node->GetTransform().GetForward(Transform::Space::World), glm::normalize(parentRot * glm::vec3(0.f, 0.f, 1.f)));
}

TEST_F(TransformTest, GetMatrixWorld)
{
	std::shared_ptr<Node> parentNode = m_scene->AddNode("Node");
	parentNode.get()->AddChild(m_node);

	parentNode->GetTransform().SetPosition(glm::vec3(5.f, 0.f, 0.f));
	parentNode->GetTransform().SetRotation(glm::angleAxis(kHalfPi, glm::vec3(0.f, 1.f, 0.f)));
	parentNode->GetTransform().RecalculateMatrices();

	m_node->GetTransform().SetPosition(glm::vec3(1.f, 2.f, 3.f));
	m_node->GetTransform().SetScale(glm::vec3(2.f, 1.f, 1.f));
	m_node->GetTransform().RecalculateMatrices();

	const glm::mat4 expected =
		parentNode->GetTransform().GetMatrix() *
		m_node->GetTransform().GetMatrix();

	ExpectMat4Near(m_node->GetTransform().GetMatrix(Transform::Space::World), expected);
}

// Setters World

TEST_F(TransformTest, SetPositionWorld)
{
	std::shared_ptr<Node> parentNode = m_scene->AddNode("Node");
	parentNode.get()->AddChild(m_node);

	parentNode->GetTransform().SetPosition(glm::vec3(10.f, 0.f, 0.f));
	parentNode->GetTransform().RecalculateMatrices();

	m_node->GetTransform().SetPosition(glm::vec3(13.f, 2.f, 1.f), Transform::Space::World);
	ExpectVec3Near(m_node->GetTransform().GetPosition(), glm::vec3(3.f, 2.f, 1.f));
	ExpectVec3Near(m_node->GetTransform().GetPosition(Transform::Space::World), glm::vec3(13.f, 2.f, 1.f));
}

TEST_F(TransformTest, SetRotationWorld)
{
	std::shared_ptr<Node> parentNode = m_scene->AddNode("Node");
	parentNode.get()->AddChild(m_node);

	const glm::quat parentRot = glm::angleAxis(kHalfPi, glm::vec3(0.f, 1.f, 0.f));
	const glm::quat worldRot = glm::angleAxis(kHalfPi, glm::vec3(1.f, 0.f, 0.f));
	parentNode->GetTransform().SetRotation(parentRot);
	parentNode->GetTransform().RecalculateMatrices();

	m_node->GetTransform().SetRotation(worldRot, Transform::Space::World);
	ExpectQuatNear(m_node->GetTransform().GetRotation(), glm::inverse(parentRot) * worldRot);
	ExpectQuatNear(m_node->GetTransform().GetRotation(Transform::Space::World), worldRot);
}

TEST_F(TransformTest, SetEulerWorld)
{
	std::shared_ptr<Node> parentNode = m_scene->AddNode("Node");
	parentNode.get()->AddChild(m_node);

	const glm::quat parentRot = glm::angleAxis(kHalfPi, glm::vec3(0.f, 1.f, 0.f));
	const glm::vec3 worldEuler(0.f, 0.f, kHalfPi);
	parentNode->GetTransform().SetRotation(parentRot);
	parentNode->GetTransform().RecalculateMatrices();

	m_node->GetTransform().SetEuler(worldEuler, Transform::Space::World);
	ExpectQuatNear(m_node->GetTransform().GetRotation(), glm::inverse(parentRot) * glm::quat(worldEuler));
	ExpectQuatNear(m_node->GetTransform().GetRotation(Transform::Space::World), glm::quat(worldEuler));
}

TEST_F(TransformTest, SetMatrixWorld)
{
	std::shared_ptr<Node> parentNode = m_scene->AddNode("Node");
	parentNode.get()->AddChild(m_node);
	parentNode->GetTransform().SetPosition(glm::vec3(10.f, 0.f, 0.f));
	parentNode->GetTransform().RecalculateMatrices();

	const glm::vec3 worldPos(12.f, 3.f, 4.f);
	const glm::quat worldRot = glm::angleAxis(kHalfPi, glm::vec3(0.f, 0.f, 1.f));
	const glm::vec3 worldScale(2.f, 2.f, 2.f);
	const glm::mat4 worldMatrix =
		glm::translate(glm::mat4(1.f), worldPos) *
		glm::toMat4(worldRot) *
		glm::scale(glm::mat4(1.f), worldScale);

	m_node->GetTransform().SetMatrix(worldMatrix, Transform::Space::World);
	m_node->GetTransform().RecalculateMatrices();

	ExpectVec3Near(m_node->GetTransform().GetPosition(), glm::vec3(2.f, 3.f, 4.f));
	ExpectMat4Near(m_node->GetTransform().GetMatrix(Transform::Space::World), worldMatrix);
}

TEST_F(TransformTest, SetWorldMatrixRejectsUnrepresentableParentShear)
{
	auto parentNode = m_scene->AddNode("Parent");
	parentNode->AddChild(m_node);
	parentNode->GetTransform().SetRotation(glm::angleAxis(0.5f, glm::vec3(0.f, 0.f, 1.f)));
	parentNode->GetTransform().SetScale(glm::vec3(2.f, 1.f, 1.f));
	parentNode->GetTransform().RecalculateMatrices();
	m_node->GetTransform().RecalculateMatrices();

	m_node->GetTransform().SetMatrix(glm::mat4(1.f), Transform::Space::World);

	EXPECT_FALSE(m_node->GetTransform().IsDirty());
}

TEST_F(TransformTest, SettersCombinedWorld)
{
	std::shared_ptr<Node> parentNode = m_scene->AddNode("Node");
	parentNode.get()->AddChild(m_node);
	parentNode->GetTransform().SetPosition(glm::vec3(5.f, 0.f, 0.f));
	parentNode->GetTransform().SetRotation(glm::angleAxis(kHalfPi, glm::vec3(0.f, 0.f, 1.f)));
	parentNode->GetTransform().RecalculateMatrices();

	const glm::vec3 worldPos(7.f, 2.f, 0.f);
	const glm::quat worldRot = glm::angleAxis(kHalfPi, glm::vec3(1.f, 0.f, 0.f));
	m_node->GetTransform().SetPosition(worldPos, Transform::Space::World);
	m_node->GetTransform().SetRotation(worldRot, Transform::Space::World);

	ExpectVec3Near(m_node->GetTransform().GetPosition(Transform::Space::World), worldPos);
	ExpectQuatNear(m_node->GetTransform().GetRotation(Transform::Space::World), worldRot);
}

// Utility World

TEST_F(TransformTest, MoveWorld)
{
	std::shared_ptr<Node> parentNode = m_scene->AddNode("Node");
	parentNode.get()->AddChild(m_node);
	parentNode->GetTransform().SetPosition(glm::vec3(10.f, 0.f, 0.f));
	parentNode->GetTransform().RecalculateMatrices();

	m_node->GetTransform().SetPosition(glm::vec3(1.f, 0.f, 0.f));
	m_node->GetTransform().Move(glm::vec3(2.f, 3.f, 4.f), Transform::Space::World);
	ExpectVec3Near(m_node->GetTransform().GetPosition(), glm::vec3(3.f, 3.f, 4.f));
	ExpectVec3Near(m_node->GetTransform().GetPosition(Transform::Space::World), glm::vec3(13.f, 3.f, 4.f));
}

TEST_F(TransformTest, RotateWorld)
{
	std::shared_ptr<Node> parentNode = m_scene->AddNode("Node");
	parentNode.get()->AddChild(m_node);

	const glm::quat parentRot = glm::angleAxis(kHalfPi, glm::vec3(0.f, 1.f, 0.f));
	const glm::quat delta = glm::angleAxis(kHalfPi, glm::vec3(1.f, 0.f, 0.f));
	parentNode->GetTransform().SetRotation(parentRot);
	parentNode->GetTransform().RecalculateMatrices();

	m_node->GetTransform().Rotate(delta, Transform::Space::World);
	ExpectQuatNear(m_node->GetTransform().GetRotation(Transform::Space::World), delta * parentRot);
}

TEST_F(TransformTest, SlerpRotationClampsEndpointsAndTakesShortestPath)
{
	auto &transform = m_node->GetTransform();
	const glm::quat initial = glm::angleAxis(0.3f, glm::vec3(1.f, 0.f, 0.f));
	const glm::quat target = glm::angleAxis(kHalfPi, glm::vec3(0.f, 1.f, 0.f));

	transform.SetRotation(initial);
	transform.SlerpRotation(target, 0.f);
	ExpectQuatNear(transform.GetRotation(), initial);

	transform.SlerpRotation(target, 2.f);
	ExpectQuatNear(transform.GetRotation(), target);

	transform.SetRotation(glm::quat(1.f, 0.f, 0.f, 0.f));
	transform.SlerpRotation(-glm::quat(1.f, 0.f, 0.f, 0.f), 0.5f);
	ExpectQuatNear(transform.GetRotation(), glm::quat(1.f, 0.f, 0.f, 0.f));
}

TEST_F(TransformTest, SlerpRotationUsesWorldSpaceWithScaledParent)
{
	auto parentNode = m_scene->AddNode("Parent");
	parentNode->AddChild(m_node);

	const glm::quat parentRotation = glm::angleAxis(0.6f, glm::vec3(0.f, 1.f, 0.f));
	const glm::quat initialWorldRotation = glm::angleAxis(-0.3f, glm::vec3(1.f, 0.f, 0.f));
	const glm::quat targetWorldRotation = glm::angleAxis(0.8f, glm::vec3(0.f, 0.f, 1.f));
	parentNode->GetTransform().SetRotation(parentRotation);
	parentNode->GetTransform().SetScale(glm::vec3(2.f, 0.5f, 3.f));
	parentNode->GetTransform().RecalculateMatrices();

	auto &transform = m_node->GetTransform();
	transform.SetRotation(initialWorldRotation, Transform::Space::World);
	transform.SlerpRotation(targetWorldRotation, 0.5f, Transform::Space::World);

	ExpectQuatNear(
		transform.GetRotation(Transform::Space::World),
		glm::normalize(glm::slerp(initialWorldRotation, targetWorldRotation, 0.5f)));
}

TEST_F(TransformTest, SlerpRotationRejectsInvalidInputsWithoutChangingState)
{
	auto &transform = m_node->GetTransform();
	const glm::quat initial = glm::angleAxis(0.3f, glm::vec3(1.f, 0.f, 0.f));
	transform.SetRotation(initial);
	transform.RecalculateMatrices();

	transform.SlerpRotation(glm::quat(0.f, 0.f, 0.f, 0.f), 0.5f);
	ExpectQuatNear(transform.GetRotation(), initial);
	EXPECT_FALSE(transform.IsDirty());

	transform.SlerpRotation(glm::quat(1.f, 0.f, 0.f, 0.f), std::numeric_limits<float>::quiet_NaN());
	ExpectQuatNear(transform.GetRotation(), initial);
	EXPECT_FALSE(transform.IsDirty());
}

TEST_F(TransformTest, RotateEulerWorld)
{
	std::shared_ptr<Node> parentNode = m_scene->AddNode("Node");
	parentNode.get()->AddChild(m_node);

	const glm::quat parentRot = glm::angleAxis(kHalfPi, glm::vec3(0.f, 1.f, 0.f));
	const glm::vec3 deltaEuler(0.f, 0.f, kHalfPi);
	parentNode->GetTransform().SetRotation(parentRot);
	parentNode->GetTransform().RecalculateMatrices();

	m_node->GetTransform().RotateEuler(deltaEuler, Transform::Space::World);
	ExpectQuatNear(m_node->GetTransform().GetRotation(Transform::Space::World), glm::quat(deltaEuler) * parentRot);
}

TEST_F(TransformTest, RotateAxisWorld)
{
	std::shared_ptr<Node> parentNode = m_scene->AddNode("Node");
	parentNode.get()->AddChild(m_node);

	const glm::quat parentRot = glm::angleAxis(kHalfPi, glm::vec3(0.f, 1.f, 0.f));
	parentNode->GetTransform().SetRotation(parentRot);
	parentNode->GetTransform().RecalculateMatrices();

	const glm::quat delta = glm::angleAxis(kHalfPi, glm::vec3(0.f, 0.f, 1.f));
	m_node->GetTransform().RotateAxis(kHalfPi, glm::vec3(0.f, 0.f, 1.f), Transform::Space::World);
	ExpectQuatNear(m_node->GetTransform().GetRotation(Transform::Space::World), delta * parentRot);
}

TEST_F(TransformTest, LookAtWorld)
{
	std::shared_ptr<Node> parentNode = m_scene->AddNode("Node");
	parentNode.get()->AddChild(m_node);
	parentNode->GetTransform().SetPosition(glm::vec3(5.f, 0.f, 0.f));
	parentNode->GetTransform().RecalculateMatrices();

	m_node->GetTransform().SetPosition(glm::vec3(1.f, 0.f, 0.f));
	const glm::vec3 targetWorld(7.f, 0.f, 0.f);
	m_node->GetTransform().LookAt(targetWorld, glm::vec3(0.f, 1.f, 0.f), Transform::Space::World);
	m_node->GetTransform().RecalculateMatrices();

	const glm::vec3 expectedDirection = glm::normalize(targetWorld - m_node->GetTransform().GetPosition(Transform::Space::World));
	ExpectVec3Near(m_node->GetTransform().GetForward(Transform::Space::World), expectedDirection);
}

TEST_F(TransformTest, LookAtWorldIgnoresParentScaleForOrientation)
{
	auto parentNode = m_scene->AddNode("Parent");
	parentNode->AddChild(m_node);
	const glm::quat parentRotation = glm::angleAxis(0.6f, glm::normalize(glm::vec3(1.f, 2.f, 0.5f)));
	parentNode->GetTransform().SetPosition(glm::vec3(2.f, -1.f, 3.f));
	parentNode->GetTransform().SetRotation(parentRotation);
	parentNode->GetTransform().SetScale(glm::vec3(2.f, 0.5f, 3.f));
	parentNode->GetTransform().RecalculateMatrices();

	const glm::vec3 target = m_node->GetTransform().GetPosition(Transform::Space::World) +
		glm::normalize(glm::vec3(-1.f, 2.f, 3.f)) * 5.f;
	m_node->GetTransform().LookAt(target, glm::vec3(0.f, 1.f, 0.f), Transform::Space::World);

	ExpectVec3Near(
		m_node->GetTransform().GetForward(Transform::Space::World),
		glm::normalize(target - m_node->GetTransform().GetPosition(Transform::Space::World)));
}

// Dirty Tracking

TEST_F(TransformTest, MarkDirtyAfterChange)
{
	m_node->GetTransform().RecalculateMatrices();
	EXPECT_FALSE(m_node->GetTransform().IsDirty());

	m_node->GetTransform().SetPosition(glm::vec3(1.f, 0.f, 0.f));
	EXPECT_TRUE(m_node->GetTransform().IsDirty());
}

TEST_F(TransformTest, CleanAfterRecalcMatrices)
{
	m_node->GetTransform().SetPosition(glm::vec3(1.f, 2.f, 3.f));
	EXPECT_TRUE(m_node->GetTransform().IsDirty());

	m_node->GetTransform().RecalculateMatrices();
	EXPECT_FALSE(m_node->GetTransform().IsDirty());
}

TEST_F(TransformTest, MarkDirtyRecursively)
{
	std::shared_ptr<Node> childNode = m_scene->AddNode("Node");
	m_node.get()->AddChild(childNode);

	m_node->GetTransform().RecalculateMatrices();
	childNode->GetTransform().RecalculateMatrices();

	EXPECT_FALSE(m_node->GetTransform().IsDirty());
	EXPECT_FALSE(childNode->GetTransform().IsDirty());

	m_node->GetTransform().SetPosition(glm::vec3(1.f, 0.f, 0.f));

	EXPECT_TRUE(m_node->GetTransform().IsDirty());
	EXPECT_TRUE(childNode->GetTransform().IsDirty());
}
