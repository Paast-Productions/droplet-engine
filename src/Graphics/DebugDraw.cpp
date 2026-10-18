#include "Graphics/DebugDraw.hpp"
#include <Graphics/VK/Camera.hpp>
#include <math/Bezier.hpp>
#include <math/Curves.hpp>
#include <glm/gtx/quaternion.hpp>
#include <tracy/public/tracy/Tracy.hpp>
#include <array>
#include <map>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <limits>
#define _USE_MATH_DEFINES
#include <math.h>
#undef _USE_MATH_DEFINES

using namespace Droplet::Math;
using namespace Droplet::Graphics;
using namespace Droplet::Graphics::Debug;


//
// Internal
//

namespace
{
	struct CameraUniform
	{
		glm::mat4 view;
		glm::mat4 projection;
	};

	std::vector<char> ReadShader(const std::filesystem::path &p_path)
	{
		ZoneScoped;

		std::ifstream file(p_path, std::ios::binary | std::ios::ate);
		if (!file)
		{
			throw std::runtime_error("Failed to open debug draw shader: " + p_path.string());
		}

		const std::streamoff size = static_cast<std::streamoff>(file.tellg());
		if (size <= 0 || (size % sizeof(std::uint32_t)) != 0)
		{
			throw std::runtime_error("Invalid debug draw shader bytecode: " + p_path.string());
		}

		std::vector<char> code(static_cast<std::size_t>(size));
		file.seekg(0);
		file.read(code.data(), static_cast<std::streamsize>(code.size()));
		if (!file)
		{
			throw std::runtime_error("Failed to read debug draw shader: " + p_path.string());
		}
		return code;
	}

	vk::raii::ShaderModule CreateShaderModule(const vk::raii::Device &p_device, const std::filesystem::path &p_path)
	{
		ZoneScoped;

		const std::vector<char> code = ReadShader(p_path);
		const vk::ShaderModuleCreateInfo createInfo
		{
			.codeSize = code.size(),
			.pCode = reinterpret_cast<const std::uint32_t *>(code.data())
		};
		return vk::raii::ShaderModule(p_device, createInfo);
	}

	vma::raii::Buffer CreateHostBuffer(const vma::raii::Allocator &p_allocator, vk::DeviceSize p_size, vk::BufferUsageFlags p_usage)
	{
		ZoneScoped;

		const vk::BufferCreateInfo bufferInfo
		{
			.size = p_size,
			.usage = p_usage,
			.sharingMode = vk::SharingMode::eExclusive
		};
		const vma::AllocationCreateInfo allocationInfo
		{
			.flags = vma::AllocationCreateFlagBits::eHostAccessSequentialWrite,
			.usage = vma::MemoryUsage::eAuto,
			.requiredFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
		};
		return vma::raii::Buffer(p_allocator, bufferInfo, allocationInfo);
	}

	template<typename T>
	void EnsureBuffer(const vma::raii::Allocator &p_allocator, vma::raii::Buffer &p_buffer, vk::DeviceSize &p_capacity, std::size_t p_count, vk::BufferUsageFlags p_usage)
	{
		ZoneScoped;

		const vk::DeviceSize requiredSize = static_cast<vk::DeviceSize>(p_count) * sizeof(T);
		if (requiredSize <= p_capacity)
		{
			return;
		}

		vk::DeviceSize newCapacity = std::max<vk::DeviceSize>(p_capacity, 4096);
		while (newCapacity < requiredSize)
		{
			if (newCapacity > std::numeric_limits<vk::DeviceSize>::max() / 2)
			{
				newCapacity = requiredSize;
				break;
			}
			newCapacity *= 2;
		}
		p_buffer = CreateHostBuffer(p_allocator, newCapacity, p_usage);
		p_capacity = newCapacity;
	}

	template<typename T>
	void UploadBuffer(vma::raii::Buffer &p_buffer, const std::vector<T> &p_data)
	{
		ZoneScoped;

		if (!p_data.empty())
		{
			p_buffer.getAllocation().copyFromMemory(p_data.data(), 0, p_data.size() * sizeof(T));
		}
	}


	namespace Icosahedron
	{
		// Source: https://schneide.blog/2016/07/15/generating-an-icosphere-in-c/

		using Index = int;

		struct Triangle
		{
			Index vertex[3];

			Triangle(Index v1, Index v2, Index v3) : vertex{ v1, v2, v3 } { }
		};

		using TriangleList = std::vector<Triangle>;
		using VertexList = std::vector<glm::vec3>;

		const float X = .525731112119133606f;
		const float Z = .850650808352039932f;
		const float N = 0.f;

		static const VertexList IcosahedronVertices =
		{
			{-X,N,Z},	{X,N,Z},	{-X,N,-Z},	{X,N,-Z},
			{N,Z,X},	{N,Z,-X},	{N,-Z,X},	{N,-Z,-X},
			{Z,X,N},	{-Z,X, N},	{Z,-X,N},	{-Z,-X, N}
		};

		static const TriangleList IcosahedronTriangles =
		{
			{0,4,1},	{0,9,4},	{9,5,4},	{4,5,8},	{4,8,1},
			{8,10,1},	{8,3,10},	{5,3,8},	{5,2,3},	{2,7,3},
			{7,10,3},	{7,6,10},	{7,11,6},	{11,0,6},	{0,1,6},
			{6,1,10},	{9,0,11},	{9,11,2},	{9,2,5},	{7,2,11}
		};

		using Lookup = std::map<std::pair<Index, Index>, Index>;

