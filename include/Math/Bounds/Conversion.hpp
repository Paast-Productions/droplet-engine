#pragma once
#include <math/bounds/Ray.hpp>
#include <math/bounds/Plane.hpp>
#include <math/bounds/AABB.hpp>
#include <math/bounds/OBB.hpp>
#include <math/bounds/Sphere.hpp>
#include <math/bounds/Frustum.hpp>

namespace Droplet::Math
{
	// Utility functions for converting between different bounding volumes.

	[[nodiscard]] OBB AABBToOBB(const AABB &p_aabb);
	[[nodiscard]] AABB OBBToAABB(const OBB &p_obb);
	[[nodiscard]] AABB SphereToAABB(const Sphere &p_sphere);
	[[nodiscard]] AABB FrustumToAABB(const Frustum &p_frustum);
	[[nodiscard]] OBB FrustumToOBB(const Frustum &p_frustum);
}