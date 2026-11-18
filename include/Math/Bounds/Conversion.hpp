#pragma once
#include <Math/Bounds/Ray.hpp>
#include <Math/Bounds/Plane.hpp>
#include <Math/Bounds/AABB.hpp>
#include <Math/Bounds/OBB.hpp>
#include <Math/Bounds/Sphere.hpp>
#include <Math/Bounds/Frustum.hpp>

namespace Droplet::Math
{
	// Utility functions for converting between different bounding volumes.

	[[nodiscard]] OBB AABBToOBB(const AABB &p_aabb);
	[[nodiscard]] AABB OBBToAABB(const OBB &p_obb);
	[[nodiscard]] AABB SphereToAABB(const Sphere &p_sphere);
	[[nodiscard]] AABB FrustumToAABB(const Frustum &p_frustum);
	[[nodiscard]] OBB FrustumToOBB(const Frustum &p_frustum);
}