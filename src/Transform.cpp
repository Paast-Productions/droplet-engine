#include "Transform.hpp"
#include <algorithm>
#include <stdexcept>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <cmath>
#include <SceneSystem/Node.hpp>
#include <tracy/public/tracy/Tracy.hpp>

using namespace Droplet::Scene;
using namespace glm;

static float Sign(float x)
{
	float sign = static_cast<float>((x > 0.0f) - (x < 0.0f));
	return (sign == 0.0f) ? 1.0f : sign;
}

namespace
{
	bool IsFinite(const vec3 &p_value)
	{
		return std::isfinite(p_value.x) && std::isfinite(p_value.y) && std::isfinite(p_value.z);
	}

	bool IsFinite(const vec4 &p_value)
	{
		return std::isfinite(p_value.x) && std::isfinite(p_value.y) &&
			std::isfinite(p_value.z) && std::isfinite(p_value.w);
	}

	bool IsFinite(const quat &p_value)
	{
		return std::isfinite(p_value.x) && std::isfinite(p_value.y) &&
			std::isfinite(p_value.z) && std::isfinite(p_value.w);
	}

	bool IsFinite(const mat4 &p_value)
	{
		for (int column = 0; column < 4; ++column)
		{
			for (int row = 0; row < 4; ++row)
			{
				if (!std::isfinite(p_value[column][row]))
				{
					return false;
				}
			}
		}

		return true;
	}

	bool DecomposeTransformMatrix(const mat4 &p_matrix, vec3 &p_position,
		quat &p_rotation, vec3 &p_scale)
	{
		if (!IsFinite(p_matrix))
		{
			return false;
		}

		vec3 skew{};
		vec4 perspective{};
		if (!decompose(p_matrix, p_scale, p_rotation, p_position, skew, perspective))
		{
			return false;
		}

		constexpr float tolerance = 1.0e-4f;
		if (!IsFinite(p_position) || !IsFinite(p_rotation) || !IsFinite(p_scale) ||
			!IsFinite(skew) || !IsFinite(perspective) ||
			length(skew) > tolerance ||
			length(vec3(perspective)) > tolerance ||
			std::abs(perspective.w - 1.0f) > tolerance)
		{
			return false;
		}

		const float rotationLength = length(p_rotation);
		if (!std::isfinite(rotationLength) || rotationLength <= tolerance)
		{
			return false;
		}

		p_rotation = normalize(p_rotation);
		return true;
	}
}

Transform::Transform(Droplet::Scene::Node *p_owner) : m_owner(p_owner)
{
	if (m_owner == nullptr)
	{
		throw std::invalid_argument("Cannot create Transform: owner Node is null.");
	}

	// Ensure that the Transform is being constructed by its owning Node.
	if (&(m_owner->GetTransform()) != this)
	{
		throw std::runtime_error("Transforms may only be constructed by their owning Node.");
	}
}

vec3 Transform::GetPosition(Space p_space) const
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
		mat4 parentWorldMatrix = GetParentTransform()->GetMatrix(Space::World);

		vec3 worldPos = parentWorldMatrix * vec4(m_position, 1.0f);
		return worldPos;
	}
}

quat Transform::GetRotation(Space p_space) const
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
		return GetParentTransform()->GetRotation(Space::World) * m_rotation;
	}
}

vec3 Transform::GetEuler(Space p_space) const
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
		return eulerAngles(m_rotation);

	case Space::World:
		return eulerAngles(GetRotation(Space::World));
	}
}

vec3 Transform::GetScale() const
{
	return m_scale;
}

mat4 Transform::GetMatrix(Space p_space)
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

bool Droplet::Scene::Transform::HasBeenChanged() const
{
	return m_wasChanged;
}

vec3 Transform::GetUp(Space p_space) const
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
		return normalize(m_rotation * vec3(0.f, 1.f, 0.f));

	case Space::World:
		return normalize(GetRotation(Space::World) * vec3(0.f, 1.f, 0.f));
	}
}

vec3 Transform::GetRight(Space p_space) const
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
		return normalize(m_rotation * vec3(1.f, 0.f, 0.f));

	case Space::World:
		return normalize(GetRotation(Space::World) * vec3(1.f, 0.f, 0.f));
	}
}

vec3 Transform::GetForward(Space p_space) const
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
		return normalize(m_rotation * vec3(0.f, 0.f, 1.f));

	case Space::World:
		return normalize(GetRotation(Space::World) * vec3(0.f, 0.f, 1.f));
	}
}

