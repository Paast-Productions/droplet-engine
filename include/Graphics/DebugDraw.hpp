#pragma once
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <math/bounds/Ray.hpp>
#include <math/bounds/AABB.hpp>
#include <math/bounds/OBB.hpp>
#include <math/bounds/Sphere.hpp>
#include <math/bounds/Frustum.hpp>
#include <resource/GuidUtils.hpp>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc_raii.hpp>
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#include <array>
#include <cstdint>

namespace Droplet::Graphics
{
	// Forward declarations
	class Renderer;
	class Camera;
}

namespace Droplet::Graphics::Debug
{
	/// @brief A utility class for rendering debug shapes in 3D space.
	/// 
	/// Handles the drawing of lines, tris, quads, curves, bounding volumes and icons.
	/// 
	/// @note Initialized and managed by the Renderer class.
	class DebugDraw
	{
	public:

		/// @brief Draw a ray in 3D space.
		/// @param p_ray The ray to be drawn, represented by its origin and direction.
		/// @param p_col The color of the ray.
		/// @param p_depth Whether to enable depth testing for the ray.
		static void DrawRay(
			const Math::Ray &p_ray,
			glm::vec4 p_col = glm::vec4(0, 0, 0, 1),
			bool p_depth = true
		);

		/// @brief Draw a line segment in 3D space.
		/// @param p_start The start point of the line segment.
		/// @param p_end The end point of the line segment.
		/// @param p_col The color of the line segment.
		/// @param p_depth Whether to enable depth testing for the line segment.
		static void DrawLine(
			const glm::vec3 &p_start, const glm::vec3 &p_end,
			glm::vec4 p_col = glm::vec4(0, 0, 0, 1),
			bool p_depth = true
		);

		/// @brief Draw a triangle in 3D space.
		/// @param p_v1 The first vertex of the triangle.
		/// @param p_v2 The second vertex of the triangle.
		/// @param p_v3 The third vertex of the triangle.
		/// @param p_col The color of the triangle.
		/// @param p_depth Whether to enable depth testing for the triangle.
		/// @param p_wire Whether to draw only the wireframe of the triangle.
		static void DrawTri(
			const glm::vec3 &p_v1, const glm::vec3 &p_v2, const glm::vec3 &p_v3,
			glm::vec4 p_col = glm::vec4(0, 0, 0, 1),
			bool p_depth = true,
			bool p_wire = false
		);

		/// @brief Draw a quad in 3D space.
		/// @param p_center The center point of the quad.
		/// @param p_up The up direction and legth of the quad.
		/// @param p_right The right direction and length of the quad.
		/// @param p_col The color of the quad.
		/// @param p_depth Whether to enable depth testing for the quad.
		/// @param p_wire Whether to draw only the wireframe of the quad.
		static void DrawQuad(
			const glm::vec3 &p_center, const glm::vec3 &p_up, const glm::vec3 &p_right,
			glm::vec4 p_col = glm::vec4(0, 0, 0, 1),
			bool p_depth = true,
			bool p_wire = false
		);

		/// @brief Draw a bezier curve with one control point in 3D space.
		/// @param p_p1 Start point of curve.
		/// @param p_d Vector from start point to control point.
		/// @param p_p2 End point of curve.
		/// @param p_col Color of the curve.
		/// @param p_seg Number of segments to divide the curve into.
		/// @param p_depth Whether to enable depth testing for the curve.
		static void DrawCurve(
			const glm::vec3 &p_p1, const glm::vec3 &p_d, const glm::vec3 &p_p2,
			glm::vec4 p_col = glm::vec4(0, 0, 0, 1),
			int p_seg = 12,
			bool p_depth = true
		);

		/// @brief Draw a bezier curve with two control points in 3D space.
		/// @param p_p1 Start point of curve.
		/// @param p_d1 Vector from start point to first control point.
		/// @param p_d2 Vector from end point to second control point.
		/// @param p_p2 End point of curve.
		/// @param p_col Color of the curve.
		/// @param p_seg Number of segments to divide the curve into.
		/// @param p_depth Whether to enable depth testing for the curve.
		static void DrawCurve(
			const glm::vec3 &p_p1, const glm::vec3 &p_d1, const glm::vec3 &p_d2, const glm::vec3 &p_p2,
			glm::vec4 p_col = glm::vec4(0, 0, 0, 1),
			int p_seg = 12,
			bool p_depth = true
		);

