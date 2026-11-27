#pragma once
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <Math/Bounds/Ray.hpp>
#include <Math/Bounds/AABB.hpp>
#include <Math/Bounds/OBB.hpp>
#include <Math/Bounds/Sphere.hpp>
#include <Math/Bounds/Frustum.hpp>
#include <Resource/GuidUtils.hpp>
#include <Common/RenderData.hpp>

#include <array>
#include <cstdint>
#include <unordered_map>
#include <memory>

namespace Droplet
{
	class MeshResource;
}

namespace Droplet::Graphics::Debug
{
	/// @brief A utility class for rendering debug shapes in 3D space.
	/// 
	/// Handles the drawing of lines, tris, quads, curves, bounding volumes and icons.
	/// 
	/// @note The renderer collects each frame's geometry with GetFrameMeshes().
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

		/// @brief Draw a ray in 3D space.
		/// @param p_pos The origin of the ray.
		/// @param p_dir The direction of the ray.
		/// @param p_col The color of the ray.
		/// @param p_depth Whether to enable depth testing for the ray.
		static void DrawRay(
			const glm::vec3 &p_pos, 
			const glm::vec3 &p_dir,
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
		
		/// @brief Draw a wireframe sphere in 3D space.
		/// @param p_sphere The sphere to be drawn.
		/// @param p_col The color of the sphere.
		/// @param p_seg The number of segments to divide the sphere into.
		/// @param p_depth Whether to enable depth testing for the sphere.
		static void DrawWireSphere(
			const Math::Sphere &p_sphere,
			glm::vec4 p_col = glm::vec4(0, 0, 0, 1),
			int p_seg = 12,
			bool p_depth = true
		);

		/// @brief Draw an arc in 3D space.
		/// @param p_center The center of the arc.
		/// @param p_normal The normal vector of the arc's plane.
		/// @param p_centerToStart Vector from the center to the start point of the arc.
		/// @param p_radius The radius of the arc.
		/// @param p_angle The angle of the arc in degrees.
		/// @param p_col The color of the arc.
		/// @param p_seg The number of segments to divide the arc into.
		/// @param p_depth Whether to enable depth testing for the arc.
		/// @param p_wire Whether to draw only the wireframe of the arc.
		static void DrawArc(
			const glm::vec3 &p_center, const glm::vec3 &p_normal, const glm::vec3 &p_centerToStart, 
			float p_radius, float p_angle, 
			glm::vec4 p_col = glm::vec4(0, 0, 0, 1),
			int p_seg = 12,
			bool p_depth = true,
			bool p_wire = false

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

		/// @brief Draw a mesh resource in 3D space, transformed by a world matrix.
		/// @param p_mesh The mesh to be drawn. Its vertex layout must contain a POSITION attribute.
		/// @param p_world The world matrix that transforms the mesh from local to world space.
		/// @param p_col The color of the mesh.
		/// @param p_depth Whether to enable depth testing for the mesh.
		/// @param p_wire Whether to draw only the wireframe of the mesh.
		static void DrawMesh(
			const MeshResource &p_mesh,
			const glm::mat4 &p_world,
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


		/**
		 * @brief This frame's batched debug geometry as dynamic meshes, then clears the draw commands
		 * @returns The non-empty batches: lines and triangles, each with and without depth test
		 */
		[[nodiscard]] static std::vector<std::shared_ptr<const Common::DynamicMeshData>> GetFrameMeshes();

	protected:

		/// @brief Get the singleton instance of DebugDraw
		/// @return Reference to the singleton instance of DebugDraw
		[[nodiscard]] static DebugDraw &Get()
		{
			static DebugDraw instance{};
			return instance;
		}

		/// @brief Builds this frame's line and triangle meshes from the draw commands
		void PrepareRender();

		/// @brief Clear all draw commands for the next frame
		void ClearDrawCommands();


	private:
		DebugDraw() = default;

		/// @brief Internal structure to order draws by their draw options
		struct InternalDrawOptions
		{
			bool depthTest = true;
			GUID icon = 0;

			InternalDrawOptions() = default;

			InternalDrawOptions(bool p_depthTest, GUID p_icon = 0)
				: depthTest(p_depthTest), icon(p_icon) {}

			// Comparison operators for unordered_map key
			bool operator==(const InternalDrawOptions &other) const
			{
				return icon == other.icon && depthTest == other.depthTest;
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
			glm::vec4 color;
		};

		struct DbgTri
		{
			glm::vec3 v1, v2, v3;
			glm::vec4 color;
		};

		struct DbgIcon
		{
			glm::vec3 position;
			glm::vec2 scale;
			glm::vec4 color;
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
		void AppendToMesh(std::vector<Vertex> &p_mesh, const std::vector<DbgLine> &p_lines);

		/// @brief Append the given triangles to the mesh, converting them to vertices
		/// @param p_mesh The mesh to append to
		/// @param p_tris The triangles to convert to vertices and append
		void AppendToMesh(std::vector<Vertex> &p_mesh, const std::vector<DbgTri> &p_tris);

		/// @brief Append the given icons to the mesh, converting them to vertices
		/// @param p_mesh The mesh to append to
		/// @param p_icons The icons to convert to vertices and append
		/// @param p_right The right direction for the icons
		/// @param p_up The up direction for the icons
		void AppendToMesh(std::vector<IconVertex> &p_mesh, const std::vector<DbgIcon> &p_icons, glm::vec3 p_right, glm::vec3 p_up);

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
	};
}
