#pragma once

#include <asset/AssetManager.hpp>
#include "asset/Resource.hpp"

#include <vector>
#include <string>
#include <utility>
#include <assimp/Importer.hpp>

using json = nlohmann::json;

namespace Droplet
{
	/// @brief A resource loader using Assimp
	class AssimpLoader
	{
	public:
		AssimpLoader() = default;
		~AssimpLoader() = default;

		/// @brief Loads a mesh file.
		/// @param p_meshFile File path of mesh.
		/// @param p_typeSpecificData Mesh specific import settings.
		/// @param p_assetRecord Contains relevant information about a specific loaded asset.
		/// @return A boolean. True if the mesh successfully loaded, else false.
		bool LoadMesh(std::string p_meshFile, const json &p_typeSpecificData, AssetRecord &p_assetRecord);

		/// @brief Lists all resources in a mesh
		/// @param p_meshFile File path of mesh.
		/// @return A vector of resource types and their names.
		std::vector<std::pair<ResourceType, std::string>> ListAssetResources(const std::string p_meshFile);

	private:
		Assimp::Importer m_importer;

		/// @brief Helper function that constructs a vertex buffer.
		/// @param p_meshData The imported mesh object.
		/// @param p_vertexByteSize The vertex byte size calculated in CreateVertexLayout()
		/// @return A vector of the vertex buffer per byte.
		std::vector<std::byte> BuildVertexData(const aiScene *&p_meshData, const std::size_t p_vertexByteSize);

		/// @brief Helper function that constructs a index buffer.
		/// @param p_meshData The imported mesh object.
		/// @return A vector of the index buffer.
		std::vector<std::uint32_t> BuildIndexData(const aiScene *&p_meshData);

		/// @brief Helper function that creates the vertex layout.
		/// @param[out] p_vertexByteSize The size of the created vertex layout.
		/// @return A vector of the vertex layout.
		std::vector<MeshResource::VertexAttribute> CreateVertexLayout(std::size_t &p_vertexByteSize);

	};
}

