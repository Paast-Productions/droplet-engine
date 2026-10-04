#include "math/bounds/Frustum.hpp"
#include <glm/glm.hpp>

Droplet::Math::Frustum::Frustum(const glm::vec3 &p_origin, const glm::vec3 &p_fwd, const glm::vec3 &p_up, float p_fov, float p_aspect, float p_near, float p_far)
{
	// Calculate the right vector from the forward and up vectors
	glm::vec3 right = glm::normalize(glm::cross(p_fwd, p_up));

	// Calculate the half dimensions of the near and far planes
	float nearHeight = 2.0f * tan(p_fov / 2.0f) * p_near;
	float nearWidth = nearHeight * p_aspect;

	float farHeight = 2.0f * tan(p_fov / 2.0f) * p_far;
	float farWidth = farHeight * p_aspect;

	// Calculate the centers of the near and far planes
	glm::vec3 nearCenter = p_origin + p_fwd * p_near;
	glm::vec3 farCenter = p_origin + p_fwd * p_far;

	// Calculate the corners of the near plane
	glm::vec3 nearTopLeft = nearCenter + (p_up * (nearHeight / 2.0f)) - (right * (nearWidth / 2.0f));
	glm::vec3 nearTopRight = nearCenter + (p_up * (nearHeight / 2.0f)) + (right * (nearWidth / 2.0f));
	glm::vec3 nearBottomLeft = nearCenter - (p_up * (nearHeight / 2.0f)) - (right * (nearWidth / 2.0f));
	glm::vec3 nearBottomRight = nearCenter - (p_up * (nearHeight / 2.0f)) + (right * (nearWidth / 2.0f));

	// Calculate the corners of the far plane
	glm::vec3 farTopLeft = farCenter + (p_up * (farHeight / 2.0f)) - (right * (farWidth / 2.0f));
	glm::vec3 farTopRight = farCenter + (p_up * (farHeight / 2.0f)) + (right * (farWidth / 2.0f));
	glm::vec3 farBottomLeft = farCenter - (p_up * (farHeight / 2.0f)) - (right * (farWidth / 2.0f));
	glm::vec3 farBottomRight = farCenter - (p_up * (farHeight / 2.0f)) + (right * (farWidth / 2.0f));

	// Create the planes using the corners
	planes[0] = Plane(nearTopRight, nearTopLeft, nearBottomLeft); // Left
	planes[1] = Plane(nearBottomRight, nearTopRight, farTopRight); // Right
	planes[2] = Plane(nearTopLeft, nearTopRight, farTopRight); // Top
	planes[3] = Plane(nearBottomLeft, nearBottomRight, farBottomRight); // Bottom
	planes[4] = Plane(nearTopLeft, nearBottomLeft, nearBottomRight); // Near
	planes[5] = Plane(farTopRight, farBottomRight, farBottomLeft); // Far
}
