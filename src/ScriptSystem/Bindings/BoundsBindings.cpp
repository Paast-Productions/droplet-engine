#include "ScriptSystem/Bindings/BoundsBindings.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <math/bounds/Intersection.hpp>
#include <tracy/public/tracy/Tracy.hpp>
#include <LuaBindings.hpp>

using namespace Droplet::Script::Bindings;
using namespace Droplet::Math;

void Droplet::Script::Bindings::RegisterBounds(sol::state_view p_luaState)
{
	ZoneScoped;

	// Ray
	{
		p_luaState.new_usertype<Ray>(
			"Ray",
			sol::constructors<
				Ray(), 
				Ray(const glm::vec3 &, const glm::vec3 &)>(),
			"pos", &Ray::pos,
			"dir", &Ray::dir,
			"GetLength", &Ray::GetLength,
			"GetEndPoint", &Ray::GetEndPoint,
			"Transform", [&](const Ray &ray, const glm::mat4x4 &transform) { return Ray::Transform(ray, transform); }
		);

		LuaClassDefinition def;
		def.name = "Ray";

		def.properties = {
			{ "pos", "Vec3" },
			{ "dir", "Vec3" }
		};

		def.functions = {
			{
				"GetLength",
				"number",
				{ }
			},
			{
				"GetEndPoint",
				"Vec3",
				{ }
			},
			{
				"Transform",
				"Ray",
				{
					{ "transform", "Mat4" }
				}
			}
		};

		LuaBindings::PushLuaClassDefinition(def);
	}

	// Plane
	{
		p_luaState.new_usertype<Plane>(
			"Plane",
			sol::constructors<
				Plane(), 
				Plane(const glm::vec3 &, float), 
				Plane(const glm::vec3 &, const glm::vec3 &), 
				Plane(const glm::vec3 &, const glm::vec3 &, const glm::vec3 &)>(),
			"normal", &Plane::normal,
			"distance", &Plane::distance,
			"GetOrigin", &Plane::GetOrigin,
			"IsBeneath", &Plane::IsBeneath
		);

		LuaClassDefinition def;
		def.name = "Plane";

		def.properties = {
			{ "normal", "Vec3" },
			{ "distance", "number" }
		};

		def.functions = {
			{
				"GetOrigin",
				"Vec3",
				{ }
			},
			{
				"IsBeneath",
				"bool",
				{
					{ "point", "Vec3" }
				}
			}
		};

		LuaBindings::PushLuaClassDefinition(def);
	}

	// AABB
	{
		p_luaState.new_usertype<AABB>(
			"AABB",
			sol::constructors<
				AABB(),
				AABB(const glm::vec3 &, const glm::vec3 &)>(),
			"center", &AABB::center,
			"extents", &AABB::extents,
			"GetSize", &AABB::GetSize,
			"GetMin", &AABB::GetMin,
			"GetMax", &AABB::GetMax,
			"Transform", [&](const AABB &aabb, const glm::mat4x4 &transform) { return AABB::Transform(aabb, transform); }
		);

		LuaClassDefinition def;
		def.name = "AABB";

		def.properties = {
			{ "center", "Vec3" },
			{ "extents", "Vec3" }
		};

		def.functions = {
			{
				"GetSize",
				"Vec3",
				{ }
			},
			{
				"GetMin",
				"Vec3",
				{ }
			},
			{
				"GetMax",
				"Vec3",
				{ }
			},
			{
				"Transform",
				"AABB",
				{
					{ "transform", "Mat4" }
				}
			}
		};

		LuaBindings::PushLuaClassDefinition(def);
	}

	// OBB
	{
		p_luaState.new_usertype<OBB>(
			"OBB",
			sol::constructors<
				OBB(),
				OBB(const glm::vec3 &, const glm::vec3 &, const glm::mat3x3 &),
				OBB(const glm::vec3 &, const glm::vec3 &, const glm::quat &)>(),
			"center", &OBB::center,
			"extents", &OBB::extents,
			"orientation", &OBB::orientation,
			"GetSize", &OBB::GetSize,
			"GetCorners", &OBB::GetCorners,
			"Transform", [&](const OBB &obb, const glm::mat4x4 &transform) { return OBB::Transform(obb, transform); }
		);

		LuaClassDefinition def;
		def.name = "OBB";

		def.properties = {
			{ "center", "Vec3" },
			{ "extents", "Vec3" },
			{ "orientation", "Mat3" }
		};

		def.functions = {
			{
				"GetSize",
				"Vec3",
				{ }
			},
			{
				"GetCorners",
				"table",
				{ }
			},
			{
				"Transform",
				"OBB",
				{
					{ "transform", "Mat4" }
				}
			}
		};
	}

	// Sphere
	{
		p_luaState.new_usertype<Sphere>(
			"Sphere",
			sol::constructors<
				Sphere(),
				Sphere(const glm::vec3 &, float)>(),
			"center", &Sphere::center,
			"radius", &Sphere::radius
		);

		LuaClassDefinition def;
		def.name = "Sphere";

		def.properties = {
			{ "center", "Vec3" },
			{ "radius", "number" }
		};

		LuaBindings::PushLuaClassDefinition(def);
	}

	// Frustum
	{
		p_luaState.new_usertype<Frustum>(
			"Frustum",
			sol::constructors<
				Frustum(const Plane &, const Plane &, const Plane &, const Plane &, const Plane &, const Plane &),
				Frustum(const glm::vec3 &, const glm::vec3 &, const glm::vec3 &, float, float, float, float)>(),
			"planes", &Frustum::planes,
			"GetPlane", &Frustum::GetPlane,
			"GetLeft", &Frustum::GetLeft,
			"GetRight", &Frustum::GetRight,
			"GetTop", &Frustum::GetTop,
			"GetBottom", &Frustum::GetBottom,
			"GetNear", &Frustum::GetNear,
			"GetFar", &Frustum::GetFar
		);

		LuaClassDefinition def;
		def.name = "Frustum";

		def.properties = {
			{ "planes", "table" }
		};

		def.functions = {
			{
				"GetPlane",
				"Plane",
				{
					{ "index", "number" }
				}
			},
			{
				"GetLeft",
				"Plane",
				{ }
			},
			{
				"GetRight",
				"Plane",
				{ }
			},
			{
				"GetTop",
				"Plane",
				{ }
			},
			{
				"GetBottom",
				"Plane",
				{ }
			},
			{
				"GetNear",
				"Plane",
				{ }
			},
			{
				"GetFar",
				"Plane",
				{ }
			}
		};

		LuaBindings::PushLuaClassDefinition(def);
	}

	// IntersectType
	{
		p_luaState.new_enum<IntersectType>(
			"IntersectType",
			{
				{ "None", IntersectType::None },
				{ "Intersects", IntersectType::Intersects },
				{ "Contains", IntersectType::Contains }
			}
		);

		LuaEnumDefinition def;
		def.name = "IntersectType";

		def.flags = {
			{ "None", (std::size_t)IntersectType::None },
			{ "Intersects", (std::size_t)IntersectType::Intersects },
			{ "Contains", (std::size_t)IntersectType::Contains }
		};

		LuaBindings::PushLuaEnumDefinition(def);
	}

	// RayHit
	{
		p_luaState.new_usertype<RayHit>(
			"RayHit",
			sol::constructors<RayHit(const glm::vec3 &, const glm::vec3 &, float, bool)>(),
			"point", &RayHit::point,
			"normal", &RayHit::normal,
			"distance", &RayHit::distance,
			"didHit", &RayHit::didHit
		);

		LuaClassDefinition def;
		def.name = "RayHit";

		def.properties = {
			{ "point", "Vec3" },
			{ "normal", "Vec3" },
			{ "distance", "number" },
			{ "didHit", "bool" }
		};

		LuaBindings::PushLuaClassDefinition(def);
	}

	// Intersection Functions
	{
		sol::table intersection = p_luaState.create_table("Intersection");

		intersection.set_function("RaycastPlane",
			[](const Ray &ray, const Plane &plane) {
				return Raycast(ray, plane);
			});

		intersection.set_function("RaycastAABB",
			[](const Ray &ray, const AABB &aabb) {
				return Raycast(ray, aabb);
			});

		intersection.set_function("RaycastOBB",
			[](const Ray &ray, const OBB &obb) {
				return Raycast(ray, obb);
			});

		intersection.set_function("RaycastSphere",
			[](const Ray &ray, const Sphere &sphere) {
				return Raycast(ray, sphere);
			});

		intersection.set_function("RaycastFrustum",
			[](const Ray &ray, const Frustum &frustum) {
				return Raycast(ray, frustum);
			});

		intersection.set_function("AABBContains",
			[](const AABB &aabb, const glm::vec3 &point) {
				return Contains(aabb, point);
			});

		intersection.set_function("OBBContains",
			[](const OBB &obb, const glm::vec3 &point) {
				return Contains(obb, point);
			});

		intersection.set_function("SphereContains",
			[](const Sphere &sphere, const glm::vec3 &point) {
				return Contains(sphere, point);
			});

		intersection.set_function("FrustumContains",
			[](const Frustum &frustum, const glm::vec3 &point) {
				return Contains(frustum, point);
			});

		intersection.set_function("AABBIntersectsPlane",
			[](const AABB &a, const Plane &b) {
				return Intersects(a, b);
			});

		intersection.set_function("AABBIntersectsAABB",
			[](const AABB &a, const AABB &b) {
				return Intersects(a, b);
			});

		intersection.set_function("AABBIntersectsOBB",
			[](const AABB &a, const OBB &b) {
				return Intersects(a, b);
			});

		intersection.set_function("AABBIntersectsSphere",
			[](const AABB &a, const Sphere &b) {
				return Intersects(a, b);
			});

		intersection.set_function("AABBIntersectsFrustum",
			[](const AABB &a, const Frustum &b) {
				return Intersects(a, b);
			});

		intersection.set_function("OBBIntersectsPlane",
			[](const OBB &a, const Plane &b) {
				return Intersects(a, b);
			});

		intersection.set_function("OBBIntersectsAABB",
			[](const OBB &a, const AABB &b) {
				return Intersects(a, b);
			});

		intersection.set_function("OBBIntersectsOBB",
			[](const OBB &a, const OBB &b) {
				return Intersects(a, b);
			});

		intersection.set_function("OBBIntersectsSphere",
			[](const OBB &a, const Sphere &b) {
				return Intersects(a, b);
			});

		intersection.set_function("OBBIntersectsFrustum",
			[](const OBB &a, const Frustum &b) {
				return Intersects(a, b);
			});

		intersection.set_function("SphereIntersectsPlane",
			[](const Sphere &a, const Plane &b) {
				return Intersects(a, b);
			});

		intersection.set_function("SphereIntersectsAABB",
			[](const Sphere &a, const AABB &b) {
				return Intersects(a, b);
			});

		intersection.set_function("SphereIntersectsOBB",
			[](const Sphere &a, const OBB &b) {
				return Intersects(a, b);
			});

		intersection.set_function("SphereIntersectsSphere",
			[](const Sphere &a, const Sphere &b) {
				return Intersects(a, b);
			});

		intersection.set_function("SphereIntersectsFrustum",
			[](const Sphere &a, const Frustum &b) {
				return Intersects(a, b);
			});

		intersection.set_function("FrustumIntersectsPlane",
			[](const Frustum &a, const Plane &b) {
				return Intersects(a, b);
			});

		intersection.set_function("FrustumIntersectsAABB",
			[](const Frustum &a, const AABB &b) {
				return Intersects(a, b);
			});

		intersection.set_function("FrustumIntersectsOBB",
			[](const Frustum &a, const OBB &b) {
				return Intersects(a, b);
			});

		intersection.set_function("FrustumIntersectsSphere",
			[](const Frustum &a, const Sphere &b) {
				return Intersects(a, b);
			});

		intersection.set_function("FrustumIntersectsFrustum",
			[](const Frustum &a, const Frustum &b) {
				return Intersects(a, b);
			});

		LuaClassDefinition def;
		def.name = "Intersection";

		def.functions = {
			// Raycast
			{
				"RaycastPlane",
				"RayHit",
				true,
				{
					{ "ray", "Ray" },
					{ "plane", "Plane" }
				}
			},
			{
				"RaycastAABB",
				"RayHit",
				true,
				{
					{ "ray", "Ray" },
					{ "aabb", "AABB" }
				}
			},
			{
				"RaycastOBB",
				"RayHit",
				true,
				{
					{ "ray", "Ray" },
					{ "obb", "OBB" }
				}
			},
			{
				"RaycastSphere",
				"RayHit",
				true,
				{
					{ "ray", "Ray" },
					{ "sphere", "Sphere" }
				}
			},
			{
				"RaycastFrustum",
				"RayHit",
				true,
				{
					{ "ray", "Ray" },
					{ "frustum", "Frustum" }
				}
			},

			// Contains
			{
				"AABBContains",
				"bool",
				true,
				{
					{ "aabb", "AABB" },
					{ "point", "Vec3" }
				}
			},
			{
				"OBBContains",
				"bool",
				true,
				{
					{ "obb", "OBB" },
					{ "point", "Vec3" }
				}
			},
			{
				"SphereContains",
				"bool",
				true,
				{
					{ "sphere", "Sphere" },
					{ "point", "Vec3" }
				}
			},
			{
				"FrustumContains",
				"bool",
				true,
				{
					{ "frustum", "Frustum" },
					{ "point", "Vec3" }
				}
			},

			// AABB Intersects
			{
				"AABBIntersectsPlane",
				"IntersectType",
				true,
				{
					{ "aabb", "AABB" },
					{ "plane", "Plane" }
				}
			},
			{
				"AABBIntersectsAABB",
				"IntersectType",
				true,
				{
					{ "aabb1", "AABB" },
					{ "aabb2", "AABB" }
				}
			},
			{
				"AABBIntersectsOBB",
				"IntersectType",
				true,
				{
					{ "aabb", "AABB" },
					{ "obb", "OBB" }
				}
			},
			{
				"AABBIntersectsSphere",
				"IntersectType",
				true,
				{
					{ "aabb", "AABB" },
					{ "sphere", "Sphere" }
				}
			},
			{
				"AABBIntersectsFrustum",
				"IntersectType",
				true,
				{
					{ "aabb", "AABB" },
					{ "frustum", "Frustum" }
				}
			},

			// OBB Intersects
			{
				"OBBIntersectsPlane",
				"IntersectType",
				true,
				{
					{ "obb", "OBB" },
					{ "plane", "Plane" }
				}
			},
			{
				"OBBIntersectsAABB",
				"IntersectType",
				true,
				{
					{ "obb", "OBB" },
					{ "aabb", "AABB" }
				}
			},
			{
				"OBBIntersectsOBB",
				"IntersectType",
				true,
				{
					{ "obb1", "OBB" },
					{ "obb2", "OBB" }
				}
			},
			{
				"OBBIntersectsSphere",
				"IntersectType",
				true,
				{
					{ "obb", "OBB" },
					{ "sphere", "Sphere" }
				}
			},
			{
				"OBBIntersectsFrustum",
				"IntersectType",
				true,
				{
					{ "obb", "OBB" },
					{ "frustum", "Frustum" }
				}
			},

			// Sphere Intersects
			{
				"SphereIntersectsPlane",
				"IntersectType",
				true,
				{
					{ "sphere", "Sphere" },
					{ "plane", "Plane" }
				}
			},
			{
				"SphereIntersectsAABB",
				"IntersectType",
				true,
				{
					{ "sphere", "Sphere" },
					{ "aabb", "AABB" }
				}
			},
			{
				"SphereIntersectsOBB",
				"IntersectType",
				true,
				{
					{ "sphere", "Sphere" },
					{ "obb", "OBB" }
				}
			},
			{
				"SphereIntersectsSphere",
				"IntersectType",
				true,
				{
					{ "sphere1", "Sphere" },
					{ "sphere2", "Sphere" }
				}
			},
			{
				"SphereIntersectsFrustum",
				"IntersectType",
				true,
				{
					{ "sphere", "Sphere" },
					{ "frustum", "Frustum" }
				}
			},

			// Frustum Intersects
			{
				"FrustumIntersectsPlane",
				"IntersectType",
				true,
				{
					{ "frustum", "Frustum" },
					{ "plane", "Plane" }
				}
			},
			{
				"FrustumIntersectsAABB",
				"IntersectType",
				true,
				{
					{ "frustum", "Frustum" },
					{ "aabb", "AABB" }
				}
			},
			{
				"FrustumIntersectsOBB",
				"IntersectType",
				true,
				{
					{ "frustum", "Frustum" },
					{ "obb", "OBB" }
				}
			},
			{
				"FrustumIntersectsSphere",
				"IntersectType",
				true,
				{
					{ "frustum", "Frustum" },
					{ "sphere", "Sphere" }
				}
			},
			{
				"FrustumIntersectsFrustum",
				"IntersectType",
				true,
				{
					{ "frustum1", "Frustum" },
					{ "frustum2", "Frustum" }
				}
			}
		};

		LuaBindings::PushLuaClassDefinition(def);
	}
}