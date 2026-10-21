#include "ScriptSystem/Bindings/DebugDrawBindings.hpp"
#include "Graphics/DebugDraw.hpp"
#include <glm/glm.hpp>
#include <math/bounds/Intersection.hpp>
#include <tracy/public/tracy/Tracy.hpp>
#include <LuaBindings.hpp>

using namespace Droplet::Script::Bindings;
using namespace Droplet::Graphics::Debug;
using namespace Droplet::Math;

void Droplet::Script::Bindings::RegisterDebugDraw(sol::state_view p_luaState)
{
    ZoneScoped;

	sol::table dbgDraw = p_luaState.create_table("DebugDraw");

	dbgDraw.set_function("DrawRay", &DebugDraw::DrawRay);
	dbgDraw.set_function("DrawLine", &DebugDraw::DrawLine);
	dbgDraw.set_function("DrawTri", &DebugDraw::DrawTri);
	dbgDraw.set_function("DrawQuad", &DebugDraw::DrawQuad);

	dbgDraw.set_function("DrawCurveQuadratic",
		[](const glm::vec3 &p1, const glm::vec3 &d, const glm::vec3 &p2, glm::vec4 col = glm::vec4(0, 0, 0, 1), int seg = 12, bool depth = true) {
			DebugDraw::DrawCurve(p1, d, p2, col, seg, depth);
		});
	dbgDraw.set_function("DrawCurveCubic",
		[](const glm::vec3 &p1, const glm::vec3 &d1, const glm::vec3 &d2, const glm::vec3 &p2, glm::vec4 col = glm::vec4(0, 0, 0, 1), int seg = 12, bool depth = true) {
			DebugDraw::DrawCurve(p1, d1, d2, p2, col, seg, depth);
		});

	dbgDraw.set_function("DrawAABB",
		[](const AABB &aabb, glm::vec4 col = glm::vec4(0, 0, 0, 1), bool depth = true, bool wire = false) {
			DebugDraw::DrawBox(aabb, col, depth, wire);
		});

	dbgDraw.set_function("DrawOBB",
		[](const OBB &obb, glm::vec4 col = glm::vec4(0, 0, 0, 1), bool depth = true, bool wire = false) {
			DebugDraw::DrawBox(obb, col, depth, wire);
		});

	dbgDraw.set_function("DrawSphere",
		[](const Sphere &sphere, glm::vec4 col = glm::vec4(0, 0, 0, 1), int subdiv = 1, bool depth = true) {
			DebugDraw::DrawSphere(sphere, col, subdiv, depth);
		});

	dbgDraw.set_function("DrawCircle", &DebugDraw::DrawCircle);
	dbgDraw.set_function("DrawFrustum", &DebugDraw::DrawFrustum);
	dbgDraw.set_function("DrawIcon", &DebugDraw::DrawIcon);

	LuaClassDefinition def;
	def.name = "DebugDraw";

	def.functions = {
		{
			"DrawRay",
			"void",
			true,
			{
				{ "ray", "Ray" },
				{ "color", "Vec4", true },
				{ "depth", "bool", true }
			}
		},
		{
			"DrawLine",
			"void",
			true,
			{
				{ "start", "Vec3" },
				{ "end", "Vec3" },
				{ "color", "Vec4", true },
				{ "depth", "bool", true }
			}
		},
		{
			"DrawTri",
			"void",
			true,
			{
				{ "p1", "Vec3" },
				{ "p2", "Vec3" },
				{ "p3", "Vec3" },
				{ "color", "Vec4", true },
				{ "depth", "bool", true },
				{ "wire", "bool", true }
			}
		},
		{
			"DrawQuad",
			"void",
			true,
			{
				{ "center", "Vec3" },
				{ "up", "Vec3" },
				{ "right", "Vec3" },
				{ "color", "Vec4", true },
				{ "depth", "bool", true },
				{ "wire", "bool", true }
			}
		},
		{
			"DrawCurveQuadratic",
			"void",
			true,
			{
				{ "p1", "Vec3" },
				{ "d", "Vec3" },
				{ "p2", "Vec3" },
				{ "color", "Vec4", true },
				{ "segments", "int", true },
				{ "depth", "bool", true }
			}
		},
		{
			"DrawCurveCubic",
			"void",
			true,
			{
				{ "p1", "Vec3" },
				{ "d1", "Vec3" },
				{ "d2", "Vec3" },
				{ "p2", "Vec3" },
				{ "color", "Vec4", true },
				{ "segments", "int", true },
				{ "depth", "bool", true }
			}
		},
		{
			"DrawAABB",
			"void",
			true,
			{
				{ "aabb", "AABB" },
				{ "color", "Vec4", true },
				{ "depth", "bool", true },
				{ "wire", "bool", true }
			}
		},
		{
			"DrawOBB",
			"void",
			true,
			{
				{ "obb", "OBB" },
				{ "color", "Vec4", true },
				{ "depth", "bool", true },
				{ "wire", "bool", true }
			}
		},
		{
			"DrawSphere",
			"void",
			true,
			{
				{ "sphere", "Sphere" },
				{ "color", "Vec4", true },
				{ "subdivisions", "int", true },
				{ "depth", "bool", true }
			}
		},
		{
			"DrawCircle",
			"void",
			true,
			{
				{ "center", "Vec3" },
				{ "normal", "Vec3" },
				{ "radius", "float" },
				{ "color", "Vec4", true },
				{ "segments", "int", true },
				{ "depth", "bool", true },
				{ "wire", "bool", true }
			}
		},
		{
			"DrawFrustum",
			"void",
			true,
			{
				{ "frustum", "Frustum" },
				{ "color", "Vec4", true },
				{ "depth", "bool", true },
				{ "wire", "bool", true }
			}
		},
		{
			"DrawIcon",
			"void",
			true,
			{
				{ "icon", "number" },
				{ "position", "Vec3" },
				{ "size", "Vec2" },
				{ "color", "Vec4", true },
				{ "depth", "bool", true }
			}
		}
	};

	LuaBindings::PushLuaClassDefinition(def);
}
