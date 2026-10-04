#include "math/bounds/Intersection.hpp"
#include <glm/gtx/closest_point.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/extended_min_max.hpp>
#include <algorithm>
#include <array>
#include <limits>

using namespace Droplet::Math;

namespace
{
	constexpr float EPSILON = 1e-5f;

	[[nodiscard]] float SignedDistanceToPlane(const Plane &p_plane, const glm::vec3 &p_point)
	{
		return glm::dot(p_plane.normal, p_point) - p_plane.distance;
	}

	[[nodiscard]] std::array<glm::vec3, 8> GetCorners(const AABB &p_aabb)
	{
		glm::vec3 min = p_aabb.GetMin();
		glm::vec3 max = p_aabb.GetMax();

		return {
			glm::vec3(min.x, min.y, min.z),
			glm::vec3(max.x, min.y, min.z),
			glm::vec3(min.x, max.y, min.z),
			glm::vec3(max.x, max.y, min.z),
			glm::vec3(min.x, min.y, max.z),
			glm::vec3(max.x, min.y, max.z),
			glm::vec3(min.x, max.y, max.z),
			glm::vec3(max.x, max.y, max.z),
		};
	}

	[[nodiscard]] std::array<glm::vec3, 8> GetCorners(const OBB &p_obb)
	{
		glm::vec3 axisX = p_obb.orientation[0] * p_obb.extents.x;
		glm::vec3 axisY = p_obb.orientation[1] * p_obb.extents.y;
		glm::vec3 axisZ = p_obb.orientation[2] * p_obb.extents.z;

		return {
			p_obb.center - axisX - axisY - axisZ,
			p_obb.center + axisX - axisY - axisZ,
			p_obb.center - axisX + axisY - axisZ,
			p_obb.center + axisX + axisY - axisZ,
			p_obb.center - axisX - axisY + axisZ,
			p_obb.center + axisX - axisY + axisZ,
			p_obb.center - axisX + axisY + axisZ,
			p_obb.center + axisX + axisY + axisZ,
		};
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

	[[nodiscard]] bool GetFrustumCorners(const Frustum &p_frustum, std::array<glm::vec3, 8> &p_corners)
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

	[[nodiscard]] bool IsPointInsideAABB(const AABB &p_aabb, const glm::vec3 &p_point)
	{
		glm::vec3 min = p_aabb.GetMin();
		glm::vec3 max = p_aabb.GetMax();

		return p_point.x >= min.x - EPSILON && p_point.x <= max.x + EPSILON &&
			p_point.y >= min.y - EPSILON && p_point.y <= max.y + EPSILON &&
			p_point.z >= min.z - EPSILON && p_point.z <= max.z + EPSILON;
	}

	[[nodiscard]] bool IsPointInsideOBB(const OBB &p_obb, const glm::vec3 &p_point)
	{
		glm::vec3 local = glm::transpose(p_obb.orientation) * (p_point - p_obb.center);

		return glm::abs(local.x) <= p_obb.extents.x + EPSILON &&
			glm::abs(local.y) <= p_obb.extents.y + EPSILON &&
			glm::abs(local.z) <= p_obb.extents.z + EPSILON;
	}

	[[nodiscard]] bool IsPointInsideSphere(const Sphere &p_sphere, const glm::vec3 &p_point)
	{
		glm::vec3 delta = p_point - p_sphere.center;
		return glm::dot(delta, delta) <= p_sphere.radius * p_sphere.radius + EPSILON;
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

	[[nodiscard]] OBB ToOBB(const AABB &p_aabb)
	{
		return OBB(p_aabb.center, p_aabb.extents, glm::mat3(1.0f));
	}

	[[nodiscard]] bool IntersectsOBBs(const OBB &p_a, const OBB &p_b)
	{
		constexpr float SAT_EPSILON = 1e-6f;

		glm::vec3 aAxis[3] = {
			glm::normalize(p_a.orientation[0]),
			glm::normalize(p_a.orientation[1]),
			glm::normalize(p_a.orientation[2]),
		};

		glm::vec3 bAxis[3] = {
			glm::normalize(p_b.orientation[0]),
			glm::normalize(p_b.orientation[1]),
			glm::normalize(p_b.orientation[2]),
		};

		float r[3][3]{};
		float absR[3][3]{};

		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				r[i][j] = glm::dot(aAxis[i], bAxis[j]);
				absR[i][j] = glm::abs(r[i][j]) + SAT_EPSILON;
			}
		}

		glm::vec3 delta = p_b.center - p_a.center;
		glm::vec3 t(
			glm::dot(delta, aAxis[0]),
			glm::dot(delta, aAxis[1]),
			glm::dot(delta, aAxis[2])
		);

		float ra, rb;

		for (int i = 0; i < 3; ++i)
		{
			ra = p_a.extents[i];
			rb = p_b.extents[0] * absR[i][0] + p_b.extents[1] * absR[i][1] + p_b.extents[2] * absR[i][2];

			if (glm::abs(t[i]) > ra + rb)
			{
				return false;
			}
		}

		for (int j = 0; j < 3; ++j)
		{
			ra = p_a.extents[0] * absR[0][j] + p_a.extents[1] * absR[1][j] + p_a.extents[2] * absR[2][j];
			rb = p_b.extents[j];

			if (glm::abs(t[0] * r[0][j] + t[1] * r[1][j] + t[2] * r[2][j]) > ra + rb)
			{
				return false;
			}
		}

		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				ra = p_a.extents[(i + 1) % 3] * absR[(i + 2) % 3][j] + p_a.extents[(i + 2) % 3] * absR[(i + 1) % 3][j];
				rb = p_b.extents[(j + 1) % 3] * absR[i][(j + 2) % 3] + p_b.extents[(j + 2) % 3] * absR[i][(j + 1) % 3];

				if (glm::abs(t[(i + 2) % 3] * r[(i + 1) % 3][j] - t[(i + 1) % 3] * r[(i + 2) % 3][j]) > ra + rb)
				{
					return false;
				}
			}
		}

		return true;
	}

	[[nodiscard]] bool RaycastAABBInternal(const glm::vec3 &p_rayOrigin, const glm::vec3 &p_rayDir, const AABB &p_aabb, float &p_hitT, glm::vec3 &p_hitNormal)
	{
		glm::vec3 min = p_aabb.GetMin();
		glm::vec3 max = p_aabb.GetMax();

		float tMin = -std::numeric_limits<float>::infinity();
		float tMax = std::numeric_limits<float>::infinity();

		glm::vec3 tMinNormal(0.0f);
		glm::vec3 tMaxNormal(0.0f);

		for (int axis = 0; axis < 3; ++axis)
		{
			if (glm::abs(p_rayDir[axis]) < EPSILON)
			{
				if (p_rayOrigin[axis] < min[axis] || p_rayOrigin[axis] > max[axis])
				{
					return false;
				}

				continue;
			}

			float t1 = (min[axis] - p_rayOrigin[axis]) / p_rayDir[axis];
			float t2 = (max[axis] - p_rayOrigin[axis]) / p_rayDir[axis];

			glm::vec3 n1(0.0f);
			glm::vec3 n2(0.0f);
			n1[axis] = -1.0f;
			n2[axis] = 1.0f;

			if (t1 > t2)
			{
				std::swap(t1, t2);
				std::swap(n1, n2);
			}

			if (t1 > tMin)
			{
				tMin = t1;
				tMinNormal = n1;
			}

			if (t2 < tMax)
			{
				tMax = t2;
				tMaxNormal = n2;
			}

			if (tMin > tMax)
			{
				return false;
			}
		}

		if (tMax < 0.0f)
		{
			return false;
		}

		if (tMin >= 0.0f)
		{
			p_hitT = tMin;
			p_hitNormal = tMinNormal;
		}
		else
		{
			p_hitT = tMax;
			p_hitNormal = tMaxNormal;
		}

		return true;
	}
}