		static Index VertexForEdge(Lookup &p_lookup, VertexList &p_vertices, Index p_first, Index p_second)
		{
			ZoneScoped;

			Lookup::key_type key(p_first, p_second);
			if (key.first > key.second)
			{
				std::swap(key.first, key.second);
			}

			auto inserted = p_lookup.insert({ key, p_vertices.size() });
			if (inserted.second)
			{
				const glm::vec3 &edge0 = p_vertices[p_first];
				const glm::vec3 &edge1 = p_vertices[p_second];
				p_vertices.push_back(glm::normalize(edge0 + edge1));
			}

			return inserted.first->second;
		}

		static TriangleList Subdivide(VertexList &p_vertices, TriangleList p_triangles)
		{
			ZoneScoped;

			Lookup lookup;
			TriangleList result;

			for (auto &&each : p_triangles)
			{
				std::array<Index, 3> mid{};
				for (int edge = 0; edge < 3; ++edge)
				{
					mid[edge] = VertexForEdge(lookup, p_vertices,
						each.vertex[edge], each.vertex[(edge + 1) % 3]);
				}

				result.emplace_back( each.vertex[0], mid[0], mid[2] );
				result.emplace_back( each.vertex[1], mid[1], mid[0] );
				result.emplace_back( each.vertex[2], mid[2], mid[1] );
				result.emplace_back( mid[0],		 mid[1], mid[2] );
			}

			return result;
		}

		using IndexedMesh = std::pair<VertexList, TriangleList>;

		static IndexedMesh MakeIcosphere(int subdivisions)
		{
			ZoneScoped;

			VertexList vertices = IcosahedronVertices;
			TriangleList triangles = IcosahedronTriangles;

			for (int i = 0; i < subdivisions; ++i)
			{
				triangles = Subdivide(vertices, triangles);
			}

			return{ vertices, triangles };
		}
	};

	static void GenerateIcoSphere(int subdivisions, std::vector<glm::vec3> &outVertices, std::vector<int> &outIndices)
	{
		ZoneScoped;

		auto [vertices, triangles] = Icosahedron::MakeIcosphere(subdivisions);

		outVertices.resize(vertices.size());
		std::copy(vertices.begin(), vertices.end(), outVertices.begin());

		outIndices.resize(triangles.size() * 3);
		for (std::size_t i = 0; i < triangles.size(); ++i)
		{
			outIndices[i * 3 + 0] = triangles[i].vertex[0];
			outIndices[i * 3 + 1] = triangles[i].vertex[1];
			outIndices[i * 3 + 2] = triangles[i].vertex[2];
		}
	}

	static void GenerateCircle(int segments, std::vector<glm::vec3> &outVertices)
	{
		ZoneScoped;

		float invSegments = 1.0f / float(segments);

		outVertices.resize(segments);
		for (int i = 0; i < segments; ++i)
		{
			float angle = 2.0f * M_PI * float(i) * invSegments;
			outVertices[i] = glm::vec3(std::cos(angle), std::sin(angle), 0.0f);
		}
	}

	static void GenerateBezierCurve(const glm::vec3 &p_p1, const glm::vec3 &p_d, const glm::vec3 &p_p2, int segments, std::vector<glm::vec3> &outVertices)
	{
		glm::vec3 v0 = p_p1;
		glm::vec3 v1 = v0 + p_d;
		glm::vec3 v2 = p_p2;
		
		outVertices.clear();
		outVertices.reserve((std::size_t)segments + 2);

		outVertices.emplace_back(v0);

		for (int i = 1; i <= segments; ++i)
		{
			float t = float(i) / float(segments + 1);
			glm::vec3 point = Bezier::QuadraticInterpolate(v0, v1, v2, t);
			outVertices.emplace_back(point);
		}

		outVertices.emplace_back(v2);
	}

	static void GenerateBezierCurve(const glm::vec3 &p_p1, const glm::vec3 &p_d1, const glm::vec3 &p_d2, const glm::vec3 &p_p2, int segments, std::vector<glm::vec3> &outVertices)
	{
		glm::vec3 v0 = p_p1;
		glm::vec3 v1 = v0 + p_d1;
		glm::vec3 v2 = p_p2 + p_d2;
		glm::vec3 v3 = p_p2;

		outVertices.clear();
		outVertices.reserve((std::size_t)segments + 2);

		outVertices.emplace_back(v0);

		for (int i = 1; i <= segments; ++i)
		{
			float t = float(i) / float(segments + 1);
			glm::vec3 point = Bezier::CubicInterpolate(v0, v1, v2, v3, t);
			outVertices.emplace_back(point);
		}

		outVertices.emplace_back(v3);
	}

	static int ClampCurveSegments(int detail)
	{
		return std::clamp(detail, 3, 128);
	}

	static int ClampCircleSegments(int detail)
	{
		return std::clamp(detail, 3, 64);
	}

	static int ClampSphereSubdivisions(int detail)
	{
		return std::clamp(detail, 0, 6);
	}
}



//
// Renderer Backend
//

