#include "Transform.hpp"
// #include "Node.hpp"

using namespace Droplet;

Transform::Transform(std::weak_ptr<Node> p_owner) : m_owner(std::move(p_owner))
{

}

const glm::vec3 &Transform::GetPosition(Space p_space) const
{
	if ()
}

const glm::quat &Transform::GetRotation(Space p_space) const
{
	// TODO: insert return statement here
}

glm::vec3 Transform::GetEuler(Space p_space) const
{
	return glm::vec3();
}

const glm::vec3 &Transform::GetScale() const
{
	// TODO: insert return statement here
}

const glm::mat4 &Transform::GetMatrix(Space p_space) const
{
	// TODO: insert return statement here
}

bool Transform::IsDirty() const
{
	return false;
}

glm::vec3 Transform::GetUp(Space p_space) const
{
	return glm::vec3();
}

glm::vec3 Transform::GetRight(Space p_space) const
{
	return glm::vec3();
}

glm::vec3 Transform::GetForward(Space p_space) const
{
	return glm::vec3();
}

void Transform::SetPosition(const glm::vec3 &p_position, Space p_space)
{

}

void Transform::SetRotation(const glm::quat &p_rotation, Space p_space)
{

}

void Transform::SetEuler(const glm::vec3 &p_eulerAngles, Space p_space)
{

}

void Transform::SetScale(const glm::vec3 &p_scale)
{

}

void Transform::SetMatrix(const glm::mat4 &p_matrix, Space p_space)
{

}

void Transform::MakeDirty()
{

}

void Transform::RecalculateWorldMatrix()
{

}

void Transform::Move(const glm::vec3 &p_delta, Space p_space)
{

}

void Transform::Rotate(const glm::quat &p_delta, Space p_space)
{

}

void Transform::RotateEuler(const glm::vec3 &p_delta, Space p_space)
{

}

void Transform::AddScale(const glm::vec3 &p_delta)
{

}

void Transform::RotateAxis(float p_angle, const glm::vec3 &p_axis, Space p_space)
{

}

void Transform::LookAt(const glm::vec3 &p_target, const glm::vec3 &p_up, Space p_space)
{

}