RayHit Droplet::Math::Raycast(const Ray &p_ray, const Plane &p_plane)
{
	float intersectionDistance;
	bool result = glm::intersectRayPlane(p_ray.pos, p_ray.dir, p_plane.GetOrigin(), p_plane.normal, intersectionDistance);

	RayHit hit{};
	hit.didHit = result;
	hit.distance = intersectionDistance;
	hit.hitPoint = p_ray.pos + p_ray.dir * intersectionDistance;
	hit.hitNormal = p_plane.normal;

	// Flip the normal if the ray is coming from behind the plane
	if (glm::dot(p_ray.dir, p_plane.normal) > 0.0f)
	{
		hit.hitNormal = -p_plane.normal;
	}

	return hit;
}

RayHit Droplet::Math::Raycast(const Ray &p_ray, const AABB &p_aabb)
{
	RayHit hit{};

	float hitT = 0.0f;
	glm::vec3 hitNormal(0.0f);
	if (!RaycastAABBInternal(p_ray.pos, p_ray.dir, p_aabb, hitT, hitNormal))
	{
		return hit;
	}

	hit.didHit = true;
	hit.hitPoint = p_ray.pos + p_ray.dir * hitT;
	hit.hitNormal = hitNormal;
	hit.distance = glm::length(hit.hitPoint - p_ray.pos);
	return hit;
}

