#include "BvhLoader.hpp"

#include <bvh/v2/bvh.h>
#include <bvh/v2/node.h>
#include <bvh/v2/vec.h>
#include <bvh/v2/tri.h>
#include <bvh/v2/default_builder.h>
#include <bvh/v2/executor.h>
#include <bvh/v2/stack.h>

#include <vector>

using Scalar = float;
using Vec3 = bvh::v2::Vec<Scalar, 3>;
using BBox = bvh::v2::BBox<Scalar, 3>;
using Tri = bvh::v2::Tri<Scalar, 3>;
using Node = bvh::v2::Node<Scalar, 3>;
using Bvh = bvh::v2::Bvh<Node>;
using Ray = bvh::v2::Ray<Scalar, 3>;

using PrecomputedTri = bvh::v2::PrecomputedTri<Scalar>;

void Droplet::BvhLoader::GenerateMeshBVH(MeshResource &p_meshResource)
{
    const std::vector<std::uint32_t> &indexData = p_meshResource.GetIndexData();
    const std::vector<std::byte> &vertexData = p_meshResource.GetVertexData();
    const std::vector<MeshResource::VertexAttribute> &vertexLayout = p_meshResource.GetVertexLayout();
	std::size_t vertexByteSize = p_meshResource.GetVertexByteSize();

	// Ensure layout has POSITION attribute
    auto positionIt = std::find_if(vertexLayout.begin(), vertexLayout.end(),
		[](const MeshResource::VertexAttribute &attr) {
            return attr.first == "POSITION"; 
        });

    if (positionIt == vertexLayout.end())
    {
        throw std::runtime_error("MeshResource vertex layout does not contain POSITION attribute.");
	}

    std::size_t posDataOffset = 0;

	// Iterate through the vertex layout and add up the byte sizes of all attributes 
    // before POSITION to get the offset of the position data in the vertex data
    for (auto it = vertexLayout.begin(); it != positionIt; ++it)
    {
        posDataOffset += it->second;
	}

    std::vector<Tri> tris;
	tris.reserve(indexData.size() / 3);

	// Iterate through the index data and construct triangles from the vertex data
    for (size_t i = 0; i < indexData.size(); i += 3)
    {
        uint32_t idx0 = indexData[i];
        uint32_t idx1 = indexData[i + 1];
        uint32_t idx2 = indexData[i + 2];

        Vec3 v0, v1, v2;

		std::memcpy(&v0, vertexData.data() + idx0 * vertexByteSize + posDataOffset, sizeof(Vec3));
        std::memcpy(&v1, vertexData.data() + idx1 * vertexByteSize + posDataOffset, sizeof(Vec3));
        std::memcpy(&v2, vertexData.data() + idx2 * vertexByteSize + posDataOffset, sizeof(Vec3));

		tris.emplace_back(v0, v1, v2);
    }


    bvh::v2::SequentialExecutor executor;

    // Get triangle centers and bounding boxes (required for BVH builder)

    std::vector<BBox> bboxes(tris.size());
    std::vector<Vec3> centers(tris.size());

    executor.for_each(0, tris.size(), [&](size_t begin, size_t end) {
        for (size_t i = begin; i < end; ++i)
        {
            bboxes[i] = tris[i].get_bbox();
            centers[i] = tris[i].get_center();
        }
        });

    typename bvh::v2::DefaultBuilder<Node>::Config config;
    config.quality = bvh::v2::DefaultBuilder<Node>::Quality::Medium;
    auto bvh = bvh::v2::DefaultBuilder<Node>::build(bboxes, centers, config);

    // Permuting the primitive data allows to remove indirections during traversal, which makes it faster.
    static constexpr bool should_permute = true;

    // This precomputes some data to speed up traversal further.
    std::vector<PrecomputedTri> precomputedTris(tris.size());
    executor.for_each(0, tris.size(), [&](size_t begin, size_t end) {
        for (size_t i = begin; i < end; ++i)
        {
            auto j = should_permute ? bvh.prim_ids[i] : i;
            precomputedTris[i] = tris[j];
        }
    });

	auto meshBVH = std::make_shared<MeshResource::MeshBVH>();

    meshBVH->bvh = std::move(bvh);
    meshBVH->precomputedTris = std::move(precomputedTris);

	p_meshResource.SetBVH(std::move(meshBVH));
}
