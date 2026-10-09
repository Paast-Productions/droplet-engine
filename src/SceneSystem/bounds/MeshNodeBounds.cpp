#include "SceneSystem/bounds/MeshNodeBounds.hpp"
#include <SceneSystem/Components/MeshComponent.hpp>
#include <resource/types/MeshResource.hpp>
#include <math/bounds/MeshBVH.hpp>

using namespace Droplet::Scene;
using namespace Droplet::Math;

IntersectType MeshNodeBounds::Intersect(const Plane &p_plane, const glm::mat4x4 &p_worldMat) const
{
	// TODO

	OBB obb = GetAABB()
}

IntersectType MeshNodeBounds::Intersect(const AABB &p_aabb, const glm::mat4x4 &p_worldMat) const
{
	// TODO
}

IntersectType MeshNodeBounds::Intersect(const OBB &p_obb, const glm::mat4x4 &p_worldMat) const
{
	// TODO
}

IntersectType MeshNodeBounds::Intersect(const Sphere &p_sphere, const glm::mat4x4 &p_worldMat) const
{
	// TODO
}

RayHit MeshNodeBounds::Raycast(const Ray &p_ray, const glm::mat4x4 &p_worldMat) const
{
	// TODO
}

AABB MeshNodeBounds::GetAABB() const
{
	const std::weak_ptr<MeshResource> meshResource = m_meshComponent.lock()->GetMeshResource();
	return meshResource.lock()->GetBVH().GetBounds();
}