RayHit Droplet::Math::Raycast(const Ray &p_ray, const OBB &p_obb)
{
	glm::mat3 invOrientation = glm::transpose(p_obb.orientation);
	glm::vec3 localOrigin = invOrientation * (p_ray.pos - p_obb.center);
	glm::vec3 localDirection = invOrientation * p_ray.dir;

	float hitT = 0.0f;
	glm::vec3 localNormal(0.0f);
	if (!RaycastAABBInternal(localOrigin, localDirection, AABB(glm::vec3(0.0f), p_obb.extents), hitT, localNormal))
	{
		return RayHit();
	}

	RayHit hit{};
	hit.didHit = true;
	hit.hitPoint = p_ray.pos + p_ray.dir * hitT;
	hit.hitNormal = glm::normalize(p_obb.orientation * localNormal);
	hit.distance = glm::length(hit.hitPoint - p_ray.pos);
	return hit;
}

RayHit Droplet::Math::Raycast(const Ray &p_ray, const Sphere &p_sphere)
{
	glm::vec3 intersectionPoint;
	glm::vec3 intersectionNormal;
	bool result = glm::intersectRaySphere(p_ray.pos, p_ray.dir, p_sphere.center, p_sphere.radius * p_sphere.radius, intersectionPoint, intersectionNormal);

	RayHit hit{};
	hit.didHit = result;
	hit.distance = glm::length(intersectionPoint - p_ray.pos);
	hit.hitPoint = intersectionPoint;
	hit.hitNormal = intersectionNormal;

	return hit;
}

RayHit Droplet::Math::Raycast(const Ray &p_ray, const Frustum &p_frustum)
{
	RayHit closestHit{};
	closestHit.distance = std::numeric_limits<float>::max();

	for (int i = 0; i < 6; ++i)
	{
		RayHit hit = Raycast(p_ray, p_frustum.GetPlane(i));

		if (!hit.didHit || hit.distance < 0.0f)
		{
			continue;
		}

		if (!Contains(p_frustum, hit.hitPoint))
		{
			continue;
		}

		if (!closestHit.didHit || hit.distance < closestHit.distance)
		{
			closestHit = hit;
		}
	}

	return closestHit;
}


bool Droplet::Math::Contains(const AABB &p_this, const glm::vec3 &p_other)
{
	return IsPointInsideAABB(p_this, p_other);
}

IntersectType Droplet::Math::Intersects(const AABB &p_this, const Plane &p_other)
{
	float signedDistance = SignedDistanceToPlane(p_other, p_this.center);
	float radius = glm::dot(glm::abs(p_other.normal), p_this.extents);

	if (glm::abs(signedDistance) <= radius + EPSILON)
	{
		return IntersectType::Intersects;
	}

	return IntersectType::None;
}

