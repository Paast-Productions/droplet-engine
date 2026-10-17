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

		[[nodiscard]] static DebugDraw &Get()
		{
			static DebugDraw instance{};
			return instance;
		}

		void Initialize(/* VK Resources */);

		void RenderToCamera(const Camera &p_camera);

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

		struct DbgSprite
		{
			glm::vec3 position;
			glm::vec2 scale;
		};
		
		std::map<InternalDrawOptions, std::vector<DbgLine>>		m_wireLists{};
		std::map<InternalDrawOptions, std::vector<DbgTri>>		m_fillLists{};
		std::map<InternalDrawOptions, std::vector<DbgSprite>>	m_spriteLists{};
	};
}