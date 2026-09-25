#include "Transform.hpp"
#include <Node.hpp>

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <stdexcept>

using namespace Droplet::Scene;

Transform::Transform(Droplet::Scene::Node *p_owner) : m_owner(p_owner)
{
	// Ensure that the Transform is being constructed by its owning Node.
	if (&(m_owner->GetTransform()) != this)
	{
		throw std::runtime_error("Transforms may only be constructed by their owning Node.");
	}
}

glm::vec3 Transform::GetPosition(Space p_space) const
{
	// If the requested space is world space, but the transform has no parent, treat it as local space.
	if (p_space == Space::World)
	{
		if (!HasParent())
		{
			p_space = Space::Local;
		}
	}

	switch (p_space)
	{
	default:
	case Space::Local:
		return m_position;

	case Space::World:
		// Convert local position to world position using the parent's world matrix.
		glm::mat4 parentWorldMatrix = GetParentTransform()->GetMatrix(Space::World);

		glm::vec3 worldPos = parentWorldMatrix * glm::vec4(m_position, 1.0f);
		return worldPos;
	}
}

glm::quat Transform::GetRotation(Space p_space) const
{
	// If the requested space is world space, but the transform has no parent, treat it as local space.
	if (p_space == Space::World)
	{
		if (!HasParent())
		{
			p_space = Space::Local;
		}
	}
	
	switch (p_space)
	{
	default:
	case Space::Local:
		return m_rotation;

	case Space::World:
		glm::mat4 parentWorldMatrix = GetParentTransform()->GetMatrix(Space::World);

		glm::quat parentWorldRotation = glm::quat_cast(parentWorldMatrix);
		return parentWorldRotation * m_rotation;
	}
}

glm::vec3 Transform::GetEuler(Space p_space) const
{
	// If the requested space is world space, but the transform has no parent, treat it as local space.
	if (p_space == Space::World)
	{
		if (!HasParent())
		{
			p_space = Space::Local;
		}
	}

	switch (p_space)
	{
	default:
	case Space::Local:
		return glm::eulerAngles(m_rotation);

	case Space::World:
		return glm::eulerAngles(GetRotation(Space::World));
	}
}

glm::vec3 Transform::GetScale() const
{
	return m_scale;
}

glm::mat4 Transform::GetMatrix(Space p_space)
{
	// If the requested space is world space, but the transform has no parent, treat it as local space.
	if (p_space == Space::World)
	{
		if (!HasParent())
		{
			p_space = Space::Local;
		}
	}

	switch (p_space)
	{
	default:
	case Space::Local:
		if (m_isDirty)
		{
			UpdateLocalMatrix();
		}

		return m_localMatrix;

	case Space::World:
		if (m_isDirty)
		{
			RecalculateMatrices();
		}

		return m_worldMatrix;
	}
}

bool Transform::IsDirty() const
{
	return m_isDirty;
}

glm::vec3 Transform::GetUp(Space p_space) const
{
	// If the requested space is world space, but the transform has no parent, treat it as local space.
	if (p_space == Space::World)
	{
		if (!HasParent())
		{
			p_space = Space::Local;
		}
	}

	switch (p_space)
	{
	default:
	case Space::Local:
		return glm::normalize(m_rotation * glm::vec3(0.f, 1.f, 0.f));

	case Space::World:
		return glm::normalize(GetRotation(Space::World) * glm::vec3(0.f, 1.f, 0.f));
	}
}

glm::vec3 Transform::GetRight(Space p_space) const
{
	// If the requested space is world space, but the transform has no parent, treat it as local space.
	if (p_space == Space::World)
	{
		if (!HasParent())
		{
			p_space = Space::Local;
		}
	}

	switch (p_space)
	{
	default:
	case Space::Local:
		return glm::normalize(m_rotation * glm::vec3(1.f, 0.f, 0.f));

	case Space::World:
		return glm::normalize(GetRotation(Space::World) * glm::vec3(1.f, 0.f, 0.f));
	}
}

glm::vec3 Transform::GetForward(Space p_space) const
{
	// If the requested space is world space, but the transform has no parent, treat it as local space.
	if (p_space == Space::World)
	{
		if (!HasParent())
		{
			p_space = Space::Local;
		}
	}

	switch (p_space)
	{
	default:
	case Space::Local:
		return glm::normalize(m_rotation * glm::vec3(0.f, 0.f, 1.f));

	case Space::World:
		return glm::normalize(GetRotation(Space::World) * glm::vec3(0.f, 0.f, 1.f));
	}
}