void DebugDraw::Initialize(
	const vk::raii::Instance &p_instance,
	const vk::raii::PhysicalDevice &p_physicalDevice,
	const vk::raii::Device &p_device,
	const vma::raii::Allocator &p_allocator,
	const vk::raii::Queue &p_queue,
	const vk::raii::CommandPool &p_commandPool,
	const vk::raii::DescriptorPool &p_descriptorPool,
	vk::Format p_colorFormat,
	vk::Format p_depthFormat,
	std::uint32_t p_framesInFlight)
{
	ZoneScoped;

	Shutdown();

	m_instance = &p_instance;
	m_physicalDevice = &p_physicalDevice;
	m_device = &p_device;
	m_allocator = &p_allocator;
	m_queue = &p_queue;
	m_commandPool = &p_commandPool;
	m_descriptorPool = &p_descriptorPool;
	m_colorFormat = p_colorFormat;
	m_depthFormat = p_depthFormat;

	ClearDrawCommands();
	m_wireMesh.clear();
	m_wireDepthMesh.clear();
	m_fillMesh.clear();
	m_fillDepthMesh.clear();
	m_iconMeshes.clear();
	m_iconDepthMeshes.clear();
	m_icosphereCache.clear();
	m_circleCache.clear();

	const vk::DescriptorSetLayoutBinding cameraBinding
	{
		.binding = 0,
		.descriptorType = vk::DescriptorType::eUniformBuffer,
		.descriptorCount = 1,
		.stageFlags = vk::ShaderStageFlagBits::eVertex
	};
	m_descriptorSetLayout = vk::raii::DescriptorSetLayout(p_device, vk::DescriptorSetLayoutCreateInfo
	{
		.bindingCount = 1,
		.pBindings = &cameraBinding
	});

	const vk::PipelineLayoutCreateInfo layoutInfo
	{
		.setLayoutCount = 1,
		.pSetLayouts = &*m_descriptorSetLayout
	};
	m_pipelineLayout = vk::raii::PipelineLayout(p_device, layoutInfo);

	m_shaderModules[0] = CreateShaderModule(p_device, "debug_draw.vert.spv");
	m_shaderModules[1] = CreateShaderModule(p_device, "debug_draw.frag.spv");
	m_shaderModules[2] = CreateShaderModule(p_device, "debug_icon.vert.spv");
	m_shaderModules[3] = CreateShaderModule(p_device, "debug_icon.frag.spv");

	m_frameResources.resize(p_framesInFlight);
	for (FrameResources &frame : m_frameResources)
	{
		frame.cameraBuffer = CreateHostBuffer(p_allocator, sizeof(CameraUniform), vk::BufferUsageFlagBits::eUniformBuffer);

		const vk::DescriptorSetAllocateInfo allocateInfo
		{
			.descriptorPool = *p_descriptorPool,
			.descriptorSetCount = 1,
			.pSetLayouts = &*m_descriptorSetLayout
		};
		frame.descriptorSet = std::move(p_device.allocateDescriptorSets(allocateInfo).front());

		const vk::DescriptorBufferInfo bufferInfo
		{
			.buffer = frame.cameraBuffer,
			.offset = 0,
			.range = sizeof(CameraUniform)
		};
		const vk::WriteDescriptorSet descriptorWrite
		{
			.dstSet = frame.descriptorSet,
			.dstBinding = 0,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eUniformBuffer,
			.pBufferInfo = &bufferInfo
		};
		p_device.updateDescriptorSets(descriptorWrite, {});
	}

	const auto createPipeline = [&](std::size_t p_shaderOffset, vk::PrimitiveTopology p_topology, bool p_icon, bool p_depthTest)
	{
		const std::array<vk::PipelineShaderStageCreateInfo, 2> stages
		{{
			{
				.stage = vk::ShaderStageFlagBits::eVertex,
				.module = *m_shaderModules[p_shaderOffset],
				.pName = p_icon ? "iconVertMain" : "vertMain"
			},
			{
				.stage = vk::ShaderStageFlagBits::eFragment,
				.module = *m_shaderModules[p_shaderOffset + 1],
				.pName = p_icon ? "iconFragMain" : "fragMain"
			}
		}};

		const std::array<vk::VertexInputAttributeDescription, 3> iconAttributes
		{{
			{ .location = 0, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(IconVertex, position) },
			{ .location = 1, .binding = 0, .format = vk::Format::eR32G32B32A32Sfloat, .offset = offsetof(IconVertex, color) },
			{ .location = 2, .binding = 0, .format = vk::Format::eR32G32Sfloat, .offset = offsetof(IconVertex, texCoord) }
		}};
		const std::array<vk::VertexInputAttributeDescription, 2> meshAttributes
		{{
			{ .location = 0, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(Vertex, position) },
			{ .location = 1, .binding = 0, .format = vk::Format::eR32G32B32A32Sfloat, .offset = offsetof(Vertex, color) }
		}};
		const vk::VertexInputBindingDescription binding
		{
			.binding = 0,
			.stride = p_icon ? sizeof(IconVertex) : sizeof(Vertex),
			.inputRate = vk::VertexInputRate::eVertex
		};
		const vk::VertexInputAttributeDescription *attributeData = p_icon ? iconAttributes.data() : meshAttributes.data();
		const std::uint32_t attributeCount = p_icon ? static_cast<std::uint32_t>(iconAttributes.size()) : static_cast<std::uint32_t>(meshAttributes.size());
		const vk::PipelineVertexInputStateCreateInfo vertexInput
		{
			.vertexBindingDescriptionCount = 1,
			.pVertexBindingDescriptions = &binding,
			.vertexAttributeDescriptionCount = attributeCount,
			.pVertexAttributeDescriptions = attributeData
		};
		const vk::PipelineInputAssemblyStateCreateInfo inputAssembly { .topology = p_topology };
		const vk::PipelineViewportStateCreateInfo viewportState { .viewportCount = 1, .scissorCount = 1 };
		const vk::PipelineRasterizationStateCreateInfo rasterizer
		{
			.polygonMode = vk::PolygonMode::eFill,
			.cullMode = vk::CullModeFlagBits::eNone,
			.frontFace = vk::FrontFace::eCounterClockwise,
			.lineWidth = 1.0f
		};
		const vk::PipelineMultisampleStateCreateInfo multisampling { .rasterizationSamples = vk::SampleCountFlagBits::e1 };
		const vk::PipelineDepthStencilStateCreateInfo depthStencil
		{
			.depthTestEnable = p_depthTest ? vk::True : vk::False,
			.depthWriteEnable = vk::False,
			.depthCompareOp = vk::CompareOp::eLessOrEqual
		};
		const vk::PipelineColorBlendAttachmentState blendAttachment
		{
			.blendEnable = vk::True,
			.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
			.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
			.colorBlendOp = vk::BlendOp::eAdd,
			.srcAlphaBlendFactor = vk::BlendFactor::eOne,
			.dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
			.alphaBlendOp = vk::BlendOp::eAdd,
			.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
		};
		const vk::PipelineColorBlendStateCreateInfo colorBlending
		{
			.attachmentCount = 1,
			.pAttachments = &blendAttachment
		};
		const std::array<vk::DynamicState, 2> dynamicStates { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
		const vk::PipelineDynamicStateCreateInfo dynamicState
		{
			.dynamicStateCount = static_cast<std::uint32_t>(dynamicStates.size()),
			.pDynamicStates = dynamicStates.data()
		};
		vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> createInfo
		{
			{
				.stageCount = static_cast<std::uint32_t>(stages.size()),
				.pStages = stages.data(),
				.pVertexInputState = &vertexInput,
				.pInputAssemblyState = &inputAssembly,
				.pViewportState = &viewportState,
				.pRasterizationState = &rasterizer,
				.pMultisampleState = &multisampling,
				.pDepthStencilState = &depthStencil,
				.pColorBlendState = &colorBlending,
				.pDynamicState = &dynamicState,
				.layout = *m_pipelineLayout
			},
			{
				.colorAttachmentCount = 1,
				.pColorAttachmentFormats = &m_colorFormat,
				.depthAttachmentFormat = m_depthFormat
			}
		};
		return vk::raii::Pipeline(p_device, nullptr, createInfo.get<vk::GraphicsPipelineCreateInfo>());
	};

	m_pipelines[0] = createPipeline(0, vk::PrimitiveTopology::eLineList, false, true);
	m_pipelines[1] = createPipeline(0, vk::PrimitiveTopology::eLineList, false, false);
	m_pipelines[2] = createPipeline(0, vk::PrimitiveTopology::eTriangleList, false, true);
	m_pipelines[3] = createPipeline(0, vk::PrimitiveTopology::eTriangleList, false, false);
	m_pipelines[4] = createPipeline(2, vk::PrimitiveTopology::eTriangleList, true, true);
	m_pipelines[5] = createPipeline(2, vk::PrimitiveTopology::eTriangleList, true, false);
}

void DebugDraw::Shutdown()
{
	ZoneScoped;

	if (m_device)
	{
		(*m_device).waitIdle();
	}

	m_frameResources.clear();
	for (auto &pipeline : m_pipelines)
	{
		pipeline = nullptr;
	}
	m_pipelineLayout = nullptr;
	for (auto &shaderModule : m_shaderModules)
	{
		shaderModule = nullptr;
	}
	m_descriptorSetLayout = nullptr;

	m_instance = nullptr;
	m_physicalDevice = nullptr;
	m_device = nullptr;
	m_allocator = nullptr;
	m_queue = nullptr;
	m_commandPool = nullptr;
	m_descriptorPool = nullptr;
	m_colorFormat = {};
	m_depthFormat = {};
}

void DebugDraw::PrepareRender(std::uint32_t p_frameIndex)
{
	ZoneScoped;

	// Generate all but icon draw data
	// Combine all wireframe draw data into two meshes (depth + no depth) and all filled draw data into two other meshes

	std::vector<Vertex> wireMesh, wireDepthMesh;
	std::vector<Vertex> fillMesh, fillDepthMesh;

	for (const auto &[opt, lines] : m_wireLists)
	{
		if (opt.depthTest)
		{
			AppendToMesh(wireDepthMesh, lines, opt.color);
		}
		else
		{
			AppendToMesh(wireMesh, lines, opt.color);
		}
	}

	for (const auto &[opt, tris] : m_fillLists)
	{
		if (opt.depthTest)
		{
			AppendToMesh(fillDepthMesh, tris, opt.color);
		}
		else
		{
			AppendToMesh(fillMesh, tris, opt.color);
		}
	}

	m_wireMesh = std::move(wireMesh);
	m_wireDepthMesh = std::move(wireDepthMesh);
	m_fillMesh = std::move(fillMesh);
	m_fillDepthMesh = std::move(fillDepthMesh);

	if (!m_allocator || p_frameIndex >= m_frameResources.size())
	{
		return;
	}
	FrameResources &frame = m_frameResources[p_frameIndex];

	std::vector<Vertex> wireVertices;
	wireVertices.reserve(m_wireMesh.size() + m_wireDepthMesh.size());
	wireVertices.insert(wireVertices.end(), m_wireMesh.begin(), m_wireMesh.end());
	frame.wireDepthFirst = static_cast<std::uint32_t>(wireVertices.size());
	frame.wireDepthCount = static_cast<std::uint32_t>(m_wireDepthMesh.size());
	wireVertices.insert(wireVertices.end(), m_wireDepthMesh.begin(), m_wireDepthMesh.end());
	EnsureBuffer<Vertex>(*m_allocator, frame.wireBuffer, frame.wireCapacity, wireVertices.size(), vk::BufferUsageFlagBits::eVertexBuffer);
	UploadBuffer(frame.wireBuffer, wireVertices);

	std::vector<Vertex> fillVertices;
	fillVertices.reserve(m_fillMesh.size() + m_fillDepthMesh.size());
	fillVertices.insert(fillVertices.end(), m_fillMesh.begin(), m_fillMesh.end());
	frame.fillDepthFirst = static_cast<std::uint32_t>(fillVertices.size());
	frame.fillDepthCount = static_cast<std::uint32_t>(m_fillDepthMesh.size());
	fillVertices.insert(fillVertices.end(), m_fillDepthMesh.begin(), m_fillDepthMesh.end());
	EnsureBuffer<Vertex>(*m_allocator, frame.fillBuffer, frame.fillCapacity, fillVertices.size(), vk::BufferUsageFlagBits::eVertexBuffer);
	UploadBuffer(frame.fillBuffer, fillVertices);

	std::vector<std::uint32_t> indices(std::max(wireVertices.size(), fillVertices.size()));
	for (std::uint32_t i = 0; i < indices.size(); ++i)
	{
		indices[i] = i;
	}
	EnsureBuffer<std::uint32_t>(*m_allocator, frame.indexBuffer, frame.indexCapacity, indices.size(), vk::BufferUsageFlagBits::eIndexBuffer);
	UploadBuffer(frame.indexBuffer, indices);
}

void DebugDraw::RenderToCamera(const Camera &p_camera, const vk::raii::CommandBuffer &p_commandBuffer, vk::Extent2D p_extent, std::uint32_t p_frameIndex)
{
	ZoneScoped;

	if (!m_device || p_frameIndex >= m_frameResources.size() || p_extent.width == 0 || p_extent.height == 0)
	{
		return;
	}

	FrameResources &frame = m_frameResources[p_frameIndex];
	const float aspectRatio = static_cast<float>(p_extent.width) / static_cast<float>(p_extent.height);
	const CameraUniform cameraUniform { p_camera.GetViewMatrix(), p_camera.GetProjectionMatrix(aspectRatio) };
	frame.cameraBuffer.getAllocation().copyFromMemory(&cameraUniform, 0, sizeof(cameraUniform));

	glm::vec3 cameraRight = -p_camera.GetRight();
	glm::vec3 cameraUp = p_camera.GetUp();

	std::map<GUID, std::vector<IconVertex>> iconMeshes, iconDepthMeshes;

	for (const auto &[opt, icons] : m_iconLists)
	{
		if (opt.depthTest)
		{
			AppendToMesh(iconDepthMeshes[opt.icon], icons, opt.color, cameraRight, cameraUp);
		}
		else
		{
			AppendToMesh(iconMeshes[opt.icon], icons, opt.color, cameraRight, cameraUp);
		}
	}
	
	m_iconMeshes = std::move(iconMeshes);
	m_iconDepthMeshes = std::move(iconDepthMeshes);

	std::vector<IconVertex> iconVertices;
	for (const auto &[icon, vertices] : m_iconMeshes)
	{
		(void)icon;
		iconVertices.insert(iconVertices.end(), vertices.begin(), vertices.end());
	}
	frame.iconDepthFirst = static_cast<std::uint32_t>(iconVertices.size());
	for (const auto &[icon, vertices] : m_iconDepthMeshes)
	{
		(void)icon;
		iconVertices.insert(iconVertices.end(), vertices.begin(), vertices.end());
	}
	EnsureBuffer<IconVertex>(*m_allocator, frame.iconBuffer, frame.iconCapacity, iconVertices.size(), vk::BufferUsageFlagBits::eVertexBuffer);
	UploadBuffer(frame.iconBuffer, iconVertices);
	const std::size_t indexCount = std::max({ m_wireMesh.size() + m_wireDepthMesh.size(), m_fillMesh.size() + m_fillDepthMesh.size(), iconVertices.size() });
	std::vector<std::uint32_t> indices(indexCount);
	for (std::uint32_t i = 0; i < indices.size(); ++i)
	{
		indices[i] = i;
	}
	EnsureBuffer<std::uint32_t>(*m_allocator, frame.indexBuffer, frame.indexCapacity, indices.size(), vk::BufferUsageFlagBits::eIndexBuffer);
	UploadBuffer(frame.indexBuffer, indices);

	const vk::Viewport viewport
	{
		.x = 0.0f,
		.y = static_cast<float>(p_extent.height),
		.width = static_cast<float>(p_extent.width),
		.height = -static_cast<float>(p_extent.height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};
	const vk::Rect2D scissor { vk::Offset2D(0, 0), p_extent };
	p_commandBuffer.setViewport(0, viewport);
	p_commandBuffer.setScissor(0, scissor);
	p_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *m_pipelineLayout, 0, *frame.descriptorSet, nullptr);

	const auto drawMesh = [&](std::size_t p_pipelineIndex, const vma::raii::Buffer &p_buffer, std::uint32_t p_firstVertex, std::uint32_t p_vertexCount)
	{
		if (p_vertexCount == 0 || p_buffer == nullptr || frame.indexBuffer == nullptr)
		{
			return;
		}
		p_commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_pipelines[p_pipelineIndex]);
		const vk::DeviceSize offset = 0;
		p_commandBuffer.bindVertexBuffers(0, *p_buffer, { offset });
		p_commandBuffer.bindIndexBuffer(*frame.indexBuffer, 0, vk::IndexType::eUint32);
		p_commandBuffer.drawIndexed(p_vertexCount, 1, p_firstVertex, 0, 0);
	};

	drawMesh(1, frame.wireBuffer, 0, static_cast<std::uint32_t>(m_wireMesh.size()));
	drawMesh(0, frame.wireBuffer, frame.wireDepthFirst, frame.wireDepthCount);
	drawMesh(3, frame.fillBuffer, 0, static_cast<std::uint32_t>(m_fillMesh.size()));
	drawMesh(2, frame.fillBuffer, frame.fillDepthFirst, frame.fillDepthCount);

	const auto drawIcons = [&](const std::map<GUID, std::vector<IconVertex>> &p_meshes, std::size_t p_pipelineIndex, std::uint32_t p_firstVertex)
	{
		std::uint32_t firstVertex = p_firstVertex;
		for (const auto &[icon, vertices] : p_meshes)
		{
			(void)icon;
			drawMesh(p_pipelineIndex, frame.iconBuffer, firstVertex, static_cast<std::uint32_t>(vertices.size()));
			firstVertex += static_cast<std::uint32_t>(vertices.size());
		}
	};
	drawIcons(m_iconMeshes, 5, 0);
	drawIcons(m_iconDepthMeshes, 4, frame.iconDepthFirst);
}

