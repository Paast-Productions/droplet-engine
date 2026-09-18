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

	/// @brief Abstract class for all mesh resources.
	class MeshBaseResource : public IResource
	{
		// TODO

	public:

	protected:
		std::vector<std::byte>								m_vertexData{};			// Vertex data
		std::vector<int>									m_indexData{};			// Index data

		std::size_t											m_vertexByteSize = 0;	// Byte size of a single vertex
		std::vector<std::pair<std::string, std::size_t>>	m_vertexLayout{};		// List of attribute names and their byte sizes

	private:

	};

	/// @brief Class for mesh resources.
	class MeshResource : public MeshBaseResource
	{
		// TODO

	public:

	private:

	};

	/// @brief Class for skinned mesh resources. 
	/// @details Vertex byte data is expected to contain bone indices and weights for each vertex. 
	/// Bone indices are expected to be stored as X ints, while bone weights are expected to be stored as X floats,
	/// where X is the maximum number of bones that can influence a single vertex (almost always 4). 
	/// The sum of all weights for a single vertex must equal 1.0f.
	class SkinnedMeshResource : public MeshBaseResource
	{
	public:
		struct Bone
		{
			std::string		name = "Unnamed";
			int				parentIndex = -1;	// -1: root
			glm::mat4		offsetMat{};		// Mesh-space to bone-space at bind pose
		};

		void AddBone(const std::string &p_name, int p_parentIndex, const glm::mat4 &p_offsetMat)
		{
			Bone bone {
				.name = p_name, 
				.parentIndex = p_parentIndex, 
				.offsetMat = p_offsetMat
			};

			m_bones.push_back(bone);
			m_boneMap[p_name] = static_cast<int>(m_bones.size()) - 1;
		}

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