void Transform::SetPosition(const glm::vec3 &p_position, Space p_space)
{
	// If the requested space is world space, but the transform has no parent, treat it as local space.
	if (p_space == Space::World)
	{
		if (!HasParent())
		{
			p_space = Space::Local;
		}
	}

	switch (p_space)
	{
	default:
	case Space::Local:
		m_position = p_position;
		break;

	case Space::World:
		// Convert world position to local position using the parent's world matrix.
		glm::mat4 parentWorldMatrix = GetParentTransform()->GetMatrix(Space::World);
		glm::vec3 localPos = glm::inverse(parentWorldMatrix) * glm::vec4(p_position, 1.0f);
		m_position = localPos;
		break;
	}

	MakeDirty();
}

void Transform::SetRotation(const glm::quat &p_rotation, Space p_space)
{
	// If the requested space is world space, but the transform has no parent, treat it as local space.
	if (p_space == Space::World)
	{
		if (!HasParent())
		{
			p_space = Space::Local;
		}
	}

	switch (p_space)
	{
	default:
	case Space::Local:
		m_rotation = p_rotation;
		break;

	case Space::World:
		// Convert world rotation to local rotation using the parent's world matrix.
		glm::mat4 parentWorldMatrix = GetParentTransform()->GetMatrix(Space::World);
		glm::quat localRot = glm::inverse(glm::quat_cast(parentWorldMatrix)) * p_rotation;
		m_rotation = localRot;
		break;
	}

	ValidateRotation();

	MakeDirty();
}

void Transform::SetEuler(const glm::vec3 &p_eulerAngles, Space p_space)
{
	// If the requested space is world space, but the transform has no parent, treat it as local space.
	if (p_space == Space::World)
	{
		if (!HasParent())
		{
			p_space = Space::Local;
		}
	}

	switch (p_space)
	{
	default:
	case Transform::Space::Local:
		m_rotation = glm::quat(p_eulerAngles);
		break;

	case Transform::Space::World:
		// Convert world Euler angles to local rotation using the parent's world matrix.
		glm::mat4 parentWorldMatrix = GetParentTransform()->GetMatrix(Space::World);
		glm::quat worldRot = glm::quat(p_eulerAngles);
		glm::quat localRot = glm::inverse(glm::quat_cast(parentWorldMatrix)) * worldRot;
		m_rotation = localRot;
		break;
	}

	ValidateRotation();

	MakeDirty();
}

void Transform::SetScale(const glm::vec3 &p_scale)
{
	m_scale = p_scale;
	MakeDirty();
}

void Transform::SetMatrix(const glm::mat4 &p_matrix, Space p_space)
{
	// If the requested space is world space, but the transform has no parent, treat it as local space.
	if (p_space == Space::World)
	{
		if (!HasParent())
		{
			p_space = Space::Local;
		}
	}

	switch (p_space)
	{
	default:
	case Transform::Space::Local:
	{
		// Decompose the matrix into position, rotation, and scale.
		glm::vec3 position{};
		glm::quat rotation{};
		glm::vec3 scale{};
		glm::vec3 skew{};
		glm::vec4 perspective{};

		glm::decompose(p_matrix, scale, rotation, position, skew, perspective);

		m_position = position;
		m_rotation = rotation;
		m_scale = scale;
		break;
	}

	case Transform::Space::World:
	{
		// Transform the world matrix into local space using the parent's world matrix.
		glm::mat4 parentWorldMatrix = GetParentTransform()->GetMatrix(Space::World);
		glm::mat4 localMatrix = glm::inverse(parentWorldMatrix) * p_matrix;

		// Decompose the local matrix into position, rotation, and scale.
		glm::vec3 position{};
		glm::quat rotation{};
		glm::vec3 scale{};
		glm::vec3 skew{};
		glm::vec4 perspective{};

		glm::decompose(localMatrix, scale, rotation, position, skew, perspective);

		m_position = position;
		m_rotation = rotation;
		m_scale = scale;
		break;
	}
	}

	ValidateRotation();

	MakeDirty();
}

void Transform::MakeDirty()
{
	m_isDirty = true;

	// Recursively mark children as dirty
	for (const auto &child : m_owner->GetChildren())
	{
		child->GetTransform().MakeDirty();
	}
}

void Transform::RecalculateMatrices()
{
	UpdateLocalMatrix();

	// If the transform has a parent, multiply the local matrix by the parent's world matrix to get the world matrix.
	if (HasParent())
	{
		m_worldMatrix = GetParentTransform()->GetMatrix(Space::World) * m_localMatrix;
	}
	else
	{
		m_worldMatrix = m_localMatrix;
	}

	m_isDirty = false;
}

void Transform::Move(const glm::vec3 &p_delta, Space p_space)
{
	// If the requested space is world space, but the transform has no parent, treat it as local space.
	if (p_space == Space::World)
	{
		if (!HasParent())
		{
			p_space = Space::Local;
		}
	}

	switch (p_space)
	{
	default:
	case Space::Local:
		m_position += p_delta;
		break;

	case Space::World:
		// Convert the world delta to local space using the parent's world matrix.
		glm::mat4 parentWorldMatrix = GetParentTransform()->GetMatrix(Space::World);
		glm::vec3 localDelta = glm::inverse(parentWorldMatrix) * glm::vec4(p_delta, 0.0f);
		m_position += localDelta;
		break;
	}

	MakeDirty();
}

