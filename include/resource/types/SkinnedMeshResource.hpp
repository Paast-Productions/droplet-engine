#pragma once
#include "resource/types/MeshResource.hpp"

#include <glm/glm.hpp>

#include <unordered_map>
#include <string>
#include <stdexcept>

namespace Droplet
{
    /// @brief Class for skinned mesh resources. 
	/// @details Vertex byte data is expected to contain bone indices and weights for each vertex. 
	/// Bone indices are expected to be stored as X ints, while bone weights are expected to be stored as X floats,
	/// where X is the maximum number of bones that can influence a single vertex (almost always 4). 
	/// The sum of all weights for a single vertex must equal 1.0f.
	class SkinnedMeshResource : public MeshResource
	{
	public:
		/// @brief Struct for bone data for the skinned mesh.
		struct Bone
		{
			std::string		name = "Bone";
			int				parentIndex = -1;	// -1: root
			glm::mat4		offsetMat{};		// Mesh-space to bone-space at bind pose (default pose)
			glm::vec3		bounds[2]{};		// AABB in bone space at bind pose
		};

		/// @brief Adds a bone to the skinned mesh resource.
		/// @param p_name The name of the bone.
		/// @param p_parentIndex The index of the parent bone. -1 if the bone is a root bone.
		/// @param p_offsetMat The offset matrix of the bone. This is the transformation from mesh space to bone space at bind pose.
		/// @return The index of the newly added bone.
		/// @throws std::runtime_error If a bone with the same name already exists.
		/// @throws std::out_of_range If the parent index is out of range.
		[[nodiscard]] int AddBone(const std::string &p_name, int p_parentIndex, const glm::mat4 &p_offsetMat);

		/// @brief Validates that the vertex layout contains both bone indices and weights, and that they have the same size.
		/// @return True if the vertex layout is valid for a skinned mesh, otherwise false.
		[[nodiscard]] bool ValidateSkinnedVertexLayout() const;

		/// @brief Gets the bone data for the skinned mesh.
		/// @return A vector of bones for the skinned mesh.
		[[nodiscard]] const std::vector<Bone> &GetBones() const;

		/// @brief Gets the bone map for the skinned mesh.
		/// @return A map of bone names to their indices in the bone vector.
		[[nodiscard]] const std::unordered_map<std::string, int> &GetBoneMap() const;

	private:
		std::vector<Bone>						m_bones;
		std::unordered_map<std::string, int>	m_boneMap;
	};
}
