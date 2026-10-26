#include "math/bounds/Frustum.hpp"
#include <glm/glm.hpp>
#include <tracy/public/tracy/Tracy.hpp>

using namespace Droplet::Math;

namespace
{
	constexpr float EPSILON = 1e-5f;

	[[nodiscard]] float SignedDistanceToPlane(const Plane &p_plane, const glm::vec3 &p_point)
	{
		return glm::dot(p_plane.normal, p_point) - p_plane.distance;
	}

	[[nodiscard]] bool IntersectThreePlanes(const Plane &p_p0, const Plane &p_p1, const Plane &p_p2, glm::vec3 &p_outPoint)
	{
		glm::vec3 n0 = p_p0.normal;
		glm::vec3 n1 = p_p1.normal;
		glm::vec3 n2 = p_p2.normal;

		glm::vec3 n1xn2 = glm::cross(n1, n2);
		float denominator = glm::dot(n0, n1xn2);

		if (glm::abs(denominator) < EPSILON)
		{
			return false;
		}

		p_outPoint = (p_p0.distance * n1xn2 +
			p_p1.distance * glm::cross(n2, n0) +
			p_p2.distance * glm::cross(n0, n1)) / denominator;

		return true;
	}

	[[nodiscard]] bool GetFrustumCorners(const Frustum &p_frustum, glm::vec3 *p_corners)
	{
		const Plane &left = p_frustum.GetPlane(0);
		const Plane &right = p_frustum.GetPlane(1);
		const Plane &top = p_frustum.GetPlane(2);
		const Plane &bottom = p_frustum.GetPlane(3);
		const Plane &nearPlane = p_frustum.GetPlane(4);
		const Plane &farPlane = p_frustum.GetPlane(5);

		return
			IntersectThreePlanes(left, top, nearPlane, p_corners[0]) &&
			IntersectThreePlanes(right, top, nearPlane, p_corners[1]) &&
			IntersectThreePlanes(left, bottom, nearPlane, p_corners[2]) &&
			IntersectThreePlanes(right, bottom, nearPlane, p_corners[3]) &&
			IntersectThreePlanes(left, top, farPlane, p_corners[4]) &&
			IntersectThreePlanes(right, top, farPlane, p_corners[5]) &&
			IntersectThreePlanes(left, bottom, farPlane, p_corners[6]) &&
			IntersectThreePlanes(right, bottom, farPlane, p_corners[7]);
	}

	[[nodiscard]] bool IsPointInsideFrustum(const Frustum &p_frustum, const glm::vec3 &p_point)
	{
		for (int i = 0; i < 6; ++i)
		{
			if (SignedDistanceToPlane(p_frustum.GetPlane(i), p_point) < -EPSILON)
			{
				return false;
			}
		}

		return true;
	}
}

Frustum::Frustum(const glm::mat4x4 &p_matrix)
{
	// Left plane
	planes[0].normal.x = p_matrix[0][3] + p_matrix[0][0];
	planes[0].normal.y = p_matrix[1][3] + p_matrix[1][0];
	planes[0].normal.z = p_matrix[2][3] + p_matrix[2][0];
	planes[0].distance = p_matrix[3][3] + p_matrix[3][0];

	// Right plane
	planes[1].normal.x = p_matrix[0][3] - p_matrix[0][0];
	planes[1].normal.y = p_matrix[1][3] - p_matrix[1][0];
	planes[1].normal.z = p_matrix[2][3] - p_matrix[2][0];
	planes[1].distance = p_matrix[3][3] - p_matrix[3][0];

	// Bottom plane
	planes[2].normal.x = p_matrix[0][3] + p_matrix[0][1];
	planes[2].normal.y = p_matrix[1][3] + p_matrix[1][1];
	planes[2].normal.z = p_matrix[2][3] + p_matrix[2][1];
	planes[2].distance = p_matrix[3][3] + p_matrix[3][1];

	// Top plane
	planes[3].normal.x = p_matrix[0][3] - p_matrix[0][1];
	planes[3].normal.y = p_matrix[1][3] - p_matrix[1][1];
	planes[3].normal.z = p_matrix[2][3] - p_matrix[2][1];
	planes[3].distance = p_matrix[3][3] - p_matrix[3][1];

	// Near plane
	planes[4].normal.x = p_matrix[0][2];
	planes[4].normal.y = p_matrix[1][2];
	planes[4].normal.z = p_matrix[2][2];
	planes[4].distance = p_matrix[3][2];

	// Far plane
	planes[5].normal.x = p_matrix[0][3] - p_matrix[0][2];
	planes[5].normal.y = p_matrix[1][3] - p_matrix[1][2];
	planes[5].normal.z = p_matrix[2][3] - p_matrix[2][2];
	planes[5].distance = p_matrix[3][3] - p_matrix[3][2];
}

Frustum::Frustum(const glm::vec3 &p_origin, const glm::vec3 &p_fwd, const glm::vec3 &p_up, float p_fov, float p_aspect, float p_near, float p_far)
{
	ZoneScoped;

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
	planes[0] = Plane(nearTopLeft, farTopLeft, farBottomLeft);			// Left
	planes[1] = Plane(farTopRight, nearTopRight, nearBottomRight);		// Right
	planes[2] = Plane(farTopRight, farTopLeft, nearTopLeft);			// Top
	planes[3] = Plane(farBottomLeft, farBottomRight, nearBottomRight);	// Bottom
	planes[4] = Plane(nearTopRight, nearTopLeft, nearBottomLeft);		// Near
	planes[5] = Plane(farTopLeft, farTopRight, farBottomRight);			// Far
}

void Frustum::GetCorners(glm::vec3 *p_corners) const
{
	if (!GetFrustumCorners(*this, p_corners))
	{
		for (int i = 0; i < 8; ++i)
		{
			p_corners[i] = glm::vec3(0.0f);
		}
	}
}

Frustum Frustum::Transform(const Frustum &p_frustum, const glm::mat4x4 &p_matrix)
{
	Frustum transformedFrustum;

	glm::mat4x4 inverseTranspose = glm::transpose(glm::inverse(p_matrix));

	for (int i = 0; i < 6; ++i)
	{
		const Plane &plane = p_frustum.GetPlane(i);

		// Transform the plane's normal and distance
		glm::vec3 transformedNormal = inverseTranspose * glm::vec4(plane.normal, 0.0f);
		float transformedDistance = plane.distance - glm::dot(transformedNormal, glm::vec3(p_matrix[3]));
		transformedFrustum.planes[i] = Plane(glm::normalize(transformedNormal), transformedDistance);
	}

	return transformedFrustum;
}
