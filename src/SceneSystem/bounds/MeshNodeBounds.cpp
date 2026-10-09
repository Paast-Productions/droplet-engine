#include "SceneSystem/bounds/MeshNodeBounds.hpp"
#include <SceneSystem/Components/MeshComponent.hpp>
#include <math/bounds/Conversion.hpp>
#include <resource/types/MeshResource.hpp>
#include <math/bounds/MeshBVH.hpp>

using namespace Droplet::Scene;
using namespace Droplet::Math;

static OBB GetOBB(const MeshNodeBounds &p_bounds, const glm::mat4x4 &p_worldMat)
{
	OBB obb = AABBToOBB(p_bounds.GetAABB());

	return OBB::Transform(obb, p_worldMat);
}

IntersectType MeshNodeBounds::Intersect(const Plane &p_plane, const glm::mat4x4 &p_worldMat) const
{
	OBB obb = GetOBB(*this, p_worldMat);
	return Intersects(obb, p_plane);
}

IntersectType MeshNodeBounds::Intersect(const AABB &p_aabb, const glm::mat4x4 &p_worldMat) const
{
	OBB obb = GetOBB(*this, p_worldMat);
	return Intersects(obb, p_aabb);
}

IntersectType MeshNodeBounds::Intersect(const OBB &p_obb, const glm::mat4x4 &p_worldMat) const
{
	OBB obb = GetOBB(*this, p_worldMat);
	return Intersects(obb, p_obb);
}

IntersectType MeshNodeBounds::Intersect(const Sphere &p_sphere, const glm::mat4x4 &p_worldMat) const
{
	OBB obb = GetOBB(*this, p_worldMat);
	return Intersects(obb, p_sphere);
}

IntersectType MeshNodeBounds::Intersect(const Frustum &p_frustum, const glm::mat4x4 &p_worldMat) const
{
	OBB obb = GetOBB(*this, p_worldMat);
	return Intersects(obb, p_frustum);
}

RayHit MeshNodeBounds::Raycast(const Ray &p_ray, const glm::mat4x4 &p_worldMat) const
{
	// Transform the ray into the local space of the mesh to perform the raycast against the mesh's bounding volume hierarchy (BVH)
	glm::mat4x4 invWorldMat = glm::inverse(p_worldMat);
	Ray localRay = Ray::Transform(p_ray, invWorldMat);

	const std::weak_ptr<MeshResource> meshResource = m_meshComponent->GetMeshResource();

	RayHit hit = meshResource.lock()->GetBVH().Raycast(localRay);

	if (hit.didHit)
	{
		// Transform the hit point and normal back to world space
		hit.point = glm::vec3(p_worldMat * glm::vec4(hit.point, 1.0f));
		hit.normal = glm::normalize(glm::vec3(p_worldMat * glm::vec4(hit.normal, 0.0f)));
		hit.distance = glm::length(hit.point - p_ray.pos);
	}

	return hit;
}

AABB MeshNodeBounds::GetAABB() const
{
	const std::weak_ptr<MeshResource> meshResource = m_meshComponent->GetMeshResource();
	return meshResource.lock()->GetBVH().GetBounds();
}
