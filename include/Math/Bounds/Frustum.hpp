#pragma once
#include <glm/glm.hpp>
#include <math/bounds/Plane.hpp>

namespace Droplet::Math
{
	/// @brief A frustum is a geometric shape that represents the visible area of a camera in 3D space. It is defined by six planes: left, right, top, bottom, near, and far.
	struct Frustum
	{
		Plane planes[6];

		/// @brief Constructs a new Frustum object with default planes.
		Frustum() = default;

		/// @brief Constructs a new Frustum object with the specified planes.
		/// @param p_left The left plane of the frustum.
		/// @param p_right The right plane of the frustum.
		/// @param p_top The top plane of the frustum.
		/// @param p_bottom The bottom plane of the frustum.
		/// @param p_near The near plane of the frustum.
		/// @param p_far The far plane of the frustum.
		Frustum(const Plane &p_left, const Plane &p_right, const Plane &p_top, const Plane &p_bottom, const Plane &p_near, const Plane &p_far)
		{
			planes[0] = p_left;
			planes[1] = p_right;
			planes[2] = p_top;
			planes[3] = p_bottom;
			planes[4] = p_near;
			planes[5] = p_far;
		}

		/// @brief Constructs a new Frustum object with the specified planes.
		/// @param p_planes An array of pointers to the six planes that define the frustum.
		Frustum(const Plane *const p_planes[6])
		{
			for (int i = 0; i < 6; ++i)
			{
				planes[i] = *p_planes[i];
			}
		}
		
		/// @brief Constructs a new Frustum object from a projection matrix.
		/// @param p_matrix The projection matrix from which to extract the frustum planes.
		Frustum(const glm::mat4x4 &p_matrix);

		/// @brief Constructs a new Frustum object with the specified view parameters.
		/// @param p_origin The view origin (camera position).
		/// @param p_fwd The view direction (camera forward vector).
		/// @param p_up The view up vector (camera up vector).
		/// @param p_fov The horizontal field of view angle in radians.
		/// @param p_aspect The aspect ratio of the frustum (width / height).
		/// @param p_near The distance to the near clipping plane.
		/// @param p_far The distance to the far clipping plane.
		Frustum(const glm::vec3 &p_origin, const glm::vec3 &p_fwd, const glm::vec3 &p_up, float p_fov, float p_aspect, float p_near, float p_far);

		/// @brief Gets the plane at the specified index.
		/// @param p_index The index of the plane to retrieve (0-5).
		/// @return The plane at the specified index.
		[[nodiscard]] inline Plane &GetPlane(int p_index)
		{
			return planes[p_index];
		}
		[[nodiscard]] inline const Plane &GetPlane(int p_index) const
		{
			return planes[p_index];
		}

		/// @brief Gets the left plane of the frustum.
		/// @return The left plane of the frustum.
		[[nodiscard]] inline Plane &GetLeft()
		{
			return planes[0];
		}
		[[nodiscard]] inline const Plane &GetLeft() const
		{
			return planes[0];
		}

		/// @brief Gets the right plane of the frustum.
		/// @return The right plane of the frustum.
		[[nodiscard]] inline Plane &GetRight()
		{
			return planes[1];
		}
		[[nodiscard]] inline const Plane &GetRight() const
		{
			return planes[1];
		}

		/// @brief Gets the top plane of the frustum.
		/// @return The top plane of the frustum.
		[[nodiscard]] inline Plane &GetTop()
		{
			return planes[2];
		}
		[[nodiscard]] inline const Plane &GetTop() const
		{
			return planes[2];
		}

		/// @brief Gets the bottom plane of the frustum.
		/// @return The bottom plane of the frustum.
		[[nodiscard]] inline Plane &GetBottom()
		{
			return planes[3];
		}
		[[nodiscard]] inline const Plane &GetBottom() const
		{
			return planes[3];
		}

		/// @brief Gets the near plane of the frustum.
		/// @return The near plane of the frustum.
		[[nodiscard]] inline Plane &GetNear()
		{
			return planes[4];
		}
		[[nodiscard]] inline const Plane &GetNear() const
		{
			return planes[4];
		}

		/// @brief Gets the far plane of the frustum.
		/// @return The far plane of the frustum.
		[[nodiscard]] inline Plane &GetFar()
		{
			return planes[5];
		}
		[[nodiscard]] inline const Plane &GetFar() const
		{
			return planes[5];
		}

		/// @brief Calculates the eight corners of the frustum and stores them in the provided array.
		/// @param p_corners An array of eight glm::vec3 objects to store the corners of the frustum.
		/// @details Corners are ordered as follows:
		///		(left,	top,	near)
		///		(right, top,	near)
		///		(left,	bottom, near)
		///		(right, bottom, near)
		///		(left,	top,	far)
		///		(right, top,	far)
		///		(left,	bottom, far)
		///		(right, bottom, far)
		void GetCorners(glm::vec3 *p_corners) const;

		/// @brief Transforms the frustum by the given matrix and returns a new frustum.
		/// @param p_frustum The frustum to transform.
		/// @param p_matrix The transformation matrix to apply to the frustum.
		/// @return A new frustum that is the result of transforming the input frustum by the given matrix.
		static Frustum Transform(const Frustum &p_frustum, const glm::mat4x4 &p_matrix);
	};
}
