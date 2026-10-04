#pragma once
#include <math/bounds/Ray.hpp>
#include <math/bounds/Plane.hpp>
#include <math/bounds/AABB.hpp>
#include <math/bounds/OBB.hpp>
#include <math/bounds/Sphere.hpp>
#include <math/bounds/Frustum.hpp>

namespace Droplet::Math
{
	// Types

	/// @brief Enum representing the type of intersection between two geometric objects.
	enum class IntersectType
	{
		None,
		Intersects,
		Contains,
	};

	/// @brief Struct for storing information about a ray hit.
	struct RayHit
	{
		glm::vec3	hitPoint = glm::vec3(0.0f);
		glm::vec3	hitNormal = glm::vec3(0.0f);
		float		distance = 0.0f;
		bool		didHit = false;
	};


	// Raycast

	/// @brief Performs a raycast against a plane and returns information about the hit.
	/// @param p_ray The ray to cast.
	/// @param p_plane The plane to test against.
	/// @return A RayHit struct containing information about the hit, including the hit point, normal, distance, and whether a hit occurred.
	[[nodiscard]] constexpr RayHit Raycast(const Ray &p_ray, const Plane &p_plane);

	/// @brief Performs a raycast against an axis-aligned bounding box (AABB) and returns information about the hit.
	/// @param p_ray The ray to cast.
	/// @param p_aabb The AABB to test against.
	/// @return A RayHit struct containing information about the hit, including the hit point, normal, distance, and whether a hit occurred.
	[[nodiscard]] constexpr RayHit Raycast(const Ray &p_ray, const AABB &p_aabb);

	/// @brief Performs a raycast against an oriented bounding box (OBB) and returns information about the hit.
	/// @param p_ray The ray to cast.
	/// @param p_obb The OBB to test against.
	/// @return A RayHit struct containing information about the hit, including the hit point, normal, distance, and whether a hit occurred.
	[[nodiscard]] constexpr RayHit Raycast(const Ray &p_ray, const OBB &p_obb);

	/// @brief Performs a raycast against a sphere and returns information about the hit.
	/// @param p_ray The ray to cast.
	/// @param p_sphere The sphere to test against.
	/// @return A RayHit struct containing information about the hit, including the hit point, normal, distance, and whether a hit occurred.
	[[nodiscard]] constexpr RayHit Raycast(const Ray &p_ray, const Sphere &p_sphere);

	/// @brief Performs a raycast against a frustum and returns information about the hit.
	/// @param p_ray The ray to cast.
	/// @param p_frustum The frustum to test against.
	/// @return A RayHit struct containing information about the hit, including the hit point, normal, distance, and whether a hit occurred.
	[[nodiscard]] constexpr RayHit Raycast(const Ray &p_ray, const Frustum &p_frustum);

	// AABB

	/// @brief Determines whether an AABB contains a given point.
	/// @param p_this The AABB to test.
	/// @param p_other The point to test against.
	/// @return True if the AABB contains the point, false otherwise.
	[[nodiscard]] constexpr bool Contains(const AABB &p_this, const glm::vec3 &p_other);

	/// @brief Determines whether an axis-aligned bounding box (AABB) intersects with a plane.
	/// @param p_this The AABB to test.
	/// @param p_other The plane to test against.
	/// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
	[[nodiscard]] constexpr IntersectType Intersects(const AABB &p_this, const Plane &p_other);

	/// @brief Determines whether two axis-aligned bounding boxes (AABBs) intersect.
	/// @param p_this The AABB to test.
	/// @param p_other The AABB to test against.
	/// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
	[[nodiscard]] constexpr IntersectType Intersects(const AABB &p_this, const AABB &p_other);

	/// @brief Determines whether an axis-aligned bounding box (AABB) intersects with an oriented bounding box (OBB).
	/// @param p_this The AABB to test.
	/// @param p_other The OBB to test against.
	/// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
	[[nodiscard]] constexpr IntersectType Intersects(const AABB &p_this, const OBB &p_other);

	/// @brief Determines whether an axis-aligned bounding box (AABB) intersects with a sphere.
	/// @param p_this The AABB to test.
	/// @param p_other The sphere to test against.
	/// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
	[[nodiscard]] constexpr IntersectType Intersects(const AABB &p_this, const Sphere &p_other);

	/// @brief Determines whether an axis-aligned bounding box (AABB) intersects with a frustum.
	/// @param p_this The AABB to test.
	/// @param p_other The frustum to test against.
	/// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
	[[nodiscard]] constexpr IntersectType Intersects(const AABB &p_this, const Frustum &p_other);


	// OBB

	/// @brief Determines whether an oriented bounding box (OBB) contains a given point.
	/// @param p_this The OBB to test.
	/// @param p_other The point to test against.
	/// @return True if the OBB contains the point, false otherwise.
	[[nodiscard]] constexpr bool Contains(const OBB &p_this, const glm::vec3 &p_other);

