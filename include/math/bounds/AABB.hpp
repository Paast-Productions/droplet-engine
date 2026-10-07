#pragma once
#include <glm/glm.hpp>

namespace Droplet::Math
{
	/// @brief Axis-Aligned Bounding Box (AABB) structure.
	struct AABB
	{
		glm::vec3 center;
		glm::vec3 extents;

		/// @brief Default constructor initializes the AABB with a center at the origin and zero extents.
		AABB() : center(glm::vec3(0.0f)), extents(glm::vec3(0.0f)) {}

		/// @brief Constructs an AABB with the specified center and extents.
		/// @param p_center The center of the AABB.
		/// @param p_extents The extents (half-sizes) of the AABB.
		AABB(const glm::vec3 &p_center, const glm::vec3 &p_extents) : center(p_center), extents(p_extents) {}

		/// @brief Gets the center of the AABB.
		/// @return The center of the AABB.
		[[nodiscard]] inline glm::vec3 GetCenter() const
		{
			return center;
		}

		/// @brief Gets the extents (half-sizes) of the AABB.
		/// @return The extents of the AABB.
		[[nodiscard]] inline glm::vec3 GetExtents() const
		{
			return extents;
		}

		/// @brief Gets the size (full dimensions) of the AABB.
		/// @return The size of the AABB.
		[[nodiscard]] inline glm::vec3 GetSize() const
		{
			return extents * 2.0f;
		}

		/// @brief Gets the maximum corner of the AABB.
		/// @return The maximum corner of the AABB.
		[[nodiscard]] inline glm::vec3 GetMax() const
		{
			return center + extents;
		}

		/// @brief Gets the minimum corner of the AABB.
		/// @return The minimum corner of the AABB.
		[[nodiscard]] inline glm::vec3 GetMin() const
		{
			return center - extents;
		}

		// Static

		/// @brief Creates an AABB from the given minimum and maximum corners.
		/// @param p_min The minimum corner of the AABB.
		/// @param p_max The maximum corner of the AABB.
		/// @return An AABB that encompasses the given minimum and maximum corners.
		[[nodiscard]] static inline AABB FromMinMax(const glm::vec3 &p_min, const glm::vec3 &p_max)
		{
			return AABB((p_min + p_max) * 0.5f,  (p_max - p_min) * 0.5f);
		}

		/// @brief Creates an AABB that encompasses a set of points.
		/// @param p_points A vector of points to encompass.
		/// @return An AABB that encompasses all the given points.
		[[nodiscard]] static inline AABB FromPoints(const std::vector<glm::vec3> &p_points)
		{
			glm::vec3 min = p_points[0];
			glm::vec3 max = p_points[0];

			for (size_t i = 1; i < p_points.size(); ++i)
			{
				const glm::vec3 &point = p_points[i];

				min = glm::min(min, point);
				max = glm::max(max, point);
			}

			return FromMinMax(min, max);
		}

		/// @brief Transforms an AABB by a given transformation matrix.
		/// @param p_aabb The AABB to transform.
		/// @param p_mat The transformation matrix.
		/// @return A new AABB that is the result of transforming the original AABB by the given matrix.
		[[nodiscard]] static inline AABB Transform(const AABB &p_aabb, const glm::mat4x4 &p_mat)
		{
			glm::vec3 newCenter = glm::vec3(p_mat * glm::vec4(p_aabb.center, 1.0f));

			glm::vec3 newExtents = glm::vec3(
				glm::length(glm::vec3(p_mat[0]) * p_aabb.extents.x) +
				glm::length(glm::vec3(p_mat[1]) * p_aabb.extents.y) +
				glm::length(glm::vec3(p_mat[2]) * p_aabb.extents.z)
			);

			return AABB(newCenter, newExtents);
		}
	};
}
