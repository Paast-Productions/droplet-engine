#include "BvhLoader.hpp"
#include <resource/types/MeshResource.hpp>
#include <vector>

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

    std::vector<glm::vec3> tris;
    tris.reserve(indexData.size());

	// Iterate through the index data and extract the position data for each vertex
    for (size_t i = 0; i < indexData.size(); ++i)
    {
        std::uint32_t index = indexData[i];
        std::size_t vertexOffset = index * vertexByteSize + posDataOffset;

		if (vertexOffset + sizeof(glm::vec3) > vertexData.size()) // TODO: Is this necessary?
        {
            throw std::runtime_error("Index out of bounds when accessing vertex data.");
        }

        glm::vec3 position;
        std::memcpy(&position, &vertexData[vertexOffset], sizeof(glm::vec3));

		tris.push_back(position);
    }

	p_meshResource.m_bvh = MeshBVH(tris);
}
