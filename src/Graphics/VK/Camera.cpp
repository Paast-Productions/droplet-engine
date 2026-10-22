#include "Camera.hpp"

#include <algorithm>

using namespace Droplet::Graphics;
using namespace Droplet::Math;

Camera::Camera(
	glm::vec3 p_position,
	glm::vec3 p_up,
	float p_yaw,
	float p_pitch,
	float p_fov, 
	float p_aspect) : 
	m_position(p_position),
	m_up(p_up),
	m_worldUp(p_up),
	m_yaw(p_yaw),
	m_pitch(p_pitch),
	m_fov(p_fov),
	m_aspect(p_aspect)
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

Frustum Camera::GetFrustum() const
{
	return Frustum(
		m_position, 
		m_forward, 
		m_up, 
		glm::radians(m_fov), 
		m_aspect, 
		m_planes.x, 
		m_planes.y
	);
}

glm::mat4 Camera::GetViewMatrix() const
{
	return glm::lookAt(m_position, m_position + m_forward, m_up);
}

glm::mat4 Camera::GetProjectionMatrix() const
{
	return glm::perspective(
		glm::radians(m_fov),
		m_aspect,
		m_planes.x,
		m_planes.y
	);
}

Ray Camera::GetViewRay(const glm::vec2 &p_viewPos) const
{
	using namespace glm;

	// Convert viewport-space [0..1] to NDC [-1..1] (flip Y for screen-space origin at top-left)
	const vec2 ndc = vec2(
		(p_viewPos.x * 2.0f) - 1.0f,
		1.0f - (p_viewPos.y * 2.0f)
	);

	const mat4 invViewProj = inverse(GetProjectionMatrix() * GetViewMatrix());

	vec4 nearPoint = invViewProj * vec4(ndc, -1.0f, 1.0f);
	vec4 farPoint = invViewProj * vec4(ndc, 1.0f, 1.0f);

	nearPoint /= nearPoint.w;
	farPoint /= farPoint.w;

	const vec3 origin = vec3(nearPoint);
	const vec3 direction = normalize(vec3(farPoint - nearPoint));

	return Math::Ray(origin, direction);
}

Frustum Camera::ViewRectToWorldTile(const glm::vec4 &minMax) const
{
	using namespace glm;

	glm::vec4 ndcMinMax{
		(minMax.x - 0.5f) * 2.0f,
		(minMax.y - 0.5f) * -2.0f,
		(minMax.z - 0.5f) * 2.0f,
		(minMax.w - 0.5f) * -2.0f
	};

	const mat4 invViewProj = inverse(GetProjectionMatrix() * GetViewMatrix());
	constexpr float nearNdcZ = (GLM_CONFIG_CLIP_CONTROL & GLM_CLIP_CONTROL_ZO_BIT) ? 0.0f : -1.0f;
	constexpr float farNdcZ = 1.0f;

	const auto unproject = [&invViewProj](float x, float y, float z) {
		vec4 point = invViewProj * vec4(x, y, z, 1.0f);
		return vec3(point) / point.w;
	};

	const vec3 nearTopLeft =		unproject(ndcMinMax.x, ndcMinMax.w, nearNdcZ);
	const vec3 nearTopRight =		unproject(ndcMinMax.z, ndcMinMax.w, nearNdcZ);
	const vec3 nearBottomLeft = 	unproject(ndcMinMax.x, ndcMinMax.y, nearNdcZ);
	const vec3 nearBottomRight =	unproject(ndcMinMax.z, ndcMinMax.y, nearNdcZ);
	const vec3 farTopLeft =			unproject(ndcMinMax.x, ndcMinMax.w, farNdcZ);
	const vec3 farTopRight =		unproject(ndcMinMax.z, ndcMinMax.w, farNdcZ);
	const vec3 farBottomLeft =		unproject(ndcMinMax.x, ndcMinMax.y, farNdcZ);
	const vec3 farBottomRight =		unproject(ndcMinMax.z, ndcMinMax.y, farNdcZ);
	const vec3 center = (
		nearTopLeft + nearTopRight + nearBottomLeft + nearBottomRight +
		farTopLeft + farTopRight + farBottomLeft + farBottomRight
		) / 8.0f;

	const auto makeInwardPlane = [&center](const vec3 &a, const vec3 &b, const vec3 &c) {
		Plane plane(a, b, c);
		if (dot(plane.normal, center) - plane.distance < 0.0f)
		{
			plane.normal = -plane.normal;
			plane.distance = -plane.distance;
		}
		return plane;
	};

	return Frustum(
		makeInwardPlane(nearTopLeft, farTopLeft, farBottomLeft),
		makeInwardPlane(farTopRight, nearTopRight, nearBottomRight),
		makeInwardPlane(farTopRight, farTopLeft, nearTopLeft),
		makeInwardPlane(farBottomLeft, farBottomRight, nearBottomRight),
		makeInwardPlane(nearTopRight, nearTopLeft, nearBottomLeft),
		makeInwardPlane(farTopLeft, farTopRight, farBottomRight)
	);
}