IntersectType Droplet::Math::Intersects(const AABB &p_this, const AABB &p_other)
{
	glm::vec3 thisMin = p_this.GetMin();
	glm::vec3 thisMax = p_this.GetMax();
	glm::vec3 otherMin = p_other.GetMin();
	glm::vec3 otherMax = p_other.GetMax();

	bool overlaps =
		thisMin.x <= otherMax.x + EPSILON && thisMax.x >= otherMin.x - EPSILON &&
		thisMin.y <= otherMax.y + EPSILON && thisMax.y >= otherMin.y - EPSILON &&
		thisMin.z <= otherMax.z + EPSILON && thisMax.z >= otherMin.z - EPSILON;

	if (!overlaps)
	{
		return IntersectType::None;
	}

	bool contains =
		thisMin.x <= otherMin.x + EPSILON && thisMax.x >= otherMax.x - EPSILON &&
		thisMin.y <= otherMin.y + EPSILON && thisMax.y >= otherMax.y - EPSILON &&
		thisMin.z <= otherMin.z + EPSILON && thisMax.z >= otherMax.z - EPSILON;

	return contains ? IntersectType::Contains : IntersectType::Intersects;
}

IntersectType Droplet::Math::Intersects(const AABB &p_this, const OBB &p_other)
{
	if (!IntersectsOBBs(ToOBB(p_this), p_other))
	{
		return IntersectType::None;
	}

	std::array<glm::vec3, 8> obbCorners = GetCorners(p_other);

	for (const glm::vec3 &corner : obbCorners)
	{
		if (!Contains(p_this, corner))
		{
			return IntersectType::Intersects;
		}
	}

	return IntersectType::Contains;
}

IntersectType Droplet::Math::Intersects(const AABB &p_this, const Sphere &p_other)
{
	glm::vec3 closestPoint = glm::clamp(p_other.center, p_this.GetMin(), p_this.GetMax());
	glm::vec3 delta = p_other.center - closestPoint;

	if (glm::dot(delta, delta) > p_other.radius * p_other.radius + EPSILON)
	{
		return IntersectType::None;
	}

	glm::vec3 min = p_this.GetMin();
	glm::vec3 max = p_this.GetMax();
	glm::vec3 radiusVec(p_other.radius);

	bool contains =
		p_other.center.x - radiusVec.x >= min.x - EPSILON && p_other.center.x + radiusVec.x <= max.x + EPSILON &&
		p_other.center.y - radiusVec.y >= min.y - EPSILON && p_other.center.y + radiusVec.y <= max.y + EPSILON &&
		p_other.center.z - radiusVec.z >= min.z - EPSILON && p_other.center.z + radiusVec.z <= max.z + EPSILON;

	return contains ? IntersectType::Contains : IntersectType::Intersects;
}

IntersectType Droplet::Math::Intersects(const AABB &p_this, const Frustum &p_other)
{
	std::array<glm::vec3, 8> aabbCorners = GetCorners(p_this);

	for (int i = 0; i < 6; ++i)
	{
		const Plane &plane = p_other.GetPlane(i);

		bool allOutside = true;
		for (const glm::vec3 &corner : aabbCorners)
		{
			if (SignedDistanceToPlane(plane, corner) >= -EPSILON)
			{
				allOutside = false;
				break;
			}
		}

		if (allOutside)
		{
			return IntersectType::None;
		}
	}

	std::array<glm::vec3, 8> frustumCorners{};
	if (!GetFrustumCorners(p_other, frustumCorners))
	{
		return IntersectType::Intersects;
	}

	for (const glm::vec3 &corner : frustumCorners)
	{
		if (!Contains(p_this, corner))
		{
			return IntersectType::Intersects;
		}
	}

	return IntersectType::Contains;
}


bool Droplet::Math::Contains(const OBB &p_this, const glm::vec3 &p_other)
{
	return IsPointInsideOBB(p_this, p_other);
}

