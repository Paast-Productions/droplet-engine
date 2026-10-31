#include "Camera.hpp"

#include <algorithm>
#include <glm/gtx/quaternion.hpp>
#include <Editor/ViewOverlayUI.hpp>

using namespace Droplet::Graphics;
using namespace Droplet::Math;

Camera::Camera(
	glm::vec3 p_pos,
	glm::vec3 p_fwd,
	glm::vec3 p_up,
	float p_fov, 
	float p_aspect) : 
	m_position(p_pos),
	m_forward(p_fwd),
	m_up(p_up),
	m_fov(p_fov),
	m_aspect(p_aspect)
{
	UpdateCameraVectors();
}


void Camera::Move(glm::vec3 p_direction, float p_velocity)
{
	m_position += p_direction * p_velocity;
}

void Camera::Rotate(float p_xOffset, float p_yOffset, glm::vec3 p_up, bool p_constrainPitch)
{
	const glm::vec3 worldUp = glm::normalize(p_up);

	// Apply yaw around the world-up axis.
	glm::quat yawQuat = glm::angleAxis(glm::radians(p_xOffset), worldUp);

	glm::vec3 newFwd = glm::normalize(yawQuat * m_forward);

	if (p_constrainPitch)
	{
		// Calculate pitch from the yaw-adjusted forward direction.
		float currentPitch = glm::degrees(glm::asin(glm::clamp(glm::dot(newFwd, worldUp), -1.0f, 1.0f)));

		float newPitch = glm::clamp(currentPitch + p_yOffset, -89.0f, 89.0f);

		p_yOffset = newPitch - currentPitch;
	}

	// Recalculate right after yaw, before applying pitch.
	glm::vec3 right = glm::normalize(glm::cross(newFwd, worldUp));

	// Apply pitch around the updated right axis.
	glm::quat pitchQuat = glm::angleAxis(glm::radians(p_yOffset), right);

	newFwd = glm::normalize(pitchQuat * newFwd);

	m_forward = newFwd;
	m_right = glm::normalize(glm::cross(m_forward, worldUp));
	m_up = glm::normalize(glm::cross(m_right, m_forward));
}

void Camera::UpdateCameraVectors()
{
	m_forward = glm::normalize(m_forward);
	m_up = glm::normalize(m_up);
	m_right = glm::normalize(glm::cross(m_forward, m_up));
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
	return glm::lookAt(
		m_position, 
		m_position + m_forward, 
		m_up
	);
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

void Camera::SetOrientation(float p_yaw, float p_pitch, glm::vec3 p_up)
{
	// Apply yaw
	glm::quat yawQuat = glm::angleAxis(glm::radians(p_yaw), glm::normalize(p_up));
	m_forward = glm::normalize(yawQuat * m_forward);
	m_up = glm::normalize(yawQuat * m_up);

	// Apply pitch
	glm::quat pitchQuat = glm::angleAxis(glm::radians(p_pitch), glm::normalize(m_right));
	m_forward = glm::normalize(pitchQuat * m_forward);
	m_up = glm::normalize(pitchQuat * m_up);

	UpdateCameraVectors();
}

void Camera::SetOrientation(glm::quat p_orientation)
{
	m_forward = glm::normalize(p_orientation * glm::vec3(0, 0, -1));
	m_up = glm::normalize(p_orientation * glm::vec3(0, 1, 0));

	UpdateCameraVectors();
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
