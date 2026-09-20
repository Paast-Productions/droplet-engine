#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>


namespace Droplet
{
	enum class ResourceType
	{
		Texture2D,
		Texture3D,
		Mesh,
		SkinnedMesh,
		Animation,
		Shader,
		Material,
	};

	/// @brief Interface for all resources that can be loaded by the AssetManager. All resources must implement this interface.
	class IResource
	{
		// TODO

	public:

	protected:

	private:

	};

	/// @brief Abstract class for all texture resources.
	class TextureResource : public IResource
	{
		// TODO

	public:

	protected:

	private:

	};

	/// @brief Class for 2D texture resources.
	class Texture2DResource : public TextureResource
	{
		// TODO

	public:

	private:

	};

	/// @brief Class for 3D texture resources.
	class Texture3DResource : public TextureResource
	{
		// TODO

	public:

	private:

	};

	/// @brief Class for mesh resources.
	class MeshResource : public IResource
	{
	public:
		using VertexAttribute = std::pair<std::string, std::size_t>;

		/// @brief Sets the mesh data for the resource.
		void SetMeshData(const std::vector<std::byte> &p_vertexData, const std::vector<int> &p_indexData, 
						 std::size_t p_vertexByteSize, const std::vector<VertexAttribute> &p_vertexLayout)
		{
			m_vertexData = p_vertexData;
			m_indexData = p_indexData;
			m_vertexByteSize = p_vertexByteSize;
			m_vertexLayout = p_vertexLayout;
		}

		/// @brief Gets the mesh data for the resource.
		[[nodiscard]] const std::vector<std::byte>			&GetVertexData() const		{ return m_vertexData; }

		/// @brief Gets the mesh data for the resource.
		[[nodiscard]] const std::vector<int>				&GetIndexData() const		{ return m_indexData; }

		/// @brief Gets the byte size of a single vertex in the mesh data.
		[[nodiscard]] std::size_t							GetVertexByteSize() const	{ return m_vertexByteSize; }

		/// @brief Gets the vertex layout for the mesh data. The vertex layout is a list of attribute names and their byte sizes.
		[[nodiscard]] const std::vector<VertexAttribute>	&GetVertexLayout() const	{ return m_vertexLayout; }

	protected:
		std::vector<std::byte>			m_vertexData{};			// Vertex data
		std::vector<int>				m_indexData{};			// Index data

		std::size_t						m_vertexByteSize = 0;	// Byte size of a single vertex
		std::vector<VertexAttribute>	m_vertexLayout{};		// List of attribute names and their byte sizes

	private:

	};

	/// @brief Class for skinned mesh resources. 
	/// @details Vertex byte data is expected to contain bone indices and weights for each vertex. 
	/// Bone indices are expected to be stored as X ints, while bone weights are expected to be stored as X floats,
	/// where X is the maximum number of bones that can influence a single vertex (almost always 4). 
	/// The sum of all weights for a single vertex must equal 1.0f.
	class SkinnedMeshResource : public MeshResource
	{
	public:
		struct Bone
		{
			std::string		name = "Bone";
			int				parentIndex = -1;	// -1: root
			glm::mat4		offsetMat{};		// Mesh-space to bone-space at bind pose (default pose)
		};

		/// @brief Adds a bone to the skinned mesh resource.
		/// @param p_name The name of the bone.
		/// @param p_parentIndex The index of the parent bone. -1 if the bone is a root bone.
		/// @param p_offsetMat The offset matrix of the bone. This is the transformation from mesh space to bone space at bind pose.
		/// @return True if the bone was added successfully, false if a bone with the same name already exists.
		[[nodiscard]] bool AddBone(const std::string &p_name, int p_parentIndex, const glm::mat4 &p_offsetMat)
		{
			if (m_boneMap.find(p_name) != m_boneMap.end())
			{
				return false; // Bone with the same name already exists
			}

			Bone bone {
				.name = p_name, 
				.parentIndex = p_parentIndex, 
				.offsetMat = p_offsetMat
			};

			m_bones.push_back(bone);
			m_boneMap[p_name] = static_cast<int>(m_bones.size()) - 1;
		}

		/// @brief Validates that the vertex layout contains both bone indices and weights, and that they have the same size.
		/// @return True if the vertex layout is valid for a skinned mesh, otherwise false.
		[[nodiscard]] bool ValidateSkinnedVertexLayout() const
		{
			bool hasBoneIndices = false;
			bool hasBoneWeights = false;
			std::size_t size = -1;

			for (const auto &attribute : m_vertexLayout)
			{
				if (attribute.first == "bone_indices")
				{
					if (size == -1)
					{
						size = attribute.second;
					}
					else if (size != attribute.second)
					{
						return false; // Bone indices and weights must have the same size
					}

					hasBoneIndices = true;
				}
				else if (attribute.first == "bone_weights")
				{
					if (size == -1)
					{
						size = attribute.second;
					}
					else if (size != attribute.second)
					{
						return false; // Bone indices and weights must have the same size
					}

					hasBoneWeights = true;
				}
			}

			return hasBoneIndices && hasBoneWeights;
		}

		/// @brief Gets the bone data for the skinned mesh.
		/// @return A vector of bones for the skinned mesh.
		[[nodiscard]] const std::vector<Bone> &GetBones() const { return m_bones; }

		/// @brief Gets the bone map for the skinned mesh.
		/// @return A map of bone names to their indices in the bone vector.
		[[nodiscard]] const std::unordered_map<std::string, int> &GetBoneMap() const { return m_boneMap; }

	private:

		std::vector<Bone>						m_bones;
		std::unordered_map<std::string, int>	m_boneMap;
	};

	/// @brief Class for animation resources.
	class AnimationResource : public IResource
	{
		// TODO

	public:

	private:

	};

	/// @brief Class for shader resources.
	class ShaderResource : public IResource
	{
		// TODO

	public:

	private:

	};

	/// @brief Class for material resources.
	class MaterialResource : public IResource
	{
		// TODO

	public:

	private:

	};

}