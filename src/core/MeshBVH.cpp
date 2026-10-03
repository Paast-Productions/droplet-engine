#include "MeshBVH.hpp"

#include <bvh/v2/vec.h>
#include <bvh/v2/default_builder.h>
#include <bvh/v2/executor.h>
#include <bvh/v2/stack.h>
#include <limits>

using namespace Droplet;
using namespace bvh;

// Settings
constexpr bool      SHOULD_PERMUTE = true;                              // Permuting the primitive data allows to remove indirections during traversal, which makes it faster.
constexpr bool      USE_ROBUST_TRAVERSAL = false;                       // Robust traversal is slower, but more numerically stable.
constexpr size_t    INVALID_ID = std::numeric_limits<size_t>::max();    // Invalid primitive ID
constexpr size_t    STACK_SIZE = 64;                                    // Stack size for BVH traversal

// Type aliases for convenience
using Scalar = float;
using Vec3 = v2::Vec<Scalar, 3>;
using BBox = v2::BBox<Scalar, 3>;
using Tri = v2::Tri<Scalar, 3>;
using Node = v2::Node<Scalar, 3>;
using Bvh = v2::Bvh<Node>;
using Ray = v2::Ray<Scalar, 3>;
using PrecomputedTri = v2::PrecomputedTri<Scalar>;

MeshBVH::MeshBVH(std::vector<glm::vec3> p_triangleList)
{
	// Convert the triangle list to a vector of Tri objects
	std::vector<Tri> tris;
	tris.resize(p_triangleList.size() / 3);

	std::memcpy(tris.data(), p_triangleList.data(), p_triangleList.size() * sizeof(glm::vec3));

	// Get triangle centers and bounding boxes (required for BVH builder)
	std::vector<BBox> bboxes(tris.size());
	std::vector<Vec3> centers(tris.size());

	v2::SequentialExecutor executor;

	executor.for_each(0, tris.size(), [&](size_t begin, size_t end) {
		for (size_t i = begin; i < end; ++i)
		{
			bboxes[i] = tris[i].get_bbox();
			centers[i] = tris[i].get_center();
		}
	});

	v2::DefaultBuilder<Node>::Config config;
	config.quality = v2::DefaultBuilder<Node>::Quality::Medium;

    m_bvh = std::make_shared<Bvh>(v2::DefaultBuilder<Node>::build(bboxes, centers, config));

	// This precomputes some data to speed up traversal further.
    m_precomputedTris.resize(tris.size());

	executor.for_each(0, tris.size(), [&](size_t begin, size_t end) {
		for (size_t i = begin; i < end; ++i)
		{
			auto j = SHOULD_PERMUTE ? m_bvh->prim_ids[i] : i;
            m_precomputedTris[i] = tris[j];
		}
	});
}

MeshBVH::RayHit MeshBVH::Raycast(const glm::vec3 &p_rayOrigin, const glm::vec3 &p_rayDirection, float p_minDist, float p_maxDist) const
{
    if (!m_bvh)
    {
		throw std::runtime_error("BVH is not initialized.");
	}

    if (p_maxDist < 0.0f)
    {
        p_maxDist = std::numeric_limits<float>::max();
    }

    if (p_minDist >= p_maxDist)
    {
		throw std::invalid_argument("Minimum distance must be less than maximum distance.");
    }

    Ray ray{};
	ray.org = Vec3{ p_rayOrigin.x, p_rayOrigin.y, p_rayOrigin.z };
	ray.dir = Vec3{ p_rayDirection.x, p_rayDirection.y, p_rayDirection.z };
	ray.tmin = p_minDist;
	ray.tmax = p_maxDist;

    auto primID = INVALID_ID;
    Scalar u{}, v{};

    // Traverse the BVH and get the u, v coordinates of the closest intersection.
    v2::SmallStack<Bvh::Index, STACK_SIZE> stack;
    m_bvh->intersect<false, USE_ROBUST_TRAVERSAL>(ray, m_bvh->get_root().index, stack,
        [&](size_t begin, size_t end) {
            for (size_t i = begin; i < end; ++i) 
            {
                size_t j = SHOULD_PERMUTE ? i : m_bvh->prim_ids[i];
                if (auto hit = m_precomputedTris[j].intersect(ray))
                {
                    primID = i;
                    std::tie(ray.tmax, u, v) = *hit;
                }
            }
            return primID != INVALID_ID;
        });

    RayHit hit{};

    if (primID != INVALID_ID)
    {
        hit.didHit = true;
        hit.distance = ray.tmax;
        hit.hitPoint = p_rayOrigin + p_rayDirection * ray.tmax;

        size_t j = SHOULD_PERMUTE ? primID : m_bvh->prim_ids[primID];
        const auto &tri = m_precomputedTris[j];

        // Compute the normal of the triangle
        glm::vec3 edge1{ tri.e1.values[0], tri.e1.values[1], tri.e1.values[2] };
        glm::vec3 edge2{ tri.e2.values[0], tri.e2.values[1], tri.e2.values[2] };

        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

		// Ensure the normal is facing the ray
        if (glm::dot(normal, p_rayDirection) > 0.0f)
        {
            normal = -normal;
		}

        hit.hitNormal = normal;
	}

	return hit;
}