		/// @brief Draw an axis-aligned bounding box (AABB) in 3D space.
		/// @param p_aabb The axis-aligned bounding box to be drawn.
		/// @param p_col The color of the bounding box.
		/// @param p_depth Whether to enable depth testing for the bounding box.
		/// @param p_wire Whether to draw only the wireframe of the bounding box.
		static void DrawBox(
			const Math::AABB &p_aabb,
			glm::vec4 p_col = glm::vec4(0, 0, 0, 1),
			bool p_depth = true,
			bool p_wire = false
		);

		/// @brief Draw an oriented bounding box (OBB) in 3D space.
		/// @param p_obb The oriented bounding box to be drawn.
		/// @param p_col The color of the bounding box.
		/// @param p_depth Whether to enable depth testing for the bounding box.
		/// @param p_wire Whether to draw only the wireframe of the bounding box.
		static void DrawBox(
			const Math::OBB &p_obb,
			glm::vec4 p_col = glm::vec4(0, 0, 0, 1),
			bool p_depth = true,
			bool p_wire = false
		);
		
		/// @brief Draw a sphere in 3D space.
		/// @param p_sphere The sphere to be drawn.
		/// @param p_col The color of the sphere.
		/// @param p_subdiv The number of subdivisions for the sphere mesh.
		/// @param p_depth Whether to enable depth testing for the sphere.
		static void DrawSphere(
			const Math::Sphere &p_sphere,
			glm::vec4 p_col = glm::vec4(0, 0, 0, 1),
			int p_subdiv = 1,
			bool p_depth = true
		);
		
		/// @brief Draw a circle in 3D space.
		/// @param p_center The center of the circle.
		/// @param p_normal The normal vector of the circle's plane.
		/// @param p_radius The radius of the circle.
		/// @param p_col The color of the circle.
		/// @param p_seg The number of segments to divide the circle into.
		/// @param p_depth Whether to enable depth testing for the circle.
		/// @param p_wire Whether to draw only the wireframe of the circle.
		static void DrawCircle(
			const glm::vec3 &p_center, const glm::vec3 &p_normal, float p_radius,
			glm::vec4 p_col = glm::vec4(0, 0, 0, 1),
			int p_seg = 12,
			bool p_depth = true,
			bool p_wire = false
		);

		/// @brief Draw a frustum in 3D space.
		/// @param p_frustum The frustum to be drawn.
		/// @param p_col The color of the frustum.
		/// @param p_depth Whether to enable depth testing for the frustum.
		/// @param p_wire Whether to draw only the wireframe of the frustum.
		static void DrawFrustum(
			const Math::Frustum &p_frustum,
			glm::vec4 p_col = glm::vec4(0, 0, 0, 1),
			bool p_depth = true,
			bool p_wire = false
		);

		/// @brief Draw a billboarded icon in 3D space.
		/// @param p_icon The GUID of the icon to be drawn.
		/// @param p_position The position of the icon in 3D space.
		/// @param p_scale The scale of the icon.
		/// @param p_col The color of the icon.
		/// @param p_depth Whether to enable depth testing for the icon.
		static void DrawIcon(
			const GUID &p_icon, const glm::vec3 &p_position, const glm::vec2 &p_scale,
			glm::vec4 p_col = glm::vec4(0, 0, 0, 1),
			bool p_depth = true
		);


	protected:
		// Renderer is a friend class to ensure that only the Renderer can access the internal rendering methods of DebugDraw.
		friend class Droplet::Graphics::Renderer;

		/// @brief Get the singleton instance of DebugDraw
		/// @return Reference to the singleton instance of DebugDraw
		[[nodiscard]] static DebugDraw &Get()
		{
			static DebugDraw instance{};
			return instance;
		}