void DebugDraw::ClearDrawCommands()
{
	ZoneScoped;

	m_wireLists = {};
	m_fillLists = {};
	m_iconLists = {};
	m_wireMesh.clear();
	m_wireDepthMesh.clear();
	m_fillMesh.clear();
	m_fillDepthMesh.clear();
	m_iconMeshes.clear();
	m_iconDepthMeshes.clear();
}



//
// Mesh Generation
//

void DebugDraw::AppendToMesh(std::vector<Vertex> &p_mesh, const std::vector<DbgLine> &p_lines, glm::vec4 p_color)
{
	ZoneScoped;

	p_mesh.reserve(p_mesh.size() + p_lines.size() * 2);
	for (const auto &line : p_lines)
	{
		p_mesh.emplace_back(Vertex{ line.v1, p_color });
		p_mesh.emplace_back(Vertex{ line.v2, p_color });
	}
}

void DebugDraw::AppendToMesh(std::vector<Vertex> &p_mesh, const std::vector<DbgTri> &p_tris, glm::vec4 p_color)
{
	ZoneScoped;

	p_mesh.reserve(p_mesh.size() + p_tris.size() * 3);
	for (const auto &tri : p_tris)
	{
		p_mesh.emplace_back(Vertex{ tri.v1, p_color });
		p_mesh.emplace_back(Vertex{ tri.v2, p_color });
		p_mesh.emplace_back(Vertex{ tri.v3, p_color });
	}
}

