#include "resource/types/MeshResource.hpp"

std::unique_ptr<Droplet::MeshResource> Droplet::MeshResource::CreateFallback()
{
    auto fallback = std::make_unique<MeshResource>();
    
    struct Vertex
    {
        glm::vec3 pos;
    };
    
    // 8 vertices of a 1x1x1 unit cube
    float rawPos[8][3] = {
        {-0.5f, -0.5f,  0.5f}, { 0.5f, -0.5f,  0.5f},
        { 0.5f,  0.5f,  0.5f}, {-0.5f,  0.5f,  0.5f},
        {-0.5f, -0.5f, -0.5f}, { 0.5f, -0.5f, -0.5f},
        { 0.5f,  0.5f, -0.5f}, {-0.5f,  0.5f, -0.5f}
    };
    
    Vertex vertices[8];
    for (int i = 0; i < 8; i++)
    {
        vertices[i].pos = glm::vec3(rawPos[i][0], rawPos[i][1], rawPos[i][2]);
    }
    
    // 36 indices (standard cube)
    std::vector<std::uint32_t> indices = {
        0, 1, 2, 2, 3, 0, // Front
        1, 5, 6, 6, 2, 1, // Right
        7, 6, 5, 5, 4, 7, // Back
        4, 0, 3, 3, 7, 4, // Left
        4, 5, 1, 1, 0, 4, // Bottom
        3, 2, 6, 6, 7, 3  // Top
    };
    
    // Copy data into byte vector
    std::size_t vertexByteSize = sizeof(Vertex);
    std::vector<std::byte> vertexData(8 * vertexByteSize);
    std::memcpy(vertexData.data(), vertices, vertexData.size());
    
    std::vector<MeshResource::VertexAttribute> layout = {
        {"POSITION", sizeof(float) * 3}
    };
    
    fallback->SetMeshData(vertexData, indices, vertexByteSize, layout);
    
    // TODO: Should bvh be generated here?
    
    return fallback;
}

bool Droplet::MeshResource::FindVertexAttribute(const std::string &p_attribute, std::size_t &p_offset, std::size_t &p_size) const
{
    // Ensure layout has attribute
    auto boneIndexIt = std::find_if(m_vertexLayout.begin(), m_vertexLayout.end(),
        [&](const MeshResource::VertexAttribute &attr) {
            return attr.first == p_attribute;
        });

    if (boneIndexIt == m_vertexLayout.end())
    {
		return false; // Attribute not found
    }

	p_size = boneIndexIt->second; // Set the size of the attribute
    p_offset = 0;
    
    // Iterate through the vertex layout and add up the byte sizes of all attributes 
    // before the attribute to get the attributes offset in the vertex data
    for (auto it = m_vertexLayout.begin(); it != boneIndexIt; ++it)
    {
        p_offset += it->second;
    }

	return true;
}