		/// @brief Initialize the debug draw system with the given Vulkan data and formats.
		/// @param p_instance The Vulkan instance.
		/// @param p_physicalDevice The Vulkan physical device.
		/// @param p_device The Vulkan logical device.
		/// @param p_allocator The Vulkan memory allocator.
		/// @param p_queue The Vulkan queue for command submission.
		/// @param p_commandPool The Vulkan command pool for allocating command buffers.
		/// @param p_descriptorPool The Vulkan descriptor pool for allocating descriptor sets.
		/// @param p_colorFormat The format of the color attachment.
		/// @param p_depthFormat The format of the depth attachment.
		/// @param p_framesInFlight The number of frames in flight.
		void Initialize(
			const vk::raii::Instance &p_instance,
			const vk::raii::PhysicalDevice &p_physicalDevice,
			const vk::raii::Device &p_device,
			const vma::raii::Allocator &p_allocator,
			const vk::raii::Queue &p_queue,
			const vk::raii::CommandPool &p_commandPool,
			const vk::raii::DescriptorPool &p_descriptorPool,
			vk::Format p_colorFormat,
			vk::Format p_depthFormat,
			std::uint32_t p_framesInFlight);

		/// @brief Shutdown the debug draw system and release graphics resources
		void Shutdown();

		/// @brief Generate camera-indepentent draw data
		/// @param p_frameIndex The index of the current frame in flight
		void PrepareRender(std::uint32_t p_frameIndex);

		/// @brief Generate camera-dependent draw data and render to the camera output
		/// @param p_camera The camera to render to.
		/// @param p_commandBuffer The command buffer to record draw commands into.
		/// @param p_extent The extent of the render target.
		/// @param p_frameIndex The index of the current frame in flight.
		void RenderToCamera(
			const Camera &p_camera, 
			const vk::raii::CommandBuffer &p_commandBuffer, 
			vk::Extent2D p_extent, 
			std::uint32_t p_frameIndex);

		/// @brief Clear all draw commands for the next frame
		void ClearDrawCommands();


	private:
		DebugDraw() = default;

		/// @brief Internal structure to order draws by their draw options
		struct InternalDrawOptions
		{
			glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			bool depthTest = true;
			GUID icon = 0;

			InternalDrawOptions() = default;

			InternalDrawOptions(glm::vec4 p_color, bool p_depthTest, GUID p_icon = 0)
				: color(p_color), depthTest(p_depthTest), icon(p_icon) {}

			// Comparison operators for unordered_map key
			bool operator==(const InternalDrawOptions &other) const
			{
				return icon == other.icon && color == other.color && depthTest == other.depthTest;
			}
			bool operator!=(const InternalDrawOptions &other) const
			{
				return !(*this == other);
			}
			bool operator<(const InternalDrawOptions &other) const
			{
				if (icon != other.icon)
					return icon < other.icon;
				if (depthTest != other.depthTest)
					return depthTest < other.depthTest;
				if (color.x != other.color.x)
					return color.x < other.color.x;
				if (color.y != other.color.y)
					return color.y < other.color.y;
				if (color.z != other.color.z)
					return color.z < other.color.z;
				if (color.w != other.color.w)
					return color.w < other.color.w;
				return false;
			}
			bool operator<=(const InternalDrawOptions &other) const
			{
				return *this < other || *this == other;
			}
			bool operator>(const InternalDrawOptions &other) const
			{
				return !(*this <= other);
			}
			bool operator>=(const InternalDrawOptions &other) const
			{
				return !(*this < other);
			}
		};

		struct DbgLine
		{
			glm::vec3 v1, v2;
		};

		struct DbgTri
		{
			glm::vec3 v1, v2, v3;
		};

		struct DbgIcon
		{
			glm::vec3 position;
			glm::vec2 scale;
		};

		struct Vertex
		{
			glm::vec3 position;
			glm::vec4 color;
		};

		struct IconVertex
		{
			glm::vec3 position;
			glm::vec4 color;
			glm::vec2 texCoord;
		};

