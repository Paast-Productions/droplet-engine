#pragma once
#include <glm/glm.hpp>

namespace Droplet::Math
{
	/// @brief Oriented Bounding Box (OBB) representation.
	struct OBB
	{
		glm::vec3 center;
		glm::vec3 extents;
		glm::mat3 orientation;

		/// @brief Default constructor initializes the OBB with zero center, zero extents, and identity orientation.
		OBB() : center(glm::vec3(0.0f)), extents(glm::vec3(0.0f)), orientation(glm::mat3(1.0f)) {}

		/// @brief Constructs an OBB with the specified center, extents, and orientation.
		/// @param p_center The center of the OBB.
		/// @param p_extents The half-sizes of the OBB along each axis.
		/// @param p_orientation The orientation of the OBB represented as a 3x3 rotation matrix.
		OBB(const glm::vec3 &p_center, const glm::vec3 &p_extents, const glm::mat3 &p_orientation) : 
			center(p_center), extents(p_extents), orientation(p_orientation) {}

		/// @brief Constructs an OBB with the specified center, extents, and orientation represented as a quaternion.
		/// @param p_center The center of the OBB.
		/// @param p_extents The half-sizes of the OBB along each axis.
		/// @param p_quat The orientation of the OBB represented as a quaternion.
		OBB(const glm::vec3 &p_center, const glm::vec3 &p_extents, const glm::quat &p_quat);

		/// @brief Returns the center of the OBB.
		/// @return The center of the OBB as a glm::vec3.
		[[nodiscard]] inline glm::vec3 GetCenter() const
		{
			return center;
		}

		/// @brief Returns the extents (half-sizes) of the OBB.
		/// @return The extents of the OBB as a glm::vec3.
		[[nodiscard]] inline glm::vec3 GetExtents() const
		{
			return extents;
		}

		/// @brief Returns the size (full dimensions) of the OBB.
		/// @return The size of the OBB as a glm::vec3.
		[[nodiscard]] inline glm::vec3 GetSize() const
		{
			return extents * 2.0f;
		}

		/// @brief Returns the orientation of the OBB as a 3x3 rotation matrix.
		/// @return The orientation of the OBB as a glm::mat3.
		[[nodiscard]] inline glm::mat3 GetOrientation() const
		{
			return orientation;
		}

		/// @brief Computes the eight corner points of the OBB and stores them in the provided array.
		/// @param p_corners An array of 8 glm::vec3 to store the corner points of the OBB.
		void GetCorners(glm::vec3 p_corners[8]) const;

		// Static

		/// @brief Creates an OBB that encompasses a set of points.
		/// @note This function computes an OBB fitted for the given points using PCA (Principal Component Analysis).
		/// @param p_points A vector of points to encompass.
		/// @return An OBB that encompasses all the given points.
		[[nodiscard]] static OBB FromPoints(const std::vector<glm::vec3> &p_points);

		/// @brief Transforms the given OBB by the specified transformation matrix.
		/// @param p_obb The OBB to be transformed.
		/// @param p_mat The transformation matrix to apply to the OBB.
		/// @return A new OBB that is the result of transforming the input OBB by the given matrix.
		[[nodiscard]] static inline OBB Transform(const OBB &p_obb, const glm::mat4x4 &p_mat)
		{
			glm::vec3 newCenter = glm::vec3(p_mat * glm::vec4(p_obb.center, 1.0f));
			glm::mat3 newOrientation = glm::mat3(p_mat) * p_obb.orientation;

			// Scale the extents based on the transformation matrix
			glm::vec3 scale{
				glm::length(glm::vec3(p_mat[0])),
				glm::length(glm::vec3(p_mat[1])),
				glm::length(glm::vec3(p_mat[2]))
			};

			glm::vec3 newExtents = p_obb.extents * scale;

			return OBB(newCenter, newExtents, newOrientation);
		}
	};
}
