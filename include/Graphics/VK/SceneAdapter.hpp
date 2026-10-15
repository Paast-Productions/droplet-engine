#pragma once

#include <Graphics/VK/VertexBuffer.hpp>
#include <Graphics/VK/IndexBuffer.hpp>
#include <vulkan/vulkan.h>
#include <map>
#include <Graphics/VK/Context.hpp>
#include <Graphics/VK/Sampler.hpp>
#include <SceneSystem/Scene.hpp>
#include <Graphics/VK/ImageView.hpp>
#include <Graphics/VK/Image.hpp>
#include <SceneSystem/Node.hpp>
#include <Graphics/VK/Allocator.hpp>
#include <Graphics/VK/DescriptorSet.hpp>
#include <Graphics/VK/StorageBuffer.hpp>
#include <glm/glm.hpp>
#include <Graphics/VK/Allocator.hpp>

namespace Droplet::Graphics::VK
{
	struct MeshData
	{
		vk::Buffer vertexBuffer;
		vk::Buffer indexBuffer;
		std::uint32_t indexCount;
	};

	struct MeshBuffer
	{
		VertexBuffer vertices{ nullptr };
		IndexBuffer indices{ nullptr };

		MeshBuffer &operator=(MeshBuffer &&p_other) noexcept
		{
			if (*this == p_other)
			{
				return *this;
			}

			std::swap(vertices, p_other.vertices);
			std::swap(indices, p_other.indices);

			return *this;
		}

		bool operator==(MeshBuffer &p_other) noexcept
		{
			return indices == p_other.indices && vertices == p_other.vertices;
		}

		MeshBuffer(MeshBuffer &&p_other) noexcept
		{
			std::swap(vertices, p_other.vertices);
			std::swap(indices, p_other.indices);
		}
	};

	struct SampleTexture
	{
		Sampler sampler;
		Image image;
		ImageView textureView;
	};

	struct PushConstants
	{
		std::uint32_t unibuf;
		std::uint32_t sampler;
	};

	struct RenderData
	{
		std::vector<MeshData> meshData;
		std::vector<PushConstants> pushConstants;
	};

	static class SceneAdapter
	{
	public:
		static SceneAdapter &GetInstance() 
		{
			static SceneAdapter s;
			return s;
		}

		~SceneAdapter() = default;

		void Initialize(const Context &p_context, Allocator &p_allocator, const CommandPool &p_commandPool, DescriptorSet &p_descriptorSet);

		void Deinitialize();

		void SubmitNodeData(const std::vector<Droplet::Scene::NodeData> p_nodeData);

		void UpdateMaps(const Context &p_context, Allocator &p_allocator, DescriptorSet &p_descriptorSet);

		const RenderData GetRenderData();
	private:
		SceneAdapter() {}

		std::unordered_map<std::uint32_t, VertexBuffer> m_vertexMap;
		std::unordered_map<std::uint32_t, IndexBuffer> m_indexMap;

		std::unordered_map<std::uint32_t, Image> m_imageMap;
		std::unordered_map<std::uint32_t, Sampler> m_samplerMap;
		std::unordered_map<std::uint32_t, ImageView> m_viewMap;

		std::unordered_map<std::uint32_t, StorageBuffer> m_bufferMap;

		RenderData m_renderData;
		std::vector<Droplet::Scene::NodeData> m_nodeData;
	};
}