void DebugDraw::AppendToMesh(std::vector<IconVertex> &p_mesh, const std::vector<DbgIcon> &p_icons, glm::vec4 p_color, glm::vec3 p_right, glm::vec3 p_up)
{
	ZoneScoped;

	p_mesh.reserve(p_mesh.size() + p_icons.size() * 6);
	for (const auto &icon : p_icons)
	{
		glm::vec3 center = icon.position;
		glm::vec3 right = p_right * icon.scale.x * 0.5f;
		glm::vec3 up = p_up * icon.scale.y * 0.5f;

		const IconVertex bottomLeft { center - right - up, p_color, glm::vec2(0.0f, 0.0f) };
		const IconVertex bottomRight { center + right - up, p_color, glm::vec2(1.0f, 0.0f) };
		const IconVertex topRight { center + right + up, p_color, glm::vec2(1.0f, 1.0f) };
		const IconVertex topLeft { center - right + up, p_color, glm::vec2(0.0f, 1.0f) };
		p_mesh.insert(p_mesh.end(), { bottomLeft, bottomRight, topRight, bottomLeft, topRight, topLeft });
	}
}



//
// Drawing Functions
//

void DebugDraw::DrawRay(const Ray &p_ray, glm::vec4 p_col, bool p_depth)
{
	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_col, p_depth);
	dbgDraw.m_wireLists[intOpt].emplace_back(p_ray.pos, p_ray.pos + p_ray.dir);
}

