#pragma once
#include <Graphics/VK/VertexBuffer.hpp>
#include <Graphics/VK/IndexBuffer.hpp>
#include <vulkan/vulkan.h>
#include <map>
#include <Graphics/VK/Context.hpp>
#include <Graphics/VK/Sampler.hpp>
#include <SceneSystem/Scene.hpp>
#include <ImageView.hpp>
#include <SceneSystem/Node.hpp>

namespace Droplet::Graphics::VK
{
	struct MeshData
	{
		vk::Buffer vertexBuffer;
		vk::Buffer indexBuffer;
	};

	struct MeshBuffer
	{
		VertexBuffer vertices;
		IndexBuffer indices;
	};

	struct SampleTexture
	{
		Sampler sampler;
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

	class SceneAdapter
	{
	public:
		SceneAdapter() = delete;
		~SceneAdapter() = default;

		SceneAdapter(const Context &p_context);

		void UpdateMaps(const std::vector<Droplet::Scene::NodeData> p_nodeData);

		const RenderData GetRenderData();
	private:

		std::unordered_map<std::uint32_t, MeshBuffer> m_meshMap;
		std::unordered_map<std::uint32_t, SampleTexture> m_textureMap;

		const Context &m_context;
	};
}