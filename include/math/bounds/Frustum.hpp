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
		[[nodiscard]] inline const Plane &GetPlane(int p_index) const
		{
			return planes[p_index];
		}

		/// @brief Gets the left plane of the frustum.
		/// @return The left plane of the frustum.
		[[nodiscard]] inline const Plane &GetLeft() const
		{
			return planes[0];
		}

		/// @brief Gets the right plane of the frustum.
		/// @return The right plane of the frustum.
		[[nodiscard]] inline const Plane &GetRight() const
		{
			return planes[1];
		}

		/// @brief Gets the top plane of the frustum.
		/// @return The top plane of the frustum.
		[[nodiscard]] inline const Plane &GetTop() const
		{
			return planes[2];
		}

		/// @brief Gets the bottom plane of the frustum.
		/// @return The bottom plane of the frustum.
		[[nodiscard]] inline const Plane &GetBottom() const
		{
			return planes[3];
		}

		/// @brief Gets the near plane of the frustum.
		/// @return The near plane of the frustum.
		[[nodiscard]] inline const Plane &GetNear() const
		{
			return planes[4];
		}

		/// @brief Gets the far plane of the frustum.
		/// @return The far plane of the frustum.
		[[nodiscard]] inline const Plane &GetFar() const
		{
			return planes[5];
		}
	};
}
