#pragma once

#include <asset/ResourceManager.hpp>
#include "asset/Resource.hpp"

#include <vector>
#include <string>
#include <utility>
#include <assimp/Importer.hpp>

namespace Droplet
{
	/// @brief A resource loader using Assimp
	class AssimpLoader
	{
	public:
		AssimpLoader() = default;
		~AssimpLoader() = default;

		/// @brief Loads a mesh from an asset file.
		/// @param p_meshFile Path to the asset file storing the mesh.
		/// @param p_typeSpecificData Mesh specific import settings.
		/// @return A unique pointer with the stored mesh data.
		[[nodiscard]] std::unique_ptr<MeshResource> LoadMesh(const std::string &p_meshFile, const nlohmann::json &p_typeSpecificData);

		/// @brief Loads a skinned mesh from an asset file.
		/// @param p_meshFile Path to the asset file storing the skinned mesh.
		/// @param p_typeSpecificData Mesh specific import settings.
		/// @return A unique pointer with the stored skinned mesh data.
		[[nodiscard]] std::unique_ptr<SkinnedMeshResource> LoadSkinnedMesh(const std::string &p_meshFile, const nlohmann::json &p_typeSpecificData);

		/// @brief Loads an animation from an asset file.
		/// @param p_meshFile Path to the asset file storing the animation.
		/// @param p_animName The name given to the animation within the asset file.
		/// @param p_typeSpecificData Mesh specific import settings.
		/// @return A unique pointer with the stored animation data.
		[[nodiscard]] std::unique_ptr<AnimationResource> LoadAnimation(const std::string &p_meshFile, const std::string &p_animName,
			const nlohmann::json &p_typeSpecificData);

		/// @brief Lists all resources in a mesh
		/// @param p_meshFile File path of mesh.
		/// @return A vector of resource types and their names.
		[[nodiscard]] std::vector<std::pair<ResourceType, std::string>> ListAssetResources(const std::string &p_meshFile);

	private:
		Assimp::Importer m_importer{};

		/// @brief Helper function that constructs a vertex buffer.
		/// @param p_meshData The imported mesh object.
		/// @param p_vertexByteSize The vertex byte size calculated in CreateVertexLayout()
		/// @return A vector of the vertex buffer per byte.
		[[nodiscard]] std::vector<std::byte> BuildVertexData(const aiScene *&p_meshData, const std::size_t &p_vertexByteSize);

		/// @brief Helper function that constructs a index buffer.
		/// @param p_meshData The imported mesh object.
		/// @return A vector of the index buffer.
		[[nodiscard]] std::vector<std::uint32_t> BuildIndexData(const aiScene *&p_meshData);

		/// @brief Helper function that creates the vertex layout.
		/// @param[out] p_vertexByteSize The size of the created vertex layout.
		/// @return A vector of the vertex layout.
		[[nodiscard]] std::vector<MeshResource::VertexAttribute> CreateVertexLayout(std::size_t &p_vertexByteSize);

	};
}