IntersectType Droplet::Math::Intersects(const OBB &p_this, const Plane &p_other)
{
	glm::vec3 axisX = glm::normalize(p_this.orientation[0]);
	glm::vec3 axisY = glm::normalize(p_this.orientation[1]);
	glm::vec3 axisZ = glm::normalize(p_this.orientation[2]);

	float radius =
		p_this.extents.x * glm::abs(glm::dot(p_other.normal, axisX)) +
		p_this.extents.y * glm::abs(glm::dot(p_other.normal, axisY)) +
		p_this.extents.z * glm::abs(glm::dot(p_other.normal, axisZ));

	float signedDistance = SignedDistanceToPlane(p_other, p_this.center);

	if (glm::abs(signedDistance) <= radius + EPSILON)
	{
		return IntersectType::Intersects;
	}

	return IntersectType::None;
}

IntersectType Droplet::Math::Intersects(const OBB &p_this, const AABB &p_other)
{
	if (!IntersectsOBBs(p_this, ToOBB(p_other)))
	{
		return IntersectType::None;
	}

	std::array<glm::vec3, 8> aabbCorners = GetCorners(p_other);

	for (const glm::vec3 &corner : aabbCorners)
	{
		if (!Contains(p_this, corner))
		{
			return IntersectType::Intersects;
		}
	}

	return IntersectType::Contains;
}

IntersectType Droplet::Math::Intersects(const OBB &p_this, const OBB &p_other)
{
	if (!IntersectsOBBs(p_this, p_other))
	{
		return IntersectType::None;
	}

	std::array<glm::vec3, 8> otherCorners = GetCorners(p_other);

	for (const glm::vec3 &corner : otherCorners)
	{
		if (!Contains(p_this, corner))
		{
			return IntersectType::Intersects;
		}
	}

	return IntersectType::Contains;
}

IntersectType Droplet::Math::Intersects(const OBB &p_this, const Sphere &p_other)
{
	glm::vec3 localCenter = glm::transpose(p_this.orientation) * (p_other.center - p_this.center);
	glm::vec3 clamped = glm::clamp(localCenter, -p_this.extents, p_this.extents);
	glm::vec3 closestWorld = p_this.center + p_this.orientation * clamped;
	glm::vec3 delta = p_other.center - closestWorld;

	if (glm::dot(delta, delta) > p_other.radius * p_other.radius + EPSILON)
	{
		return IntersectType::None;
	}

	glm::vec3 radiusVec(p_other.radius);

	bool contains =
		glm::abs(localCenter.x) + radiusVec.x <= p_this.extents.x + EPSILON &&
		glm::abs(localCenter.y) + radiusVec.y <= p_this.extents.y + EPSILON &&
		glm::abs(localCenter.z) + radiusVec.z <= p_this.extents.z + EPSILON;

	return contains ? IntersectType::Contains : IntersectType::Intersects;
}

IntersectType Droplet::Math::Intersects(const OBB &p_this, const Frustum &p_other)
{
	for (int i = 0; i < 6; ++i)
	{
		const Plane &plane = p_other.GetPlane(i);

		float signedDistance = SignedDistanceToPlane(plane, p_this.center);

		float radius =
			p_this.extents.x * glm::abs(glm::dot(plane.normal, p_this.orientation[0])) +
			p_this.extents.y * glm::abs(glm::dot(plane.normal, p_this.orientation[1])) +
			p_this.extents.z * glm::abs(glm::dot(plane.normal, p_this.orientation[2]));

		if (signedDistance < -radius - EPSILON)
		{
			return IntersectType::None;
		}
	}

	std::array<glm::vec3, 8> frustumCorners{};

	if (!GetFrustumCorners(p_other, frustumCorners))
	{
		return IntersectType::Intersects;
	}

	for (const glm::vec3 &corner : frustumCorners)
	{
		if (!Contains(p_this, corner))
		{
			return IntersectType::Intersects;
		}
	}

	return IntersectType::Contains;
}


