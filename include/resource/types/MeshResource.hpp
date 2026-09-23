#pragma once
#include "IResource.hpp"

#include <string>
#include <vector>

namespace Droplet
{
    /// @brief Class for mesh resources.
	class MeshResource : public IResource
	{
	public:
		using VertexAttribute = std::pair<std::string, std::size_t>;

		/// @brief Struct for bounding volume hierarchy (BVH) data for the mesh.
		struct MeshBVH
		{
			// TODO
		};

		/// @brief Sets the mesh data for the resource.
		/// @param p_vertexData The vertex data for the mesh.
		/// @param p_indexData The index data for the mesh.
		/// @param p_vertexByteSize The byte size of a single vertex.
		/// @param p_vertexLayout The vertex layout for the mesh data.
		void SetMeshData(const std::vector<std::byte> &p_vertexData, const std::vector<std::uint32_t> &p_indexData,
						 std::size_t p_vertexByteSize, const std::vector<VertexAttribute> &p_vertexLayout)
		{
			m_vertexData = p_vertexData;
			m_indexData = p_indexData;
			m_vertexByteSize = p_vertexByteSize;
			m_vertexLayout = p_vertexLayout;
		}

		/// @brief Gets the mesh data for the resource.
		/// @return A vector of bytes representing the vertex data for the mesh.
		[[nodiscard]] const std::vector<std::byte>			&GetVertexData() const		{ return m_vertexData; }

		/// @brief Gets the mesh data for the resource.
		/// @return A vector of ints representing the index data for the mesh.
		[[nodiscard]] const std::vector<std::uint32_t>		&GetIndexData() const		{ return m_indexData; }

		/// @brief Gets the byte size of a single vertex in the mesh data.
		/// @return The byte size of a single vertex in the mesh data.
		[[nodiscard]] std::size_t							GetVertexByteSize() const	{ return m_vertexByteSize; }

		/// @brief Gets the vertex layout for the mesh data.
		/// @return A vector of pairs representing the vertex layout for the mesh data. Each pair contains the attribute name and its byte size.
		[[nodiscard]] const std::vector<VertexAttribute>	&GetVertexLayout() const	{ return m_vertexLayout; }

	protected:
		std::vector<std::byte>			m_vertexData{};			// Vertex data
		std::vector<std::uint32_t>		m_indexData{};			// Index data

		std::size_t						m_vertexByteSize = 0;	// Byte size of a single vertex
		std::vector<VertexAttribute>	m_vertexLayout{};		// List of attribute names and their byte sizes

		MeshBVH							m_bvh{};				// Bounding volume hierarchy for the mesh

	private:

	};
}