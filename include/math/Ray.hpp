#pragma once
#include <glm/glm.hpp>

namespace Droplet::Math
{
	/// @brief Represents a ray in 3D space with an origin and direction.
	struct Ray
	{
		glm::vec3 pos;	// Origin of the ray
		glm::vec3 dir;	// Direction of the ray

		/// @brief Calculates the length based on the direction vector.
		/// @return The length of the ray's direction vector.
		[[nodiscard]] inline float GetLength() const
		{
			return glm::length(dir);
		}

		/// @brief Calculates the endpoint of the ray based on its origin and direction.
		/// @return The endpoint of the ray.
		[[nodiscard]] inline glm::vec3 GetEndPoint() const
		{
			return pos + dir;
		}
	};

	/// @brief Transforms a ray by a given transformation matrix.
	/// @note This function does not normalize the direction vector after transformation. 
	/// @param ray The ray to be transformed.
	/// @param transform The transformation matrix to apply to the ray.
	/// @return A new Ray that is the result of transforming the input ray by the given matrix.
	[[nodiscard]] inline Ray RayTransform(const Ray &p_ray, const glm::mat4x4 &p_transform)
	{
		Ray transformedRay{
			.pos = glm::vec3(p_transform * glm::vec4(p_ray.pos, 1.0f)),
			.dir = glm::vec3(p_transform * glm::vec4(p_ray.dir, 0.0f))
		};
		return transformedRay;
	}
}