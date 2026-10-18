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

namespace Droplet::Graphics
{
	// Forward declarations
	class Renderer;
	class Camera;
}

namespace Droplet::Graphics::Debug
{
	struct DrawOptions
	{
		glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		bool depthTest = true;
			
		// Optional
		bool wireframe = false;
		int detail = 6; // For circle & curve: number of segments. For sphere: subdivisions.
	};

	class DebugDraw
	{
	public:

		static void DrawRay(
			const Math::Ray &p_ray, 
			const DrawOptions &p_opt
		);

		static void DrawLine(
			const glm::vec3 &p_start, const glm::vec3 &p_end,
			const DrawOptions &p_opt
		);

		static void DrawTri(
			const glm::vec3 &p_v1, const glm::vec3 &p_v2, const glm::vec3 &p_v3,
			const DrawOptions &p_opt
		);

		static void DrawQuad(
			const glm::vec3 &p_center, const glm::vec3 &p_up, const glm::vec3 &p_right,
			const DrawOptions &p_opt
		);

		/// @brief Bezier curve with 1 control point
		/// @param p_p1 Start point of curve
		/// @param p_d Vector from start point to control point
		/// @param p_p2 End point of curve
		/// @param p_opt Draw options
		static void DrawCurve(
			const glm::vec3 &p_p1, const glm::vec3 &p_d, const glm::vec3 &p_p2,
			const DrawOptions &p_opt
		);

		/// @brief Bezier curve with 2 control points
		/// @param p_p1 Start point of curve
		/// @param p_d1 Vector from start point to first control point
		/// @param p_d2 Vector from end point to second control point
		/// @param p_p2 End point of curve
		/// @param p_opt Draw options
		static void DrawCurve(
			const glm::vec3 &p_p1, const glm::vec3 &p_d1, const glm::vec3 &p_d2, const glm::vec3 &p_p2,
			const DrawOptions &p_opt
		);

		static void DrawBox(
			const Math::AABB &p_aabb,
			const DrawOptions &p_opt
		);

		static void DrawBox(
			const Math::OBB &p_obb,
			const DrawOptions &p_opt
		);

		static void DrawSphere(
			const Math::Sphere &p_sphere,
			const DrawOptions &p_opt
		);

		static void DrawCircle(
			const glm::vec3 &p_center, const glm::vec3 &p_normal, float p_radius,
			const DrawOptions &p_opt
		);

		static void DrawFrustum(
			const Math::Frustum &p_frustum,
			const DrawOptions &p_opt
		);

		static void DrawIcon(
			const GUID &p_icon, const glm::vec3 &p_position, const glm::vec2 &p_scale,
			const DrawOptions &p_opt
		);


	protected:
		friend class Droplet::Graphics::Renderer;

		/// @brief Get the singleton instance of DebugDraw
		/// @return Reference to the singleton instance of DebugDraw
		[[nodiscard]] static DebugDraw &Get()
		{
			static DebugDraw instance{};
			return instance;
		}

		/// @brief Initialize the debug draw system with necessary graphics resources
		void Initialize(/* VK Resources */);

		/// @brief Generate camera-indepentent draw data
		void PrepareRender();

		/// @brief Generate camera-dependent draw data and render to the camera output
		/// @param p_camera The camera to render to
		void RenderToCamera(const Camera &p_camera);

		/// @brief Clear all draw commands for the next frame
		void ClearDrawCommands();


	private:
		DebugDraw() = default;


		struct InternalDrawOptions
		{
			glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			bool depthTest = true;
			GUID icon = 0;

			InternalDrawOptions() = default;

			InternalDrawOptions(const DrawOptions &p_opt)
				: color(p_opt.color), depthTest(p_opt.depthTest) {}

			InternalDrawOptions(const DrawOptions &p_opt, GUID p_icon)
				: color(p_opt.color), depthTest(p_opt.depthTest), icon(p_icon) {}

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

		
		std::map<InternalDrawOptions, std::vector<DbgLine>>	m_wireLists{};
		std::map<InternalDrawOptions, std::vector<DbgTri>>	m_fillLists{};
		std::map<InternalDrawOptions, std::vector<DbgIcon>>	m_iconLists{};

		// TODO: 
		// Vulkan resources
		// 
		// Wireframe pipeline
		// Wireframe depth pipeline
		// Filled pipeline
		// Filled depth pipeline
		// Icon pipeline
		// Icon depth pipeline
		// 
		// Wireframe mesh
		// Wireframe depth mesh
		// Filled mesh
		// Filled depth mesh
		// Map of icon meshes indexed by GUID
		// Map of icon depth meshes indexed by GUID
	};
}