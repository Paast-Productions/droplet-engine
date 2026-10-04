#include "resource/types/MeshResource.hpp"

bool Droplet::MeshResource::FindVertexAttribute(const std::string &p_attribute, std::size_t p_offset, std::size_t p_size) const
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

    // Iterate through the vertex layout and add up the byte sizes of all attributes 
    // before the attribute to get the attributes offset in the vertex data
    for (auto it = m_vertexLayout.begin(); it != boneIndexIt; ++it)
    {
        p_offset += it->second;
    }

	return true;
}
