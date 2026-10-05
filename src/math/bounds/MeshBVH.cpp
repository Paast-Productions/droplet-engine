#include "math/bounds/MeshBVH.hpp"

#include <bvh/v2/vec.h>
#include <bvh/v2/default_builder.h>
#include <bvh/v2/executor.h>
#include <bvh/v2/stack.h>
#include <limits>

using namespace Droplet::Math;
using namespace bvh;

// Settings
constexpr bool			SHOULD_PERMUTE = true;									// Permuting the primitive data allows to remove indirections during traversal, which makes it faster.
constexpr bool          USE_ROBUST_TRAVERSAL = false;							// Robust traversal is slower, but more numerically stable.
constexpr std::size_t	INVALID_ID = std::numeric_limits<std::size_t>::max();	// Invalid primitive ID
constexpr std::size_t   STACK_SIZE = 64;										// Stack size for BVH traversal

// Type aliases
using Node = bvh::v2::Node<float, 3>;

MeshBVH::MeshBVH(std::vector<glm::vec3> p_triangleList)
{
	if (p_triangleList.empty())
	{
		throw std::invalid_argument("Triangle list is empty.");
	}

	if (p_triangleList.size() % 3 != 0)
	{
		throw std::invalid_argument("Triangle list size must be a multiple of 3.");
	}

	// Convert the triangle list to a vector of Tri objects
	std::vector<v2::Tri<float, 3>> tris;
	tris.resize(p_triangleList.size() / 3);

	std::memcpy(tris.data(), p_triangleList.data(), p_triangleList.size() * sizeof(glm::vec3));

	// Get triangle centers and bounding boxes (required for BVH builder)
	std::vector<v2::BBox<float, 3>> bboxes(tris.size());
	std::vector<v2::Vec<float, 3>> centers(tris.size());

	v2::SequentialExecutor executor;

	executor.for_each(0, tris.size(), [&](std::size_t begin, std::size_t end) {
		for (std::size_t i = begin; i < end; ++i)
		{
			bboxes[i] = tris[i].get_bbox();
			centers[i] = tris[i].get_center();
		}
	});

	v2::DefaultBuilder<Node>::Config config;
	config.quality = v2::DefaultBuilder<Node>::Quality::Medium;

	m_bvh = std::make_shared<v2::Bvh<Node>>(v2::DefaultBuilder<Node>::build(bboxes, centers, config));

	// This precomputes some data to speed up traversal further.
	m_precomputedTris.resize(tris.size());

	executor.for_each(0, tris.size(), [&](std::size_t begin, std::size_t end) {
		for (std::size_t i = begin; i < end; ++i)
		{
			auto j = SHOULD_PERMUTE ? m_bvh->prim_ids[i] : i;
			m_precomputedTris[i] = tris[j];
		}
	});
}

RayHit MeshBVH::Raycast(const Ray &p_ray, float p_minDist, float p_maxDist) const
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

	v2::Ray<float, 3> ray{};
	ray.org = { p_ray.pos.x, p_ray.pos.y, p_ray.pos.z };
	ray.dir = { p_ray.dir.x, p_ray.dir.y, p_ray.dir.z };
	ray.tmin = p_minDist;
	ray.tmax = p_maxDist;

	std::size_t primID = INVALID_ID;
	float u{}, v{};

	// Traverse the BVH and get the u, v coordinates of the closest intersection.
	v2::SmallStack<v2::Bvh<Node>::Index, STACK_SIZE> stack;
	m_bvh->intersect<false, USE_ROBUST_TRAVERSAL>(ray, m_bvh->get_root().index, stack,
		[&](std::size_t begin, std::size_t end) {
			for (std::size_t i = begin; i < end; ++i)
			{
				std::size_t j = SHOULD_PERMUTE ? i : m_bvh->prim_ids[i];
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
		hit.point = p_ray.pos + p_ray.dir * ray.tmax;

		std::size_t j = SHOULD_PERMUTE ? primID : m_bvh->prim_ids[primID];
		const v2::PrecomputedTri<float> &tri = m_precomputedTris[j];

		// Compute the normal of the triangle
		glm::vec3 edge1{ tri.e1.values[0], tri.e1.values[1], tri.e1.values[2] };
		glm::vec3 edge2{ tri.e2.values[0], tri.e2.values[1], tri.e2.values[2] };

		glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

		// Ensure the normal is facing the ray
		if (glm::dot(normal, p_ray.dir) > 0.0f)
		{
			normal = -normal;
		}

		hit.normal = normal;
	}

	return hit;
}