bool Droplet::Math::Contains(const Sphere &p_this, const glm::vec3 &p_other)
{
	return IsPointInsideSphere(p_this, p_other);
}

IntersectType Droplet::Math::Intersects(const Sphere &p_this, const Plane &p_other)
{
	float distance = glm::abs(SignedDistanceToPlane(p_other, p_this.center));

	return (distance <= p_this.radius + EPSILON) ? IntersectType::Intersects : IntersectType::None;
}

IntersectType Droplet::Math::Intersects(const Sphere &p_this, const AABB &p_other)
{
	glm::vec3 closestPoint = glm::clamp(p_this.center, p_other.GetMin(), p_other.GetMax());
	glm::vec3 delta = p_this.center - closestPoint;

	if (glm::dot(delta, delta) > p_this.radius * p_this.radius + EPSILON)
	{
		return IntersectType::None;
	}

	std::array<glm::vec3, 8> aabbCorners = GetCorners(p_other);

	for (const glm::vec3 &corner : aabbCorners)
	{
		if (!Contains(p_this, corner))
		{
			return IntersectType::Intersects;
		}
	}

	return IntersectType::Contains;
}

IntersectType Droplet::Math::Intersects(const Sphere &p_this, const OBB &p_other)
{
	glm::vec3 localCenter = glm::transpose(p_other.orientation) * (p_this.center - p_other.center);
	glm::vec3 clamped = glm::clamp(localCenter, -p_other.extents, p_other.extents);
	glm::vec3 closestWorld = p_other.center + p_other.orientation * clamped;
	glm::vec3 delta = p_this.center - closestWorld;

	if (glm::dot(delta, delta) > p_this.radius * p_this.radius + EPSILON)
	{
		return IntersectType::None;
	}

	std::array<glm::vec3, 8> obbCorners = GetCorners(p_other);

	for (const glm::vec3 &corner : obbCorners)
	{
		if (!Contains(p_this, corner))
		{
			return IntersectType::Intersects;
		}
	}

	return IntersectType::Contains;
}

IntersectType Droplet::Math::Intersects(const Sphere &p_this, const Sphere &p_other)
{
	glm::vec3 delta = p_other.center - p_this.center;

	float distSquared = glm::dot(delta, delta);
	float radiusSum = p_this.radius + p_other.radius;

	if (distSquared > radiusSum * radiusSum + EPSILON)
	{
		return IntersectType::None;
	}

	float distance = glm::sqrt(glm::max(distSquared, 0.0f));

	if (distance + p_other.radius <= p_this.radius + EPSILON)
	{
		return IntersectType::Contains;
	}

	return IntersectType::Intersects;
}

IntersectType Droplet::Math::Intersects(const Sphere &p_this, const Frustum &p_other)
{
	for (int i = 0; i < 6; ++i)
	{
		float signedDistance = SignedDistanceToPlane(p_other.GetPlane(i), p_this.center);

		if (signedDistance < -p_this.radius - EPSILON)
		{
			return IntersectType::None;
		}
	}

	std::array<glm::vec3, 8> frustumCorners{};

	if (!GetFrustumCorners(p_other, frustumCorners))
	{
		return IntersectType::Intersects;
	}

	for (const glm::vec3 &corner : frustumCorners)
	{
		if (!Contains(p_this, corner))
		{
			return IntersectType::Intersects;
		}
	}

	return IntersectType::Contains;
}


bool Droplet::Math::Contains(const Frustum &p_this, const glm::vec3 &p_other)
{
	return IsPointInsideFrustum(p_this, p_other);
}

IntersectType Droplet::Math::Intersects(const Frustum &p_this, const Plane &p_other)
{
	std::array<glm::vec3, 8> frustumCorners{};

	if (!GetFrustumCorners(p_this, frustumCorners))
	{
		return IntersectType::None;
	}

	bool hasPositive = false;
	bool hasNegative = false;

	for (const glm::vec3 &corner : frustumCorners)
	{
		float signedDistance = SignedDistanceToPlane(p_other, corner);
		hasPositive = hasPositive || signedDistance > EPSILON;
		hasNegative = hasNegative || signedDistance < -EPSILON;

		if (hasPositive && hasNegative)
		{
			return IntersectType::Intersects;
		}
	}

	return IntersectType::None;
}

