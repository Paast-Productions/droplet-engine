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
		[[nodiscard]] inline glm::vec3 GetMin() const
		{
			return center - extents;
		}

		// Static

		/// @brief Creates an AABB from the given minimum and maximum corners.
		/// @param min The minimum corner of the AABB.
		/// @param max The maximum corner of the AABB.
		/// @return An AABB that encompasses the given minimum and maximum corners.
		[[nodiscard]] static inline AABB FromMinMax(const glm::vec3 &min, const glm::vec3 &max)
		{
			return AABB((min + max) * 0.5f,  (max - min) * 0.5f);
		}

		/// @brief Creates an AABB that encompasses a set of points.
		/// @param points A vector of points to encompass.
		/// @return An AABB that encompasses all the given points.
		[[nodiscard]] static inline AABB FromPoints(const std::vector<glm::vec3> &points)
		{
			glm::vec3 min = points[0];
			glm::vec3 max = points[0];

			for (size_t i = 1; i < points.size(); ++i)
			{
				const glm::vec3 &point = points[i];

				min = glm::min(min, point);
				max = glm::max(max, point);
			}

			return FromMinMax(min, max);
		}

		/// @brief Transforms an AABB by a given transformation matrix.
		/// @param aabb The AABB to transform.
		/// @param mat The transformation matrix.
		/// @return A new AABB that is the result of transforming the original AABB by the given matrix.
		[[nodiscard]] static inline AABB Transform(const AABB &aabb, const glm::mat4x4 &mat)
		{
			glm::vec3 newCenter = glm::vec3(mat * glm::vec4(aabb.center, 1.0f));

			glm::vec3 newExtents = glm::vec3(
				glm::length(glm::vec3(mat[0]) * aabb.extents.x) +
				glm::length(glm::vec3(mat[1]) * aabb.extents.y) +
				glm::length(glm::vec3(mat[2]) * aabb.extents.z)
			);

			return AABB(newCenter, newExtents);
		}
	};
}