	/// @brief Determines whether an oriented bounding box (OBB) intersects with a plane.
	/// @param p_this The OBB to test.
	/// @param p_other The plane to test against.
	/// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
	[[nodiscard]] constexpr IntersectType Intersects(const OBB &p_this, const Plane &p_other);

	/// @brief Determines whether an oriented bounding box (OBB) intersects with an axis-aligned bounding box (AABB).
	/// @param p_this The OBB to test.
	/// @param p_other The AABB to test against.
	/// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
	[[nodiscard]] constexpr IntersectType Intersects(const OBB &p_this, const AABB &p_other);

	/// @brief Determines whether two oriented bounding boxes (OBBs) intersect.
	/// @param p_this The OBB to test.
	/// @param p_other The OBB to test against.
	/// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
	[[nodiscard]] constexpr IntersectType Intersects(const OBB &p_this, const OBB &p_other);

	/// @brief Determines whether an oriented bounding box (OBB) intersects with a sphere.
	/// @param p_this The OBB to test.
	/// @param p_other The sphere to test against.
	/// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
	[[nodiscard]] constexpr IntersectType Intersects(const OBB &p_this, const Sphere &p_other);

	/// @brief Determines whether an oriented bounding box (OBB) intersects with a frustum.
	/// @param p_this The OBB to test.
	/// @param p_other The frustum to test against.
	/// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
	[[nodiscard]] constexpr IntersectType Intersects(const OBB &p_this, const Frustum &p_other);


	// Sphere

	/// @brief Determines whether a sphere contains a given point.
	/// @param p_this The sphere to test.
	/// @param p_other The point to test against.
	/// @return True if the Sphere contains the point, false otherwise.
	[[nodiscard]] constexpr bool Contains(const Sphere &p_this, const glm::vec3 &p_other);

	/// @brief Determines whether a sphere intersects with a plane.
	/// @param p_this The sphere to test.
	/// @param p_other The plane to test against.
	/// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
	[[nodiscard]] constexpr IntersectType Intersects(const Sphere &p_this, const Plane &p_other);

	/// @brief Determines whether a sphere intersects with an axis-aligned bounding box (AABB).
	/// @param p_this The sphere to test.
	/// @param p_other The AABB to test against.
	/// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
	[[nodiscard]] constexpr IntersectType Intersects(const Sphere &p_this, const AABB &p_other);

	/// @brief Determines whether a sphere intersects with an oriented bounding box (OBB).
	/// @param p_this The sphere to test.
	/// @param p_other The OBB to test against.
	/// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
	[[nodiscard]] constexpr IntersectType Intersects(const Sphere &p_this, const OBB &p_other);

	/// @brief Determines whether a sphere intersects with another sphere.
	/// @param p_this The sphere to test.
	/// @param p_other The sphere to test against.
	/// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
	[[nodiscard]] constexpr IntersectType Intersects(const Sphere &p_this, const Sphere &p_other);

	/// @brief Determines whether a sphere intersects with a frustum.
	/// @param p_this The sphere to test.
	/// @param p_other The frustum to test against.
	/// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
	[[nodiscard]] constexpr IntersectType Intersects(const Sphere &p_this, const Frustum &p_other);


	// Frustum

	/// @brief Determines whether a frustum contains a given point.
	/// @param p_this The frustum to test.
	/// @param p_other The point to test against.
	/// @return True if the Frustum contains the point, false otherwise.
	[[nodiscard]] constexpr bool Contains(const Frustum &p_this, const glm::vec3 &p_other);

	/// @brief Determines whether a frustum intersects with a plane.
	/// @param p_this The frustum to test.
	/// @param p_other The plane to test against.
	/// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
	[[nodiscard]] constexpr IntersectType Intersects(const Frustum &p_this, const Plane &p_other);

	/// @brief Determines whether a frustum intersects with an axis-aligned bounding box (AABB).
	/// @param p_this The frustum to test.
	/// @param p_other The AABB to test against.
	/// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
	[[nodiscard]] constexpr IntersectType Intersects(const Frustum &p_this, const AABB &p_other);

	/// @brief Determines whether a frustum intersects with an oriented bounding box (OBB).
	/// @param p_this The frustum to test.
	/// @param p_other The OBB to test against.
	/// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
	[[nodiscard]] constexpr IntersectType Intersects(const Frustum &p_this, const OBB &p_other);

	/// @brief Determines whether a frustum intersects with a sphere.
	/// @param p_this The frustum to test.
	/// @param p_other The sphere to test against.
	/// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
	[[nodiscard]] constexpr IntersectType Intersects(const Frustum &p_this, const Sphere &p_other);

	/// @brief Determines whether two frustums intersect.
	/// @param p_this The frustum to test.
	/// @param p_other The frustum to test against.
	/// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
	[[nodiscard]] constexpr IntersectType Intersects(const Frustum &p_this, const Frustum &p_other);
}