IntersectType Droplet::Math::Intersects(const Frustum &p_this, const AABB &p_other)
{
	std::array<glm::vec3, 8> aabbCorners = GetCorners(p_other);

	for (int i = 0; i < 6; ++i)
	{
		const Plane &plane = p_this.GetPlane(i);
		bool allOutside = true;

		for (const glm::vec3 &corner : aabbCorners)
		{
			if (SignedDistanceToPlane(plane, corner) >= -EPSILON)
			{
				allOutside = false;
				break;
			}
		}

		if (allOutside)
		{
			return IntersectType::None;
		}
	}

	for (const glm::vec3 &corner : aabbCorners)
	{
		if (!Contains(p_this, corner))
		{
			return IntersectType::Intersects;
		}
	}

	return IntersectType::Contains;
}

IntersectType Droplet::Math::Intersects(const Frustum &p_this, const OBB &p_other)
{
	std::array<glm::vec3, 8> obbCorners = GetCorners(p_other);

	for (int i = 0; i < 6; ++i)
	{
		const Plane &plane = p_this.GetPlane(i);
		bool allOutside = true;

		for (const glm::vec3 &corner : obbCorners)
		{
			if (SignedDistanceToPlane(plane, corner) >= -EPSILON)
			{
				allOutside = false;
				break;
			}
		}

		if (allOutside)
		{
			return IntersectType::None;
		}
	}

	for (const glm::vec3 &corner : obbCorners)
	{
		if (!Contains(p_this, corner))
		{
			return IntersectType::Intersects;
		}
	}

	return IntersectType::Contains;
}

IntersectType Droplet::Math::Intersects(const Frustum &p_this, const Sphere &p_other)
{
	for (int i = 0; i < 6; ++i)
	{
		float signedDistance = SignedDistanceToPlane(p_this.GetPlane(i), p_other.center);

		if (signedDistance < -p_other.radius - EPSILON)
		{
			return IntersectType::None;
		}
	}

	for (int i = 0; i < 6; ++i)
	{
		if (SignedDistanceToPlane(p_this.GetPlane(i), p_other.center) < p_other.radius - EPSILON)
		{
			return IntersectType::Intersects;
		}
	}

	return IntersectType::Contains;
}

IntersectType Droplet::Math::Intersects(const Frustum &p_this, const Frustum &p_other)
{
	std::array<glm::vec3, 8> thisCorners{};
	std::array<glm::vec3, 8> otherCorners{};

	if (!GetFrustumCorners(p_this, thisCorners) || !GetFrustumCorners(p_other, otherCorners))
	{
		return IntersectType::None;
	}

	for (int i = 0; i < 6; ++i)
	{
		const Plane &plane = p_this.GetPlane(i);
		bool allOutside = true;

		for (const glm::vec3 &corner : otherCorners)
		{
			if (SignedDistanceToPlane(plane, corner) >= -EPSILON)
			{
				allOutside = false;
				break;
			}
		}

		if (allOutside)
		{
			return IntersectType::None;
		}
	}

	for (int i = 0; i < 6; ++i)
	{
		const Plane &plane = p_other.GetPlane(i);
		bool allOutside = true;

		for (const glm::vec3 &corner : thisCorners)
		{
			if (SignedDistanceToPlane(plane, corner) >= -EPSILON)
			{
				allOutside = false;
				break;
			}
		}

		if (allOutside)
		{
			return IntersectType::None;
		}
	}

	for (const glm::vec3 &corner : otherCorners)
	{
		if (!Contains(p_this, corner))
		{
			return IntersectType::Intersects;
		}
	}

	return IntersectType::Contains;
}
