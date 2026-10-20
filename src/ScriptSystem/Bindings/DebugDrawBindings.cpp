#include "ScriptSystem/Bindings/DebugDrawBindings.hpp"
#include "Graphics/DebugDraw.hpp"
#include <glm/glm.hpp>
#include <tracy/public/tracy/Tracy.hpp>

using namespace Droplet::Script::Bindings;
using namespace Droplet::Graphics::Debug;

void Droplet::Script::Bindings::RegisterDebugDraw(sol::state_view p_luaState)
{
    ZoneScoped;

	sol::table dbgDraw = p_luaState.create_table("DebugDraw");

	dbgDraw.set_function("DrawRay",
		[](glm::vec3 origin, glm::vec3 dir, glm::vec4 col = glm::vec4(0, 0, 0, 1), bool depth = true) {
			DebugDraw::DrawRay(Math::Ray(origin, dir), col, depth);
		}
	);

	dbgDraw.set_function("DrawLine",
		[](glm::vec3 start, glm::vec3 end, glm::vec4 col = glm::vec4(0, 0, 0, 1), bool depth = true) {
			DebugDraw::DrawLine(start, end, col, depth);
		}
	);

	dbgDraw.set_function("DrawTri",
		[](const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &v3, glm::vec4 col = glm::vec4(0, 0, 0, 1), bool depth = true, bool wire = false) {
			DebugDraw::DrawTri(v1, v2, v3, col, depth, wire);
		}
	);

	dbgDraw.set_function("DrawQuad",
		[](const glm::vec3 &center, const glm::vec3 &up, const glm::vec3 &right, glm::vec4 col = glm::vec4(0, 0, 0, 1), bool depth = true, bool wire = false) {
			DebugDraw::DrawQuad(center, up, right, col, depth, wire);
		}
	);

	dbgDraw.set_function("DrawCurveQuadratic",
		[](const glm::vec3 &p1, const glm::vec3 &d, const glm::vec3 &p2, glm::vec4 col = glm::vec4(0, 0, 0, 1), int seg = 12, bool depth = true) {
			DebugDraw::DrawCurve(p1, d, p2, col, seg, depth);
		}
	);

	dbgDraw.set_function("DrawCurveCubic",
		[](const glm::vec3 &p1, const glm::vec3 &d1, const glm::vec3 &d2, const glm::vec3 &p2, glm::vec4 col = glm::vec4(0, 0, 0, 1), int seg = 12, bool depth = true) {
			DebugDraw::DrawCurve(p1, d1, d2, p2, col, seg, depth);
		}
	);

	dbgDraw.set_function("DrawAABB",
		[](const Math::AABB &aabb, glm::vec4 col = glm::vec4(0, 0, 0, 1), bool depth = true, bool wire = false) {
			DebugDraw::DrawBox(aabb, col, depth, wire);
		}
	);

	dbgDraw.set_function("DrawOBB",
		[](const Math::OBB &obb, glm::vec4 col = glm::vec4(0, 0, 0, 1), bool depth = true, bool wire = false) {
			DebugDraw::DrawBox(obb, col, depth, wire);
		}
	);

	dbgDraw.set_function("DrawSphere",
		[](const Math::Sphere &sphere, glm::vec4 col = glm::vec4(0, 0, 0, 1), int subdiv = 1, bool depth = true) {
			DebugDraw::DrawSphere(sphere, col, subdiv, depth);
		}
	);

	dbgDraw.set_function("DrawCircle",
		[](const glm::vec3 &center, const glm::vec3 &normal, float radius, glm::vec4 col = glm::vec4(0, 0, 0, 1), int seg = 12, bool depth = true, bool wire = false) {
			DebugDraw::DrawCircle(center, normal, radius, col, seg, depth, wire);
		}
	);

	dbgDraw.set_function("DrawFrustum",
		[](const Math::Frustum &frustum, glm::vec4 col = glm::vec4(0, 0, 0, 1), bool depth = true, bool wire = false) {
			DebugDraw::DrawFrustum(frustum, col, depth, wire);
		}
	);

	dbgDraw.set_function("DrawIcon",
		[](const GUID &icon, const glm::vec3 &position, const glm::vec2 &scale, glm::vec4 col = glm::vec4(0, 0, 0, 1), bool depth = true) {
			DebugDraw::DrawIcon(icon, position, scale, col, depth);
		}
	);
}
