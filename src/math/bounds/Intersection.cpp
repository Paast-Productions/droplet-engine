#include "math/bounds/Intersection.hpp"
#include <glm/gtx/closest_point.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/extended_min_max.hpp>

using namespace Droplet::Math;


RayHit Droplet::Math::Raycast([[maybe_unused]] const Ray &p_ray, [[maybe_unused]] const Plane &p_plane)
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

RayHit Droplet::Math::Raycast([[maybe_unused]] const Ray &p_ray, [[maybe_unused]] const AABB &p_aabb)
{
	// TODO

	return RayHit();
}

RayHit Droplet::Math::Raycast([[maybe_unused]] const Ray &p_ray, [[maybe_unused]] const OBB &p_obb)
{
	// TODO

	return RayHit();
}

RayHit Droplet::Math::Raycast([[maybe_unused]] const Ray &p_ray, [[maybe_unused]] const Sphere &p_sphere)
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

RayHit Droplet::Math::Raycast([[maybe_unused]] const Ray &p_ray, [[maybe_unused]] const Frustum &p_frustum)
{
	// TODO

	return RayHit();
}


bool Droplet::Math::Contains([[maybe_unused]] const AABB &p_this, [[maybe_unused]] const glm::vec3 &p_other)
{
	// TODO

	return false;
}

IntersectType Droplet::Math::Intersects([[maybe_unused]] const AABB &p_this, [[maybe_unused]] const Plane &p_other)
{
	// TODO

	return IntersectType();
}

IntersectType Droplet::Math::Intersects([[maybe_unused]] const AABB &p_this, [[maybe_unused]] const AABB &p_other)
{
	// TODO

	return IntersectType();
}

IntersectType Droplet::Math::Intersects([[maybe_unused]] const AABB &p_this, [[maybe_unused]] const OBB &p_other)
{
	// TODO

	return IntersectType();
}

IntersectType Droplet::Math::Intersects([[maybe_unused]] const AABB &p_this, [[maybe_unused]] const Sphere &p_other)
{
	// TODO

	return IntersectType();
}

IntersectType Droplet::Math::Intersects([[maybe_unused]] const AABB &p_this, [[maybe_unused]] const Frustum &p_other)
{
	// TODO

	return IntersectType();
}


bool Droplet::Math::Contains([[maybe_unused]] const OBB &p_this, [[maybe_unused]] const glm::vec3 &p_other)
{
	// TODO

	return false;
}

IntersectType Droplet::Math::Intersects([[maybe_unused]] const OBB &p_this, [[maybe_unused]] const Plane &p_other)
{
	// TODO

	return IntersectType();
}

IntersectType Droplet::Math::Intersects([[maybe_unused]] const OBB &p_this, [[maybe_unused]] const AABB &p_other)
{
	// TODO

	return IntersectType();
}

IntersectType Droplet::Math::Intersects([[maybe_unused]] const OBB &p_this, [[maybe_unused]] const OBB &p_other)
{
	// TODO

	return IntersectType();
}

IntersectType Droplet::Math::Intersects([[maybe_unused]] const OBB &p_this, [[maybe_unused]] const Sphere &p_other)
{
	// TODO

	return IntersectType();
}

IntersectType Droplet::Math::Intersects([[maybe_unused]] const OBB &p_this, [[maybe_unused]] const Frustum &p_other)
{
	// TODO

	return IntersectType();
}


bool Droplet::Math::Contains([[maybe_unused]] const Sphere &p_this, [[maybe_unused]] const glm::vec3 &p_other)
{
	// TODO

	return false;
}

IntersectType Droplet::Math::Intersects([[maybe_unused]] const Sphere &p_this, [[maybe_unused]] const Plane &p_other)
{
	// TODO

	return IntersectType();
}

IntersectType Droplet::Math::Intersects([[maybe_unused]] const Sphere &p_this, [[maybe_unused]] const AABB &p_other)
{
	// TODO

	return IntersectType();
}

IntersectType Droplet::Math::Intersects([[maybe_unused]] const Sphere &p_this, [[maybe_unused]] const OBB &p_other)
{
	// TODO

	return IntersectType();
}

IntersectType Droplet::Math::Intersects([[maybe_unused]] const Sphere &p_this, [[maybe_unused]] const Sphere &p_other)
{
	// TODO

	return IntersectType();
}

IntersectType Droplet::Math::Intersects([[maybe_unused]] const Sphere &p_this, [[maybe_unused]] const Frustum &p_other)
{
	// TODO

	return IntersectType();
}


bool Droplet::Math::Contains([[maybe_unused]] const Frustum &p_this, [[maybe_unused]] const glm::vec3 &p_other)
{
	// TODO

	return false;
}

IntersectType Droplet::Math::Intersects([[maybe_unused]] const Frustum &p_this, [[maybe_unused]] const Plane &p_other)
{
	// TODO

	return IntersectType();
}

IntersectType Droplet::Math::Intersects([[maybe_unused]] const Frustum &p_this, [[maybe_unused]] const AABB &p_other)
{
	// TODO

	return IntersectType();
}

IntersectType Droplet::Math::Intersects([[maybe_unused]] const Frustum &p_this, [[maybe_unused]] const OBB &p_other)
{
	// TODO

	return IntersectType();
}

IntersectType Droplet::Math::Intersects([[maybe_unused]] const Frustum &p_this, [[maybe_unused]] const Sphere &p_other)
{
	// TODO

	return IntersectType();
}

IntersectType Droplet::Math::Intersects([[maybe_unused]] const Frustum &p_this, [[maybe_unused]] const Frustum &p_other)
{
	// TODO

	return IntersectType();
}
