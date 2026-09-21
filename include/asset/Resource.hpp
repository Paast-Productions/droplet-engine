#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <stdexcept>


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
	public:
		enum class TextureFormat
		{
			Unknown,

			// Color formats
			R8_Unorm,
			R8_Snorm,
			R8_Uscaled,
			R8_Sscaled,
			R8_Uint,
			R8_Sint,

			R16_Unorm,
			R16_Snorm,
			R16_Uscaled,
			R16_Sscaled,
			R16_Uint,
			R16_Sint,

			R32_Unorm,
			R32_Snorm,
			R32_Uscaled,
			R32_Sscaled,
			R32_Uint,
			R32_Sint,

			RGB8_Unorm,
			RGB8_Snorm,
			RGB8_Uscaled,
			RGB8_Sscaled,
			RGB8_Uint,
			RGB8_Sint,

			RGB16_Unorm,
			RGB16_Snorm,
			RGB16_Uscaled,
			RGB16_Sscaled,
			RGB16_Uint,
			RGB16_Sint,

			RGB32_Unorm,
			RGB32_Snorm,
			RGB32_Uscaled,
			RGB32_Sscaled,
			RGB32_Uint,
			RGB32_Sint,

			RGBA8_Unorm,
			RGBA8_Snorm,
			RGBA8_Uscaled,
			RGBA8_Sscaled,
			RGBA8_Uint,
			RGBA8_Sint,
			   
			RGBA16_Unorm,
			RGBA16_Snorm,
			RGBA16_Uscaled,
			RGBA16_Sscaled,
			RGBA16_Uint,
			RGBA16_Sint,
			   
			RGBA32_Unorm,
			RGBA32_Snorm,
			RGBA32_Uscaled,
			RGBA32_Sscaled,
			RGBA32_Uint,
			RGBA32_Sint,

			// Depth formats
			D24S8,
			D32,

			// Compressed formats
			BC1,
			BC2,
			BC3,
			BC4,
			BC5,
			BC6H,
			BC7,

			// Add more formats as needed...
		};

		/// @brief Sets the format of the texture data.
		/// @param p_format The format of the texture data.
		/// @param p_bytesPerPixel The number of bytes per pixel in the texture data.
		void SetFormat(TextureFormat p_format, int p_bytesPerPixel) { m_format = p_format; m_bytesPerPixel = p_bytesPerPixel; }

		/// @brief Sets the number of mipmap levels in the texture.
		/// @param p_mipLevels The number of mipmap levels in the texture.
		void SetMipLevels(int p_mipLevels) { m_mipLevels = p_mipLevels; }

		/// @brief Sets the pixel data for the texture.
		/// @param p_pixelData The pixel data for the texture.
		void SetPixelData(const std::vector<std::byte> &p_pixelData) { m_pixelData = p_pixelData; }

		/// @brief Gets the format of the texture data.
		/// @return The format of the texture data.
		[[nodiscard]] TextureFormat GetFormat() const { return m_format; }

		/// @brief Gets the number of bytes per pixel in the texture data.
		/// @return The number of bytes per pixel in the texture data.
		[[nodiscard]] int GetBytesPerPixel() const { return m_bytesPerPixel; }

		/// @brief Gets the number of mipmap levels in the texture.
		/// @return The number of mipmap levels in the texture.
		[[nodiscard]] int GetMipLevels() const { return m_mipLevels; }

		/// @brief Gets the pixel data for the texture.
		/// @return A vector of bytes representing the pixel data for the texture.
		[[nodiscard]] const std::vector<std::byte> &GetPixelData() const { return m_pixelData; }

	private:
		TextureFormat	m_format = TextureFormat::Unknown;	// The format of the texture data
		int				m_mipLevels = 1;					// The number of mipmap levels in the texture
		int				m_bytesPerPixel = 0;				// The number of bytes per pixel in the texture data (invalid for compressed formats)

		std::vector<std::byte> m_pixelData{};				// Pixel data for the texture
	};

	/// @brief Class for 2D texture resources.
	class Texture2DResource : public TextureResource
	{
	public:
		/// @brief Sets the width and height of the texture.
		/// @param p_width The width of the texture.
		/// @param p_height The height of the texture.
		void SetDimensions(std::uint32_t p_width, std::uint32_t p_height)
		{
			m_width = p_width;
			m_height = p_height;
		}

		/// @brief Gets the width of the texture.
		/// @return The width of the texture.
		[[nodiscard]] std::uint32_t GetWidth() const { return m_width; }

		/// @brief Gets the height of the texture.
		/// @return The height of the texture.
		[[nodiscard]] std::uint32_t GetHeight() const { return m_height; }

	private:
		std::uint32_t m_width = 0;		// The width of the texture
		std::uint32_t m_height = 0;		// The height of the texture
	};

	/// @brief Class for 3D texture resources.
	class Texture3DResource : public TextureResource
	{
	public:
		/// @brief Sets the width, height, and depth of the texture.
		/// @param p_width The width of the texture.
		/// @param p_height The height of the texture.
		/// @param p_depth The depth of the texture.
		void SetDimensions(std::uint32_t p_width, std::uint32_t p_height, std::uint32_t p_depth)
		{
			m_width = p_width;
			m_height = p_height;
			m_depth = p_depth;
		}

		/// @brief Gets the width of the texture.
		/// @return The width of the texture.
		[[nodiscard]] std::uint32_t GetWidth() const { return m_width; }

		/// @brief Gets the height of the texture.
		/// @return The height of the texture.
		[[nodiscard]] std::uint32_t GetHeight() const { return m_height; }

		/// @brief Gets the depth of the texture.
		/// @return The depth of the texture.
		[[nodiscard]] std::uint32_t GetDepth() const { return m_depth; }

	private:
		std::uint32_t m_width = 0;		// The width of the texture
		std::uint32_t m_height = 0;		// The height of the texture
		std::uint32_t m_depth = 0;		// The depth of the texture
	};

	/// @brief Class for mesh resources.
	class MeshResource : public IResource
	{
	public:
		using VertexAttribute = std::pair<std::string, std::size_t>;

		/// @brief Sets the mesh data for the resource.
		/// @param p_vertexData The vertex data for the mesh.
		/// @param p_indexData The index data for the mesh.
		/// @param p_vertexByteSize The byte size of a single vertex.
		/// @param p_vertexLayout The vertex layout for the mesh data.
		void SetMeshData(const std::vector<std::byte> &p_vertexData, const std::vector<int> &p_indexData, 
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
		[[nodiscard]] const std::vector<int>				&GetIndexData() const		{ return m_indexData; }

		/// @brief Gets the byte size of a single vertex in the mesh data.
		/// @return The byte size of a single vertex in the mesh data.
		[[nodiscard]] std::size_t							GetVertexByteSize() const	{ return m_vertexByteSize; }

		/// @brief Gets the vertex layout for the mesh data.
		/// @return A vector of pairs representing the vertex layout for the mesh data. Each pair contains the attribute name and its byte size.
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
		/// @return The index of the newly added bone.
		/// @throws std::runtime_error If a bone with the same name already exists.
		/// @throws std::out_of_range If the parent index is out of range.
		[[nodiscard]] int AddBone(const std::string &p_name, int p_parentIndex, const glm::mat4 &p_offsetMat)
		{
			if (m_boneMap.find(p_name) != m_boneMap.end())
			{
				throw std::runtime_error("Bone with the same name already exists: " + p_name);
			}

			if (p_parentIndex < -1 || p_parentIndex >= static_cast<int>(m_bones.size()))
			{
				throw std::out_of_range("Parent index is out of range: " + std::to_string(p_parentIndex));
			}

			Bone bone {
				.name = p_name, 
				.parentIndex = p_parentIndex, 
				.offsetMat = p_offsetMat
			};

			m_bones.push_back(bone);
			m_boneMap[p_name] = static_cast<int>(m_bones.size()) - 1;

			return m_boneMap[p_name];
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
		enum class ShaderType
		{
			Vertex,
			Fragment,
			Geometry,
			Compute,
			TessellationControl,
			TessellationEvaluation,
			Mesh,
			Task,
		};

	private:

		ShaderType m_shaderType = ShaderType::Vertex;
	};

	/// @brief Class for material resources.
	class MaterialResource : public IResource
	{
		// TODO

	public:

	private:

	};

}