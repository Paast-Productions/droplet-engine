#pragma once
#include <vector>
#include <memory>
#include <math/bounds/Intersection.hpp>
#include <glm/glm.hpp>
#include <bvh/v2/bvh.h>
#include <bvh/v2/node.h>
#include <bvh/v2/tri.h>

namespace Droplet::Math
{
	/// @brief Class for a mesh bounding volume hierarchy (BVH).
	class MeshBVH
	{
	public:
		/// @brief Default constructor for MeshBVH.
		MeshBVH() = default;
		/// @brief Constructs a MeshBVH from a list of triangles.
		/// @param p_triangleList A vector of glm::vec3 representing the triangle vertices.
		/// @throw std::invalid_argument If the size of p_triangleList is empty or not a multiple of 3.
		MeshBVH(std::vector<glm::vec3> p_triangleList);

		/// @brief Checks if the BVH has been generated.
		/// @return True if the BVH has been generated, false otherwise.
		[[nodiscard]] bool IsGenerated() const { return m_bvh != nullptr; }

		/// @brief Intersects a ray with the mesh BVH in model space.
		/// @note Function assumes that the ray is in model space. If the ray is in world space, it should be transformed to model space before calling this function.
		/// @param p_ray The ray to intersect with the mesh BVH.
		/// @param p_minDist The minimum distance for the intersection. Defaults to 0.0f.
		/// @param p_maxDist The maximum distance for the intersection. Defaults to -1.0f, which means no maximum distance.
		/// @return A RayHit struct containing information about the result.
		/// @throw std::runtime_error If the BVH is not initialized.
		/// @throw std::invalid_argument If the minimum distance is greater than or equal to the maximum distance.
		[[nodiscard]] RayHit Raycast(const Math::Ray &p_ray, float p_minDist = 0.0f, float p_maxDist = -1.0f) const;

	protected:
		std::shared_ptr<bvh::v2::Bvh<bvh::v2::Node<float, 3>>>	m_bvh{nullptr};
		std::vector<bvh::v2::PrecomputedTri<float>>				m_precomputedTris{};
	};
}