void DebugDraw::DrawLine(const glm::vec3 &p_start, const glm::vec3 &p_end, glm::vec4 p_col, bool p_depth)
{
	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_col, p_depth);
	dbgDraw.m_wireLists[intOpt].emplace_back(p_start, p_end);
}

void DebugDraw::DrawTri(const glm::vec3 &p_v1, const glm::vec3 &p_v2, const glm::vec3 &p_v3, glm::vec4 p_col, bool p_depth, bool p_wire)
{
	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_col, p_depth);
	dbgDraw.m_fillLists[intOpt].emplace_back(p_v1, p_v2, p_v3);
}

void DebugDraw::DrawQuad(const glm::vec3 &p_center, const glm::vec3 &p_up, const glm::vec3 &p_right, glm::vec4 p_col, bool p_depth, bool p_wire)
{
	// Calculate the four corners of the quad
	glm::vec3 v1 = p_center - p_right - p_up; // Bottom-left
	glm::vec3 v2 = p_center + p_right - p_up; // Bottom-right
	glm::vec3 v3 = p_center + p_right + p_up; // Top-right
	glm::vec3 v4 = p_center - p_right + p_up; // Top-left

	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_col, p_depth);

	if (p_wire)
	{
		dbgDraw.m_wireLists[intOpt].emplace_back(v1, v2);
		dbgDraw.m_wireLists[intOpt].emplace_back(v2, v3);
		dbgDraw.m_wireLists[intOpt].emplace_back(v3, v4);
		dbgDraw.m_wireLists[intOpt].emplace_back(v4, v1);
	}
	else
	{
		dbgDraw.m_fillLists[intOpt].emplace_back(v1, v2, v3);
		dbgDraw.m_fillLists[intOpt].emplace_back(v1, v3, v4);
	}
}

void DebugDraw::DrawCurve(const glm::vec3 &p_p1, const glm::vec3 &p_d, const glm::vec3 &p_p2, glm::vec4 p_col, int p_seg, bool p_depth)
{
	std::vector<glm::vec3> curvePoints;
	GenerateBezierCurve(p_p1, p_d, p_p2, ClampCurveSegments(p_seg), curvePoints);

	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_col, p_depth);

	dbgDraw.m_wireLists[intOpt].reserve(dbgDraw.m_wireLists[intOpt].size() + curvePoints.size() - 1);

	for (std::size_t i = 0; i < curvePoints.size() - 1; ++i)
	{
		dbgDraw.m_wireLists[intOpt].emplace_back(curvePoints[i], curvePoints[i + 1]);
	}
}

