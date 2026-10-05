#include "Camera.hpp"

#include <algorithm>

using namespace Droplet::Graphics;

Camera::Camera(
	glm::vec3 p_position,
	glm::vec3 p_up,
	float p_yaw,
	float p_pitch) : 
	m_position(p_position),
	m_up(p_up),
	m_worldUp(p_up),
	m_yaw(p_yaw),
	m_pitch(p_pitch)
{
	UpdateCameraVectors();
}


void Camera::Move(glm::vec3 p_direction, float p_velocity)
{
	m_position += p_direction * p_velocity;
}

void Camera::Rotate(float p_xOffset, float p_yOffset, bool p_constrainPitch)
{
	m_yaw += p_xOffset;
	m_pitch += p_yOffset;
	if (p_constrainPitch)
	{
		m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
	}

	UpdateCameraVectors();
}

void Camera::UpdateCameraVectors()
{
	glm::vec3 newForward;
	newForward.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	newForward.y = sin(glm::radians(m_pitch));
	newForward.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	m_forward = glm::normalize(newForward);

	m_right = glm::normalize(glm::cross(m_forward, m_worldUp));
	m_up = glm::normalize(glm::cross(m_right, m_forward));

}

glm::vec3 Camera::GetPosition() const
{
	return m_position;
}
glm::vec3 Camera::GetForward() const
{
	return m_forward;
}

glm::vec3 Camera::GetRight() const
{
	return m_right;
}
glm::vec3 Camera::GetUp() const
{
	return m_up;
}

glm::mat4 Camera::GetViewMatrix() const
{
	return glm::lookAt(m_position, m_position + m_forward, m_up);
}

glm::mat4 Camera::GetProjectionMatrix(
	float p_aspectRatio,
	float p_nearPlane,
	float p_farPlane) const
{
	glm::mat4 projection = glm::perspective(
		glm::radians(m_zoom),
		p_aspectRatio,
		p_nearPlane,
		p_farPlane
	);

	return projection;

}

