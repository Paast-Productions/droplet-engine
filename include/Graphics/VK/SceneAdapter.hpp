#pragma once
#include <Graphics/VK/VertexBuffer.hpp>
#include <Graphics/VK/IndexBuffer.hpp>
#include <vulkan/vulkan.h>
#include <map>
#include <Graphics/VK/Context.hpp>
#include <Graphics/VK/Sampler.hpp>
#include <SceneSystem/Scene.hpp>

namespace Droplet::Graphics::VK
{
	struct MeshData
	{
		VkBuffer vertexBuffer;
		VkBuffer indexBuffer;
	};

	struct PushConstants
	{
		std::uint16_t unibuf;
		std::uint16_t sampler;
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

		const RenderData GetRenderData();
	private:

		std::unordered_map<VertexBuffer, std::uint16_t> m_vertexMap;
		std::unordered_map<IndexBuffer, std::uint16_t> m_indexMap;
		std::unordered_map<Sampler, std::uint16_t> m_samplerMap;

		const Context &m_context;
	};
}