void Transform::Rotate(const glm::quat &p_delta, Space p_space)
{
	// If the requested space is world space, but the transform has no parent, treat it as local space.
	if (p_space == Space::World)
	{
		if (!HasParent())
		{
			p_space = Space::Local;
		}
	}

	switch (p_space)
	{
	default:
	case Space::Local:
		m_rotation = p_delta * m_rotation;
		break;

	case Space::World:
		// Convert the world rotation delta to local space using the parent's world matrix.
		glm::mat4 parentWorldMatrix = GetParentTransform()->GetMatrix(Space::World);
		glm::quat localDelta = glm::inverse(glm::quat_cast(parentWorldMatrix)) * p_delta * glm::quat_cast(parentWorldMatrix);
		m_rotation = localDelta * m_rotation;
		break;
	}

	ValidateRotation();

	MakeDirty();
}

void Transform::RotateEuler(const glm::vec3 &p_delta, Space p_space)
{
	// If the requested space is world space, but the transform has no parent, treat it as local space.
	if (p_space == Space::World)
	{
		if (!HasParent())
		{
			p_space = Space::Local;
		}
	}

	switch (p_space)
	{
	default:
	case Space::Local:
		m_rotation = glm::quat(p_delta) * m_rotation;
		break;

	case Space::World:
		// Convert the world Euler angles to a quaternion and then to local space using the parent's world matrix.
		glm::mat4 parentWorldMatrix = GetParentTransform()->GetMatrix(Space::World);
		glm::quat worldDelta = glm::quat(p_delta);
		glm::quat localDelta = glm::inverse(glm::quat_cast(parentWorldMatrix)) * worldDelta * glm::quat_cast(parentWorldMatrix);
		m_rotation = localDelta * m_rotation;
		break;
	}

	ValidateRotation();

	MakeDirty();
}

void Transform::AddScale(const glm::vec3 &p_delta)
{
	m_scale += p_delta;
	MakeDirty();
}

void Transform::RotateAxis(float p_angle, const glm::vec3 &p_axis, Space p_space)
{
	// If the requested space is world space, but the transform has no parent, treat it as local space.
	if (p_space == Space::World)
	{
		if (!HasParent())
		{
			p_space = Space::Local;
		}
	}

	switch (p_space)
	{
	default:
	case Space::Local:
		m_rotation = glm::angleAxis(p_angle, p_axis) * m_rotation;
		break;

	case Space::World:
		// Convert the world rotation to local space using the parent's world matrix.
		glm::mat4 parentWorldMatrix = GetParentTransform()->GetMatrix(Space::World);
		glm::quat localDelta = glm::inverse(glm::quat_cast(parentWorldMatrix)) * glm::angleAxis(p_angle, p_axis) * glm::quat_cast(parentWorldMatrix);
		m_rotation = localDelta * m_rotation;
		break;
	}

	ValidateRotation();

	MakeDirty();
}

void Transform::LookAt(const glm::vec3 &p_target, const glm::vec3 &p_up, Space p_space)
{
	// If the requested space is world space, but the transform has no parent, treat it as local space.
	if (p_space == Space::World)
	{
		if (!HasParent())
		{
			p_space = Space::Local;
		}
	}

	switch (p_space)
	{
	default:
	case Space::Local:
		m_rotation = glm::quat_cast(glm::lookAt(m_position, p_target, p_up));
		break;

	case Space::World:
		// Convert the world target position & up vector to local space using the parent's world matrix.
		glm::mat4 parentWorldMatrix = GetParentTransform()->GetMatrix(Space::World);
		glm::vec3 localTarget = glm::inverse(parentWorldMatrix) * glm::vec4(p_target, 1.0f);
		glm::vec3 localUp = glm::inverse(parentWorldMatrix) * glm::vec4(p_up, 0.0f);
		m_rotation = glm::quat_cast(glm::lookAt(m_position, localTarget, localUp));
		break;
	}

	ValidateRotation();

	MakeDirty();
}

void Transform::ValidateRotation()
{
	m_rotation = glm::normalize(m_rotation);
	MakeDirty();
}

void Transform::UpdateLocalMatrix()
{
	m_localMatrix = 
		glm::translate(glm::mat4(1.0f), m_position) *
		glm::toMat4(m_rotation) *
		glm::scale(glm::mat4(1.0f), m_scale);
}

bool Transform::HasParent() const
{
	assert(m_owner != nullptr && "Transform must have an owner Node.");

	return m_owner->GetParent() != nullptr;
}

Transform *Transform::GetParentTransform() const
{
	if (!HasParent())
		return nullptr;

	return &(m_owner->GetParent()->GetTransform());
}
