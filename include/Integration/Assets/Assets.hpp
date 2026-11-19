#pragma once
#include <glm/glm.hpp>

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Integration/Core/Guid.hpp"
#include "Integration/Common/RenderDesc.hpp"

/// CPU-side asset data, i.e. what the ResourceManager would normally store. Scene side only: the renderer never
/// includes this, it gets asset data from the Bridge (Assets/AssetTransfer.hpp)
namespace Assets
{
	/// Stored Mesh, loaded from file or created in memory (always in memory for sandbox)
	struct MeshAsset
	{
		std::vector<std::byte> vertexData; ///< vertices back to back, vertexStride bytes each
		std::uint32_t vertexStride{ 0 };   ///< bytes per vertex; the layout is the renderer's (Common::MeshVertex today)
		std::vector<std::uint32_t> indices;

		/// @brief Number of whole vertices in vertexData
		[[nodiscard]] std::size_t VertexCount() const noexcept { return vertexStride ? vertexData.size() / vertexStride : 0; }

		// use sphere instead of aabb to keep the data light
		glm::vec3 boundsCenter{ 0.0f };
		float boundsRadius = 0.0f;
	};

	/// Stored image texture, loaded from file or created in memory
	struct TextureAsset
	{
		std::uint32_t width = 0;
		std::uint32_t height = 0;
		Common::PixelFormat format = Common::PixelFormat::RGBA8_SRGB;
		std::vector<std::byte> pixels;	// tightly packed RGBA8, row 0 = top
		Common::TextureSampling sampling{};
	};

	/// Stored scene shader: the description is all there is to it (the renderer compiles the source)
	using ShaderAsset = Common::ShaderDesc;

	/// Simpler version of ResourceManager for testing, maps GUID to CPU data
	/// @details Every asset has a revision: 1 when it is first added, one more every time Add overwrites it. The
	///          renderer compares revisions to know that an asset it holds has changed. Revision 0 means the storage
	///          does not have it.
	class AssetStorage
	{
	public:
		/// Inserts or overwrites existing mesh asset
		/// @param id generated guid for the mesh asset
		/// @param mesh the mesh being stored
		void Add(Guid id, MeshAsset mesh) { Store(m_meshes, id, std::move(mesh)); }
		/// Inserts or overwrites existing texture asset
		/// @param id generated guid for the texture asset
		/// @param texture the texture to store
		void Add(Guid id, TextureAsset texture) { Store(m_textures, id, std::move(texture)); }
		/// Inserts or overwrites a shader asset
		void Add(Guid id, ShaderAsset shader) { Store(m_shaders, id, std::move(shader)); }

		/// The mesh stored under `id`, or nullptr
		const MeshAsset* FindMesh(Guid id) const { return Find(m_meshes, id); }
		/// The texture stored under `id`, or nullptr
		const TextureAsset* FindTexture(Guid id) const { return Find(m_textures, id); }
		/// The shader stored under `id`, or nullptr
		const ShaderAsset* FindShader(Guid id) const { return Find(m_shaders, id); }

		/// Revision of the mesh under `id`; 0 if there is none
		std::uint64_t MeshRevision(Guid id) const { return Revision(m_meshes, id); }
		/// Revision of the texture under `id`; 0 if there is none
		std::uint64_t TextureRevision(Guid id) const { return Revision(m_textures, id); }
		/// Revision of the shader under `id`; 0 if there is none
		std::uint64_t ShaderRevision(Guid id) const { return Revision(m_shaders, id); }

	private:
		/// An asset and how often it has been stored under its GUID
		template <class T>
		struct Entry
		{
			T asset;
			std::uint64_t revision{ 0 };
		};

		template <class T>
		static void Store(std::unordered_map<Guid, Entry<T>>& entries, Guid id, T asset)
		{
			Entry<T>& entry = entries[id];
			entry.asset = std::move(asset);
			++entry.revision;
		}

		template <class T>
		static const T* Find(const std::unordered_map<Guid, Entry<T>>& entries, Guid id)
		{
			const auto entryIt = entries.find(id);
			return entryIt == entries.end() ? nullptr : &entryIt->second.asset;
		}

		template <class T>
		static std::uint64_t Revision(const std::unordered_map<Guid, Entry<T>>& entries, Guid id)
		{
			const auto entryIt = entries.find(id);
			return entryIt == entries.end() ? 0 : entryIt->second.revision;
		}

		std::unordered_map<Guid, Entry<MeshAsset>> m_meshes;
		std::unordered_map<Guid, Entry<TextureAsset>> m_textures;
		std::unordered_map<Guid, Entry<ShaderAsset>> m_shaders;
	};
}