void DebugDraw::DrawCurve(const glm::vec3 &p_p1, const glm::vec3 &p_d1, const glm::vec3 &p_d2, const glm::vec3 &p_p2, glm::vec4 p_col, int p_seg, bool p_depth)
{
	std::vector<glm::vec3> curvePoints;
	GenerateBezierCurve(p_p1, p_d1, p_d2, p_p2, ClampCurveSegments(p_seg), curvePoints);

	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_col, p_depth);

	dbgDraw.m_wireLists[intOpt].reserve(dbgDraw.m_wireLists[intOpt].size() + curvePoints.size() - 1);

	for (std::size_t i = 0; i < curvePoints.size() - 1; ++i)
	{
		dbgDraw.m_wireLists[intOpt].emplace_back(curvePoints[i], curvePoints[i + 1]);
	}
}

void DebugDraw::DrawBox(const AABB &p_aabb, const glm::vec4 p_col, bool p_depth, bool p_wire)
{
	glm::vec3 min = p_aabb.GetMin();
	glm::vec3 max = p_aabb.GetMax();

	glm::vec3 v1 = glm::vec3(min.x, min.y, min.z);
	glm::vec3 v2 = glm::vec3(max.x, min.y, min.z);
	glm::vec3 v3 = glm::vec3(max.x, max.y, min.z);
	glm::vec3 v4 = glm::vec3(min.x, max.y, min.z);
	glm::vec3 v5 = glm::vec3(min.x, min.y, max.z);
	glm::vec3 v6 = glm::vec3(max.x, min.y, max.z);
	glm::vec3 v7 = glm::vec3(max.x, max.y, max.z);
	glm::vec3 v8 = glm::vec3(min.x, max.y, max.z);

	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_col, p_depth);

	if (p_wire)
	{
		// Draw wireframe edges
		std::vector<DbgLine> lines = {
			{v1, v2}, {v2, v3}, {v3, v4}, {v4, v1}, // Bottom face
			{v5, v6}, {v6, v7}, {v7, v8}, {v8, v5}, // Top face
			{v1, v5}, {v2, v6}, {v3, v7}, {v4, v8}  // Vertical edges
		};
		dbgDraw.m_wireLists[intOpt].insert(dbgDraw.m_wireLists[intOpt].end(), lines.begin(), lines.end());
	}
	else
	{
		// Draw filled faces
		std::vector<DbgTri> triangles = {
			// Bottom face
			{v1, v2, v3}, {v1, v3, v4},
			// Top face
			{v5, v6, v7}, {v5, v7, v8},
			// Front face
			{v1, v2, v6}, {v1, v6, v5},
			// Back face
			{v4, v3, v7}, {v4, v7, v8},
			// Left face
			{v1, v4, v8}, {v1, v8, v5},
			// Right face
			{v2, v3, v7}, {v2, v7, v6}
		};
		dbgDraw.m_fillLists[intOpt].insert(dbgDraw.m_fillLists[intOpt].end(), triangles.begin(), triangles.end());
	}
}

void DebugDraw::DrawBox(const OBB &p_obb, glm::vec4 p_col, bool p_depth, bool p_wire)
{
	// Calculate the corners
	std::vector<glm::vec3> v;
	v.resize(8);
	p_obb.GetCorners(v.data());

	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_col, p_depth);

	if (p_wire)
	{
		// Draw wireframe edges
		std::vector<DbgLine> lines = {
			{v[0], v[1]}, {v[1], v[2]}, {v[2], v[3]}, {v[3], v[0]}, // Bottom face
			{v[4], v[5]}, {v[5], v[6]}, {v[6], v[7]}, {v[7], v[4]}, // Top face
			{v[0], v[4]}, {v[1], v[5]}, {v[2], v[6]}, {v[3], v[7]}  // Vertical edges
		};
		dbgDraw.m_wireLists[intOpt].insert(dbgDraw.m_wireLists[intOpt].end(), lines.begin(), lines.end());
	}
	else
	{
		// Draw filled faces
		std::vector<DbgTri> triangles = {
			// Bottom face
			{v[0], v[1], v[2]}, {v[0], v[2], v[3]},
			// Top face
			{v[4], v[5], v[6]}, {v[4], v[6], v[7]},
			// Front face
			{v[0], v[1], v[5]}, {v[0], v[5], v[4]},
			// Back face
			{v[3], v[2], v[6]}, {v[3], v[6], v[7]},
			// Left face
			{v[0], v[3], v[7]}, {v[0], v[7], v[4]},
			// Right face
			{v[1], v[2], v[6]}, {v[1], v[6], v[5]}
		};
		dbgDraw.m_fillLists[intOpt].insert(dbgDraw.m_fillLists[intOpt].end(), triangles.begin(), triangles.end());
	}
}