void Transform::SetPosition(const vec3 &p_position, Space p_space)
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
		mat4 parentWorldMatrix = GetParentTransform()->GetMatrix(Space::World);
		vec3 localPos = inverse(parentWorldMatrix) * vec4(p_position, 1.0f);
		m_position = localPos;
		break;
	}

	MakeDirty();
}

void Transform::SetRotation(const quat &p_rotation, Space p_space)
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
		// Convert world rotation to local rotation using the parent's world rotation.
		quat localRot = inverse(GetParentTransform()->GetRotation(Space::World)) * p_rotation;
		m_rotation = localRot;
		break;
	}

	ValidateRotation();

	MakeDirty();
}

void Transform::SetEuler(const vec3 &p_eulerAngles, Space p_space)
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
		m_rotation = quat(p_eulerAngles);
		break;

	case Transform::Space::World:
		// Convert world Euler angles to local rotation using the parent's world rotation.
		quat worldRot = quat(p_eulerAngles);
		quat localRot = inverse(GetParentTransform()->GetRotation(Space::World)) * worldRot;
		m_rotation = localRot;
		break;
	}

	ValidateRotation();

	MakeDirty();
}

void Transform::SetScale(const vec3 &p_scale)
{
	// Sanitize scale values to avoid zero
	constexpr float MIN_SCALE = 1.0e-6f;
	m_scale.x = (std::abs(p_scale.x) < MIN_SCALE) ? (Sign(p_scale.x) * MIN_SCALE) : p_scale.x;
	m_scale.y = (std::abs(p_scale.y) < MIN_SCALE) ? (Sign(p_scale.y) * MIN_SCALE) : p_scale.y;
	m_scale.z = (std::abs(p_scale.z) < MIN_SCALE) ? (Sign(p_scale.z) * MIN_SCALE) : p_scale.z;

	MakeDirty();
}

void Transform::SetMatrix(const mat4 &p_matrix, Space p_space)
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
		vec3 position{};
		quat rotation{};
		vec3 scale{};
		if (!DecomposeTransformMatrix(p_matrix, position, rotation, scale))
		{
			return;
		}

		m_position = position;
		m_rotation = rotation;
		m_scale = scale;
		break;
	}

	case Transform::Space::World:
	{
		// Transform the world matrix into local space using the parent's world matrix.
		mat4 parentWorldMatrix = GetParentTransform()->GetMatrix(Space::World);

		const float parentDeterminant = determinant(parentWorldMatrix);
		if (!std::isfinite(parentDeterminant) || parentDeterminant == 0.0f)
		{
			return;
		}
		mat4 localMatrix = inverse(parentWorldMatrix) * p_matrix;

		vec3 position{};
		quat rotation{};
		vec3 scale{};
		if (!DecomposeTransformMatrix(localMatrix, position, rotation, scale))
		{
			return;
		}

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
	m_wasChanged = true;

	// Recursively mark children as dirty
	for (const auto &child : m_owner->GetChildren())
	{
		child->GetTransform().MakeDirty();
	}
}

void Transform::ResetFrame()
{
	ZoneScoped;

	m_wasChanged = false;

	// Recursively reset children
	for (const auto &child : m_owner->GetChildren())
	{
		child->GetTransform().ResetFrame();
	}
}

void Transform::RecalculateMatrices()
{
	ZoneScoped;

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

void Transform::Move(const vec3 &p_delta, Space p_space)
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
		mat4 parentWorldMatrix = GetParentTransform()->GetMatrix(Space::World);
		vec3 localDelta = inverse(parentWorldMatrix) * vec4(p_delta, 0.0f);
		m_position += localDelta;
		break;
	}

	MakeDirty();
}

void Transform::Rotate(const quat &p_delta, Space p_space)
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
		// Convert the world rotation delta to local space using the parent's world rotation.
		quat parentWorldRotation = GetParentTransform()->GetRotation(Space::World);
		quat localDelta = inverse(parentWorldRotation) * p_delta * parentWorldRotation;
		m_rotation = localDelta * m_rotation;
		break;
	}

	ValidateRotation();

	MakeDirty();
}

void Transform::SlerpRotation(const quat &p_target, float p_t, Space p_space)
{
	const float targetLength = length(p_target);
	if (!std::isfinite(p_t) || !IsFinite(p_target) || !std::isfinite(targetLength) || targetLength <= 1.0e-6f)
	{
		return;
	}

	const quat currentRotation = GetRotation(p_space);
	const quat targetRotation = normalize(p_target);
	const quat interpolated = normalize(slerp(currentRotation, targetRotation, std::clamp(p_t, 0.0f, 1.0f)));
	if (!IsFinite(interpolated))
	{
		return;
	}

	SetRotation(interpolated, p_space);
}

