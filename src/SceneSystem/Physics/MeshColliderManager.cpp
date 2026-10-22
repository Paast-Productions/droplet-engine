#include <MeshColliderManager.h>

#include <resource/Types/MeshResource.hpp>
#include <resource/ResourceManager.hpp>

using namespace Droplet;

JPH::MeshShapeSettings MeshColliderManager::GetMeshSettings(const GUID p_guid)
{
    if (m_shapeSettings.contains(p_guid))
    {
        return m_shapeSettings[p_guid];
    }
    
    ResourceHandle<MeshResource> meshHandle = ResourceManager::Get().LoadResource<MeshResource>(p_guid);
    
    JPH::MeshShapeSettings meshSettings = CreateMeshShapeSettings(meshHandle.Get());
    m_shapeSettings[p_guid] = meshSettings;
    return meshSettings;
}

JPH::MeshShapeSettings MeshColliderManager::CreateMeshShapeSettings(const MeshResource *p_mesh)
{
    // Make a Jolt array for vertices
    const std::size_t C_BYTES_PER_VERTEX = p_mesh->GetVertexByteSize();
    const std::size_t C_VERTEX_COUNT = p_mesh->GetVertexData().size() / C_BYTES_PER_VERTEX;
	
    JPH::VertexList joltVertices{};
    joltVertices.reserve(C_VERTEX_COUNT);
	
    std::vector<std::byte> vertexData = p_mesh->GetVertexData();
    for (std::size_t i = 0; i < C_VERTEX_COUNT; i++)
    {
        JPH::Float3 pos;
        std::memcpy(&pos, vertexData.data() + i * C_BYTES_PER_VERTEX, sizeof(JPH::Float3));
		
        joltVertices.emplace_back(pos);
    }

    // Make a Jolt array for triangles based of indices
    JPH::IndexedTriangleList triangles;
    std::vector<std::uint32_t> indexData = p_mesh->GetIndexData();
    std::size_t nrOfIndices = indexData.size();
    for (std::size_t i = 0; i < nrOfIndices; i += 3)
    {
        triangles.emplace_back(
            indexData[i],
            indexData[i + 1],
            indexData[i + 2]
        );
    }

    return JPH::MeshShapeSettings(std::move(joltVertices), std::move(triangles));
}