		struct FrameResources
		{
			vma::raii::Buffer wireBuffer { nullptr };
			vma::raii::Buffer fillBuffer { nullptr };
			vma::raii::Buffer iconBuffer { nullptr };
			vma::raii::Buffer indexBuffer { nullptr };
			vma::raii::Buffer cameraBuffer { nullptr };
			vk::raii::DescriptorSet descriptorSet { nullptr };
			vk::DeviceSize wireCapacity = 0;
			vk::DeviceSize fillCapacity = 0;
			vk::DeviceSize iconCapacity = 0;
			vk::DeviceSize indexCapacity = 0;
			std::uint32_t wireDepthFirst = 0;
			std::uint32_t fillDepthFirst = 0;
			std::uint32_t iconDepthFirst = 0;
			std::uint32_t wireDepthCount = 0;
			std::uint32_t fillDepthCount = 0;
		};

		/// @brief Append the given lines to the mesh, converting them to vertices
		/// @param p_mesh The mesh to append to
		/// @param p_lines The lines to convert to vertices and append
		/// @param p_color The color to use for the vertices
		void AppendToMesh(std::vector<Vertex> &p_mesh, const std::vector<DbgLine> &p_lines, glm::vec4 p_color);

		/// @brief Append the given triangles to the mesh, converting them to vertices
		/// @param p_mesh The mesh to append to
		/// @param p_tris The triangles to convert to vertices and append
		/// @param p_color The color to use for the vertices
		void AppendToMesh(std::vector<Vertex> &p_mesh, const std::vector<DbgTri> &p_tris, glm::vec4 p_color);

		/// @brief Append the given icons to the mesh, converting them to vertices
		/// @param p_mesh The mesh to append to
		/// @param p_icons The icons to convert to vertices and append
		/// @param p_color The color to use for the vertices
		/// @param p_right The right direction for the icons
		/// @param p_up The up direction for the icons
		void AppendToMesh(std::vector<IconVertex> &p_mesh, const std::vector<DbgIcon> &p_icons, glm::vec4 p_color, glm::vec3 p_right, glm::vec3 p_up);

		// Draw commands for the current frame, organized by draw options.
		std::map<InternalDrawOptions, std::vector<DbgLine>>	m_wireLists{};
		std::map<InternalDrawOptions, std::vector<DbgTri>>	m_fillLists{};
		std::map<InternalDrawOptions, std::vector<DbgIcon>>	m_iconLists{};

		// Frame-generated meshes.
		std::vector<Vertex> m_wireMesh{};
		std::vector<Vertex> m_wireDepthMesh{};
		std::vector<Vertex> m_fillMesh{};
		std::vector<Vertex> m_fillDepthMesh{};
		std::map<GUID, std::vector<IconVertex>> m_iconMeshes{};
		std::map<GUID, std::vector<IconVertex>> m_iconDepthMeshes{};

		// Cached unit geometry by detail level.
		std::map<int, std::vector<DbgTri>> m_icosphereCache{};
		std::map<int, std::vector<glm::vec3>> m_circleCache{};

		// Vulkan resources and state.
		const vk::raii::Instance *m_instance = nullptr;
		const vk::raii::PhysicalDevice *m_physicalDevice = nullptr;
		const vk::raii::Device *m_device = nullptr;
		const vma::raii::Allocator *m_allocator = nullptr;
		const vk::raii::Queue *m_queue = nullptr;
		const vk::raii::CommandPool *m_commandPool = nullptr;
		const vk::raii::DescriptorPool *m_descriptorPool = nullptr;
		vk::Format m_colorFormat {};
		vk::Format m_depthFormat {}; 
		std::vector<FrameResources> m_frameResources {};
		vk::raii::DescriptorSetLayout m_descriptorSetLayout { nullptr };
		vk::raii::PipelineLayout m_pipelineLayout { nullptr };
		std::array<vk::raii::ShaderModule, 4> m_shaderModules
		{
			vk::raii::ShaderModule { nullptr },
			vk::raii::ShaderModule { nullptr },
			vk::raii::ShaderModule { nullptr },
			vk::raii::ShaderModule { nullptr }
		};
		std::array<vk::raii::Pipeline, 6> m_pipelines
		{
			vk::raii::Pipeline { nullptr },
			vk::raii::Pipeline { nullptr },
			vk::raii::Pipeline { nullptr },
			vk::raii::Pipeline { nullptr },
			vk::raii::Pipeline { nullptr },
			vk::raii::Pipeline { nullptr }
		};
	};
}
