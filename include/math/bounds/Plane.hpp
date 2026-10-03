#pragma once
#include <glm/glm.hpp>

namespace Droplet::Math
{
	/// @brief Represents a plane in 3D space defined by a normal vector and a distance from the origin.
	struct Plane
	{
		glm::vec3 normal;
		float distance;

		/// @brief Default constructor initializes the plane with a normal pointing up and a distance of 0.
		Plane() : normal(glm::vec3(0.0f, 1.0f, 0.0f)), distance(0.0f) {}

		/// @brief Constructs a plane with the given normal vector and distance from the origin.
		/// @param p_normal The normal vector of the plane.
		/// @param p_distance The distance from the origin to the plane along the normal vector.
		Plane(const glm::vec3 &p_normal, float p_distance) : normal(p_normal), distance(p_distance) {}

		/// @brief Constructs a plane from three points in 3D space.
		/// @param p1 The first point.
		[[nodiscard]] inline glm::vec3 GetNormal() const
		{
			return normal;
		}

		/// @brief Returns the distance from the origin to the plane along the normal vector.
		/// @return The distance from the origin to the plane.
		[[nodiscard]] inline float GetDistance() const
		{
			return distance;
		}
	};
}