void Transform::RotateEuler(const vec3 &p_delta, Space p_space)
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
		m_rotation = quat(p_delta) * m_rotation;
		break;

	case Space::World:
		// Convert the world Euler angles to a quaternion and then to local space using the parent's world rotation.
		quat parentWorldRotation = GetParentTransform()->GetRotation(Space::World);
		quat worldDelta = quat(p_delta);
		quat localDelta = inverse(parentWorldRotation) * worldDelta * parentWorldRotation;
		m_rotation = localDelta * m_rotation;
		break;
	}

	ValidateRotation();

	MakeDirty();
}

void Transform::MoveRelative(const vec3 &p_delta)
{
	vec3 right = GetRight(Space::Local);
	vec3 up = GetUp(Space::Local);
	vec3 forward = GetForward(Space::Local);

	vec3 relativeDelta = right * p_delta.x + up * p_delta.y + forward * p_delta.z;

	Move(relativeDelta, Space::Local);
}

void Transform::RotateEulerRelative(const vec3 &p_delta)
{
	vec3 right = GetRight(Space::Local);
	vec3 up = GetUp(Space::Local);
	vec3 forward = GetForward(Space::Local);

	quat relativeRotation = angleAxis(p_delta.x, right) *
		angleAxis(p_delta.y, up) *
		angleAxis(p_delta.z, forward);

	Rotate(relativeRotation, Space::Local);
}

void Transform::AddScale(const vec3 &p_delta)
{
	m_scale += p_delta;
	MakeDirty();
}

void Transform::RotateAxis(float p_angle, const vec3 &p_axis, Space p_space)
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
		m_rotation = angleAxis(p_angle, p_axis) * m_rotation;
		break;

	case Space::World:
		// Convert the world rotation to local space using the parent's world rotation.
		quat parentWorldRotation = GetParentTransform()->GetRotation(Space::World);
		quat localDelta = inverse(parentWorldRotation) * angleAxis(p_angle, p_axis) * parentWorldRotation;
		m_rotation = localDelta * m_rotation;
		break;
	}

	ValidateRotation();

	MakeDirty();
}

void Transform::LookAt(const vec3 &p_target, const vec3 &p_up, Space p_space)
{
	// If the requested space is world space, but the transform has no parent, treat it as local space.
	if (p_space == Space::World)
	{
		if (!HasParent())
		{
			p_space = Space::Local;
		}
	}

	vec3 direction = p_target - (p_space == Space::World ? GetPosition(Space::World) : m_position);
	vec3 up = p_up;
	if (p_space == Space::World)
	{
		const quat inverseParentRotation = inverse(GetParentTransform()->GetRotation(Space::World));
		direction = inverseParentRotation * direction;
		up = inverseParentRotation * up;
	}

	const float directionLength = length(direction);
	const float upLength = length(up);
	if (!IsFinite(direction) || !IsFinite(up) || !std::isfinite(directionLength) ||
		!std::isfinite(upLength) || directionLength <= 1.0e-6f || upLength <= 1.0e-6f ||
		length(cross(direction / directionLength, up / upLength)) <= 1.0e-6f)
	{
		return;
	}

	direction /= directionLength;
	up /= upLength;
	// GLM's view matrix points along local -Z; invert it and aim along -direction
	// so the engine's local +Z forward axis points at the target.
	m_rotation = quat_cast(inverse(lookAt(vec3(0.0f), -direction, up)));

	ValidateRotation();

	MakeDirty();
}

void Transform::TransformByMatrix(const mat4 &p_matrix, Space p_space)
{
	// If the requested space is world space, but the transform has no parent, treat it as local space.
	if (p_space == Space::World)
	{
		if (!HasParent())
		{
			p_space = Space::Local;
		}
	}

	SetMatrix(GetMatrix(p_space) * p_matrix, p_space);

	ValidateRotation();
	MakeDirty();
}

void Transform::ValidateRotation()
{
	m_rotation = normalize(m_rotation);
	MakeDirty();
}

void Transform::UpdateLocalMatrix()
{
	m_localMatrix = 
		translate(mat4(1.0f), m_position) *
		toMat4(m_rotation) *
		scale(mat4(1.0f), m_scale);
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
