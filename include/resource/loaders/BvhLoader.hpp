#pragma once

#include <resource/types/MeshResource.hpp>

namespace Droplet::BvhLoader
{
	/// @brief Generates a bounding volume hierarchy (BVH) for a mesh resource.
	/// @note This function assumes that the mesh resource has already been loaded and contains valid vertex and index data.
	/// @param p_meshResource The mesh resource for which to generate the BVH.
	/// @throw std::runtime_error If the mesh resource does not contain valid vertex or index data, or if the BVH generation fails.
	void GenerateMeshBVH(MeshResource &p_meshResource);
}