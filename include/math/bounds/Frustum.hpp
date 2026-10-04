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

		/// @brief Gets the plane at the specified index.
		/// @param p_index The index of the plane to retrieve (0-5).
		/// @return The plane at the specified index.
		[[nodiscard]] inline const Plane &GetPlane(int p_index) const
		{
			return planes[p_index];
		}
	};
}
