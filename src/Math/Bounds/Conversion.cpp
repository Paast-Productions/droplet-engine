#include "Conversion.hpp"

using namespace Droplet::Math;

OBB Droplet::Math::AABBToOBB(const AABB &p_aabb)
{
	return OBB(p_aabb.GetCenter(), p_aabb.GetExtents(), glm::mat3(1.0f));
}

AABB Droplet::Math::OBBToAABB(const OBB &p_obb)
{
	glm::vec3 corners[8]{};
	p_obb.GetCorners(corners);

	return AABB::FromPoints(corners, 8);
}

AABB Droplet::Math::SphereToAABB(const Sphere &p_sphere)
{
	return AABB(p_sphere.GetCenter(), glm::vec3(p_sphere.GetRadius()));
}

AABB Droplet::Math::FrustumToAABB(const Frustum &p_frustum)
{
	// TODO: Implement Frustum to AABB conversion
	assert(false);
	return AABB();
}

OBB Droplet::Math::FrustumToOBB(const Frustum &p_frustum)
{
	// TODO: Implement Frustum to OBB conversion
	assert(false);
	return OBB();
}
