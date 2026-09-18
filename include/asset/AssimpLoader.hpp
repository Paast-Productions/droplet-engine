#pragma once

#include <asset/AssetManager.hpp>

#include <string>
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

	private:
		Assimp::Importer m_importer;

	};
}