void DebugDraw::DrawSphere(const Sphere &p_sphere, glm::vec4 p_col, int p_subdiv, bool p_depth)
{
	DebugDraw &dbgDraw = DebugDraw::Get();
	const int subdivisions = ClampSphereSubdivisions(p_subdiv);

	if (dbgDraw.m_icosphereCache.find(subdivisions) == dbgDraw.m_icosphereCache.end())
	{
		std::vector<glm::vec3> vertices;
		std::vector<int> indices;
		::GenerateIcoSphere(subdivisions, vertices, indices);

		std::vector<DbgTri> triangles;
		triangles.reserve(indices.size() / 3);

		for (std::size_t i = 0; i < indices.size(); i += 3)
		{
			triangles.emplace_back(
				vertices[indices[i + 0]],
				vertices[indices[i + 1]],
				vertices[indices[i + 2]]
			);
		}

		dbgDraw.m_icosphereCache[subdivisions] = std::move(triangles);
	}

	std::vector<DbgTri> triangles = dbgDraw.m_icosphereCache[subdivisions];

	for (auto &tri : triangles)
	{
		tri.v1 = p_sphere.center + tri.v1 * p_sphere.radius;
		tri.v2 = p_sphere.center + tri.v2 * p_sphere.radius;
		tri.v3 = p_sphere.center + tri.v3 * p_sphere.radius;
	}

	InternalDrawOptions intOpt(p_col, p_depth);

	dbgDraw.m_fillLists[intOpt].insert(dbgDraw.m_fillLists[intOpt].end(), triangles.begin(), triangles.end());
}

void DebugDraw::DrawCircle(const glm::vec3 &p_center, const glm::vec3 &p_normal, float p_radius, glm::vec4 p_col, int p_seg, bool p_depth, bool p_wire)
{
	DebugDraw &dbgDraw = DebugDraw::Get();
	const int segments = ClampCircleSegments(p_seg);

	if (dbgDraw.m_circleCache.find(segments) == dbgDraw.m_circleCache.end())
	{
		std::vector<glm::vec3> points;

		::GenerateCircle(segments, points);

		dbgDraw.m_circleCache[segments] = std::move(points);
	}

	std::vector<glm::vec3> points = dbgDraw.m_circleCache[segments];

	// points is a unit circle in the XY plane, we need to transform it to the desired position and orientation
	// Create a rotation matrix that aligns the Z axis with the normal
	glm::vec3 zAxis = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 normal = p_normal;
	if (glm::dot(normal, normal) < 1e-8f)
	{
		normal = zAxis;
	}
	else
	{
		normal = glm::normalize(normal);
	}

	float dot = glm::clamp(glm::dot(zAxis, normal), -1.0f, 1.0f);
	glm::mat4 rotation = glm::mat4(1.0f);
	if (dot <= -1.0f + 1e-6f)
	{
		rotation = glm::rotate(glm::mat4(1.0f), float(M_PI), glm::vec3(1.0f, 0.0f, 0.0f));
	}
	else if (dot < 1.0f - 1e-6f)
	{
		glm::vec3 axis = glm::normalize(glm::cross(zAxis, normal));
		float angle = std::acos(dot);
		rotation = glm::rotate(glm::mat4(1.0f), angle, axis);
	}

	for (auto &p : points)
	{
		p = p_center + glm::vec3(rotation * glm::vec4(p * p_radius, 1.0f));
	}

	InternalDrawOptions intOpt(p_col, p_depth);

	if (p_wire)
	{
		std::vector<DbgLine> lines;
		lines.reserve(points.size());

		for (std::size_t i = 0; i < points.size(); ++i)
		{
			lines.emplace_back(points[i], points[(i + 1) % points.size()]);
		}

		dbgDraw.m_wireLists[intOpt].insert(dbgDraw.m_wireLists[intOpt].end(), lines.begin(), lines.end());
	}
	else
	{
		std::vector<DbgTri> triangles;
		triangles.reserve(points.size());

		for (std::size_t i = 0; i < points.size(); ++i)
		{
			triangles.emplace_back(p_center, points[i], points[(i + 1) % points.size()]);
		}

		dbgDraw.m_fillLists[intOpt].insert(dbgDraw.m_fillLists[intOpt].end(), triangles.begin(), triangles.end());
	}
}

void DebugDraw::DrawFrustum(const Frustum &p_frustum, glm::vec4 p_col, bool p_depth, bool p_wire)
{
	// Calculate the eight corners of the frustum
	std::vector<glm::vec3> v;
	v.resize(8);
	p_frustum.GetCorners(v.data());

	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_col, p_depth);

	if (p_wire)
	{
		// Draw wireframe edges
		std::vector<DbgLine> lines = {
			{v[0], v[1]}, {v[1], v[3]}, {v[3], v[2]}, {v[2], v[0]}, // Near face
			{v[4], v[5]}, {v[5], v[7]}, {v[7], v[6]}, {v[6], v[4]}, // Far face
			{v[0], v[4]}, {v[1], v[5]}, {v[2], v[6]}, {v[3], v[7]}  // Connecting edges
		};
		dbgDraw.m_wireLists[intOpt].insert(dbgDraw.m_wireLists[intOpt].end(), lines.begin(), lines.end());
	}
	else
	{
		// Draw filled faces
		std::vector<DbgTri> triangles = {
			// Near face
			{v[0], v[1], v[3]}, {v[0], v[3], v[2]},
			// Far face
			{v[4], v[5], v[7]}, {v[4], v[7], v[6]},
			// Left face
			{v[0], v[2], v[6]}, {v[0], v[6], v[4]},
			// Right face
			{v[1], v[3], v[7]}, {v[1], v[7], v[5]},
			// Top face
			{v[0], v[1], v[5]}, {v[0], v[5], v[4]},
			// Bottom face
			{v[2], v[3], v[7]}, {v[2], v[7], v[6]}
		};
		dbgDraw.m_fillLists[intOpt].insert(dbgDraw.m_fillLists[intOpt].end(), triangles.begin(), triangles.end());
	}
}

void DebugDraw::DrawIcon(const GUID &p_icon, const glm::vec3 &p_pos, const glm::vec2 &p_scale, glm::vec4 p_col, bool p_depth)
{
	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_col, p_depth, p_icon);

	dbgDraw.m_iconLists[intOpt].emplace_back(
		p_pos,
		p_scale
	);
}
