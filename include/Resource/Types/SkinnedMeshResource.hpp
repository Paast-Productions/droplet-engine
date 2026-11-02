#pragma once
#include "resource/types/MeshResource.hpp"

#include <math/bounds/OBB.hpp>
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
	/// @par Coordinate-space convention
	/// "Mesh space" is the space of the stored vertex positions. Bone offset matrices map mesh space to bone space at bind pose,
	/// so a bone's bind-pose global transform in mesh space is inverse(offsetMat). transformMat is the bind transform relative
	/// to the parent bone (global transform for root bones). Bone bounds are OBBs in bone space (offsetMat * vertexPosition).
	/// All matrices are column-major GLM matrices acting on column vectors (parentGlobal * local = global).
	class SkinnedMeshResource : public MeshResource
	{
	public:
		/// @brief Struct for bone data for the skinned mesh.
		struct Bone
		{
			std::string		name = "Bone";
			int				parentIndex = -1;	// -1: root
			glm::mat4		transformMat{};		// Bone-space to parent-space at bind pose (default pose)
			glm::mat4		offsetMat{};		// Mesh-space to bone-space at bind pose (default pose)
			Math::OBB		bounds{};			// OBB in bone space at bind pose
		};
	    
	    /// @brief Creates a fallback instance of a skinned mesh resource (unit cube with one bone).
	    /// @return The skinned mesh resource.
	    static std::unique_ptr<SkinnedMeshResource> CreateFallback();

		/// @brief Adds a bone to the skinned mesh resource.
		/// @param p_name The name of the bone.
		/// @param p_parentIndex The index of the parent bone. -1 if the bone is a root bone.
		/// @param p_transformMat The transformation matrix of the bone. This is the bone's transformation relative to its parent bone at bind pose (default pose).
		/// @param p_offsetMat The offset matrix of the bone. This is the bone's transformation from mesh space to bone space at bind pose (default pose).
		/// @return The index of the newly added bone.
		/// @throws std::runtime_error If a bone with the same name already exists.
		/// @throws std::out_of_range If the parent index is out of range.
		[[nodiscard]] int AddBone(const std::string &p_name, int p_parentIndex = -1, const glm::mat4 &p_transformMat = glm::mat4(1.0f), const glm::mat4 &p_offsetMat = glm::mat4(1.0f));

		/// @brief Sets the bone data for the skinned mesh resource.
		/// @param p_index The index of the bone to set.
		/// @param p_parentIndex The index of the parent bone. -1 if the bone is a root bone.
		/// @param p_transformMat The transformation matrix of the bone. This is the bone's transformation relative to its parent bone at bind pose (default pose).
		/// @param p_offsetMat The offset matrix of the bone. This is the bone's transformation from mesh space to bone space at bind pose (default pose).
		void UpdateBone(int p_index, int p_parentIndex, const glm::mat4 &p_transformMat, const glm::mat4 &p_offsetMat);

		/// @brief Validates that the vertex layout contains both bone indices and weights, and that they have the same size.
		/// @return True if the vertex layout is valid for a skinned mesh, otherwise false.
		[[nodiscard]] bool ValidateSkinnedVertexLayout() const;

		/// @brief Gets the bone data for the skinned mesh.
		/// @return A vector of bones for the skinned mesh.
		[[nodiscard]] const std::vector<Bone> &GetBones() const;

		/// @brief Gets the bone map for the skinned mesh.
		/// @return A map of bone names to their indices in the bone vector.
		[[nodiscard]] const std::unordered_map<std::string, int> &GetBoneMap() const;

		/// @brief Generates the bounding volumes for each bone in the skinned mesh.
		/// @throw std::runtime_error If vertex layout is incorrect.
		void GenerateBoneBounds();

	private:
		std::vector<Bone>						m_bones;
		std::unordered_map<std::string, int>	m_boneMap;
	};
}
