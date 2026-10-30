#include "LuaBindings.hpp"
#include <Node.hpp>
#include <Transform.hpp>
#include <glm/glm.hpp>
#include <GameInput.hpp>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_video.h>
#include <ImGui/imgui.h>
#include <ImGui/imgui_stdlib.h>
#include <DebugDrawBindings.hpp>
#include <BoundsBindings.hpp>
#include <SceneBindings.hpp>
#include <tracy/public/tracy/Tracy.hpp>
#include <tracy/public/tracy/TracyLua.hpp>
#include <Component.hpp>
#include <HealthComponent.hpp>
#include <PlayerComponent.hpp>
#include <Behaviour.hpp>
#include <SceneSystem/Behaviours/PlayerBehaviour.hpp>

#include <print>
#include <filesystem>
#include <tuple>
#include <algorithm>
#include <vector>

using namespace Droplet::Script;
using namespace Droplet::Script::Bindings;

std::vector<LuaGlobalFunctionDefinition> LuaBindings::m_luaGlobalDefinitions;
std::vector<LuaClassDefinition> LuaBindings::m_luaClassDefinitions;
std::vector<LuaEnumDefinition> LuaBindings::m_luaEnumDefinitions;

void LuaBindings::RegisterBindings(sol::state_view p_luaState, Droplet::Scene::SceneManager &p_sceneManager)
{
	ZoneScoped;

	RegisterGlobalFunctions();
	RegisterNode(p_luaState);
	RegisterTransform(p_luaState);
	RegisterGLM(p_luaState);
	RegisterInput(p_luaState);
	RegisterImGui(p_luaState);
	RegisterTracy(p_luaState);
	RegisterBounds(p_luaState);
	RegisterDebugDraw(p_luaState);
	RegisterScenes(p_luaState, p_sceneManager);
	RegisterHealthComponent(p_luaState);
	RegisterPlayerComponent(p_luaState);
	RegisterPlayerBehaviour(p_luaState);

	std::filesystem::path scriptDirectory = std::filesystem::current_path()
		/ ".." / ".." / ".." / "src" / "Scripts";

	std::filesystem::path path = scriptDirectory / "LuaHelper.d.lua";
	LuaApiGenerator::Generate(path, m_luaGlobalDefinitions, m_luaClassDefinitions, m_luaEnumDefinitions);
}

void LuaBindings::RegisterGlobalFunctions()
{
	m_luaGlobalDefinitions =
	{
		{
			"OnStart",
			"void",
			{}
		},
		{
			"OnUpdate",
			"void",
			{
				{ "dt", "number" }
			} 
		},
		{
			"OnTrigger",
			"void",
			{
				{ "dt", "number" }
			} 
		}
	};
}
 
void LuaBindings::RegisterNode(sol::state_view p_luaState)
{
	ZoneScoped;

	p_luaState.new_usertype<Scene::Node>(
		"Node",
		"GetName", &Scene::Node::GetName,
		"GetTransform",
		static_cast<Scene::Transform & (Scene::Node:: *)()>(
			&Scene::Node::GetTransform),
		"GetHealthComponent", &Scene::Node::GetHealthComponent,
		"GetPlayerComponent", &Scene::Node::GetPlayerComponent
	);

	LuaClassDefinition classDef;
	classDef.name = "Node";

	classDef.functions =
	{
		{
			"GetName",
			"string",
			{}
		},
		{
			"GetTransform",
			"Transform",
			{}
		},
		{
			"GetHealthComponent",
			"HealthComponent",
			{}
		},
		{
			"GetPlayerComponent",
			"PlayerComponent",
			{}
		}
	};

	LuaBindings::m_luaClassDefinitions.push_back(classDef);
}

void LuaBindings::RegisterTransform(sol::state_view p_luaState)
{
	ZoneScoped;

	using namespace Droplet::Scene;
	typedef sol::optional<Transform::Space> OptSpace;
	constexpr Transform::Space TSLocal = Transform::Space::Local;
	constexpr Transform::Space TSWorld = Transform::Space::World;

	p_luaState.new_enum<Transform::Space>(
		"TransformSpace",
		{
			{"Local", TSLocal},
			{"World", TSWorld}
		}
	);

	LuaEnumDefinition transformSpace;
	transformSpace.name = "TransformSpace";

	transformSpace.flags =
	{
		{ "Local", (std::size_t)TSLocal },
		{ "World", (std::size_t)TSWorld }
	};

	LuaBindings::m_luaEnumDefinitions.push_back(transformSpace);

	p_luaState.new_usertype<Transform>(
		"Transform",
		"GetPosition", [&](Transform &transform, OptSpace space) { return transform.GetPosition(space.value_or(TSLocal)); },
		"GetRotation", [&](Transform &transform, OptSpace space) { return transform.GetRotation(space.value_or(TSLocal)); },
		"GetEuler", [&](Transform &transform, OptSpace space) { return transform.GetEuler(space.value_or(TSLocal)); },
		"GetScale", &Transform::GetScale,
		"GetMatrix", [&](Transform &transform, OptSpace space) { return transform.GetMatrix(space.value_or(TSLocal)); },
		"IsDirty", &Transform::IsDirty,
		"GetUp", [&](Transform &transform, OptSpace space) { return transform.GetUp(space.value_or(TSLocal)); },
		"GetRight", [&](Transform &transform, OptSpace space) { return transform.GetRight(space.value_or(TSLocal)); },
		"GetForward", [&](Transform &transform, OptSpace space) { return transform.GetForward(space.value_or(TSLocal)); },
		"SetPosition", [&](Transform &transform, const glm::vec3 &position, OptSpace space) { transform.SetPosition(position, space.value_or(TSLocal)); },
		"SetRotation", [&](Transform &transform, const glm::quat &rotation, OptSpace space) { transform.SetRotation(rotation, space.value_or(TSLocal)); },
		"SetEuler", [&](Transform &transform, const glm::vec3 &euler, OptSpace space) { transform.SetEuler(euler, space.value_or(TSLocal)); },
		"SetScale", &Transform::SetScale,
		"SetMatrix", [&](Transform &transform, const glm::mat4 &matrix, OptSpace space) { transform.SetMatrix(matrix, space.value_or(TSLocal)); },
		"MakeDirty", &Transform::MakeDirty,
		"RecalculateMatrices", &Transform::RecalculateMatrices,
		"Move", [&](Transform &transform, const glm::vec3 &amount, OptSpace space) { transform.Move(amount, space.value_or(TSLocal)); },
		"Rotate", [&](Transform &transform, const glm::quat &rotation, OptSpace space) { transform.Rotate(rotation, space.value_or(TSLocal)); },
		"RotateEuler", [&](Transform &transform, const glm::vec3 &euler, OptSpace space) { transform.RotateEuler(euler, space.value_or(TSLocal)); },
		"SlerpRotation", [&](Transform &transform, const glm::quat &target, float t, OptSpace space) { transform.SlerpRotation(target, t, space.value_or(TSLocal)); },
		"MoveRelative", &Transform::MoveRelative,
		"RotateEulerRelative", &Transform::RotateEulerRelative,
		"AddScale", &Transform::AddScale,
		"RotateAxis", [&](Transform &transform, float angle, const glm::vec3 &axis, OptSpace space) { transform.RotateAxis(angle, axis, space.value_or(TSLocal)); },
		"LookAt", [&](Transform &transform, const glm::vec3 &target, sol::optional<glm::vec3> up, OptSpace space) { transform.LookAt(target, up.value_or(glm::vec3(0.0f, 1.0f, 0.0f)), space.value_or(TSLocal)); }
	);

	LuaClassDefinition transform;
	transform.name = "Transform";

	transform.functions =
	{
		{
			"GetPosition",
			"Vec3",
			{
				{ "space", "TransformSpace", true }
			}
		},
		{
			"GetRotation",
			"Quat",
			{
				{ "space", "TransformSpace", true }
			}
		},
		{
			"GetEuler",
			"Vec3",
			{
				{ "space", "TransformSpace", true }
			}
		},
		{
			"GetScale",
			"Vec3",
			{}
		},
		{
			"GetMatrix",
			"Mat4",
			{
				{ "space", "TransformSpace", true }
			}
		},
		{
			"IsDirty",
			"boolean",
			{}
		},
		{
			"GetUp",
			"Vec3",
			{
				{ "space", "TransformSpace", true }
			}
		},
		{
			"GetRight",
			"Vec3",
			{
				{ "space", "TransformSpace", true }
			}
		},
		{
			"GetForward",
			"Vec3",
			{
				{ "space", "TransformSpace", true }
			}
		},
		{
			"SetPosition",
			"void",
			{
				{ "position", "Vec3" },
				{ "space", "TransformSpace", true }
			}
		},
		{
			"SetRotation",
			"void",
			{
				{ "rotation", "Quat" },
				{ "space", "TransformSpace", true }
			}
		},
		{
			"SetEuler",
			"void",
			{
				{ "euler", "Vec3" },
				{ "space", "TransformSpace", true }
			}
		},
		{
			"SetScale",
			"void",
			{
				{ "scale", "Vec3" }
			}
		},
		{
			"SetMatrix",
			"void",
			{
				{ "matrix", "Mat4" },
				{ "space", "TransformSpace", true }
			}
		},
		{
			"MakeDirty",
			"void",
			{}
		},
		{
			"RecalculateMatrices",
			"void",
			{}
		},
		{
			"Move",
			"void",
			{
				{ "amount", "Vec3" },
				{ "space", "TransformSpace", true }
			}
		},
		{
			"Rotate",
			"void",
			{
				{ "rotation", "Quat" },
				{ "space", "TransformSpace", true }
			}
		},
		{
			"SlerpRotation",
			"void",
			{
				{ "target", "Quat" },
				{ "t", "number" },
				{ "space", "TransformSpace", true }
			}
		},
		{
			"MoveRelative",
			"void",
			{
				{ "amount", "Vec3" }
			}
		},
		{
			"RotateEulerRelative",
			"void",
			{
				{ "euler", "Vec3" }
			}
		},
		{
			"RotateEuler",
			"void",
			{
				{ "euler", "Vec3" },
				{ "space", "TransformSpace", true }
			}
		},
		{
			"AddScale",
			"void",
			{
				{ "scale", "Vec3" }
			}
		},
		{
			"RotateAxis",
			"void",
			{
				{ "angle", "number" },
				{ "axis", "Vec3" },
				{ "space", "TransformSpace", true }
			}
		},
		{
			"LookAt",
			"void",
			{
				{ "target", "Vec3" },
				{ "up", "Vec3", true },
				{ "space", "TransformSpace", true }
			}
		}
	};

	LuaBindings::m_luaClassDefinitions.push_back(transform);
}

void LuaBindings::RegisterGLM(sol::state_view p_luaState)
{
	ZoneScoped;

	{
		p_luaState.new_usertype<glm::vec2>(
			"Vec2",
			sol::constructors<glm::vec2(float, float)>(),
			"x", &glm::vec2::x,
			"y", &glm::vec2::y
		);

		p_luaState.new_usertype<glm::vec3>(
			"Vec3",
			sol::constructors<glm::vec3(float, float, float)>(),
			"x", &glm::vec3::x,
			"y", &glm::vec3::y,
			"z", &glm::vec3::z
		);

		p_luaState.new_usertype<glm::vec4>(
			"Vec4",
			sol::constructors<glm::vec4(float, float, float, float)>(),
			"x", &glm::vec4::x,
			"y", &glm::vec4::y,
			"z", &glm::vec4::z,
			"w", &glm::vec4::w
		);

		p_luaState.new_usertype<glm::quat>(
			"Quat",
			sol::constructors<glm::quat(float, float, float, float)>(),
			"w", &glm::quat::w,
			"x", &glm::quat::x,
			"y", &glm::quat::y,
			"z", &glm::quat::z
		);

		p_luaState.new_usertype<glm::mat3>(
			"Mat3",
			sol::constructors<glm::mat3(float)>()
		);

		p_luaState.new_usertype<glm::mat4>(
			"Mat4",
			sol::constructors<glm::mat4(float)>()
		);
	}

	{
		LuaClassDefinition vec2Def;
		vec2Def.name = "Vec2";

		vec2Def.properties =
		{
			{ "x", "number" },
			{ "y", "number" }
		};

		LuaBindings::m_luaClassDefinitions.push_back(vec2Def);
	}

	{
		LuaClassDefinition vec3Def;
		vec3Def.name = "Vec3";

		vec3Def.properties =
		{
			{ "x", "number" },
			{ "y", "number" },
			{ "z", "number" }
		};

		LuaBindings::m_luaClassDefinitions.push_back(vec3Def);
	}


	{
		LuaClassDefinition vec4Def;
		vec4Def.name = "Vec4";

		vec4Def.properties =
		{
			{ "x", "number" },
			{ "y", "number" },
			{ "z", "number" },
			{ "w", "number" }
		};

		LuaBindings::m_luaClassDefinitions.push_back(vec4Def);
	}

	{
		LuaClassDefinition quatDef;
		quatDef.name = "Quat";

		quatDef.properties =
		{
			{ "w", "number" },
			{ "x", "number" },
			{ "y", "number" },
			{ "z", "number" }
		};

		LuaBindings::m_luaClassDefinitions.push_back(quatDef);
	}

	{
		LuaClassDefinition mat3Def;
		mat3Def.name = "Mat3";

		LuaBindings::m_luaClassDefinitions.push_back(mat3Def);

		LuaClassDefinition mat4Def;
		mat4Def.name = "Mat4";

		LuaBindings::m_luaClassDefinitions.push_back(mat4Def);
	}


	sol::table glm = p_luaState.create_table("GLM");

	// Vec2
	{
		glm.set_function("V2Abs",
			[](glm::vec2 p_vec) {
				return glm::abs(p_vec);
			});

		glm.set_function("V2Sign",
			[](glm::vec2 p_vec) {
				return glm::sign(p_vec);
			});

		glm.set_function("V2Floor",
			[](glm::vec2 p_vec) {
				return glm::floor(p_vec);
			});

		glm.set_function("V2Trunc",
			[](glm::vec2 p_vec) {
				return glm::trunc(p_vec);
			});

		glm.set_function("V2Round",
			[](glm::vec2 p_vec) {
				return glm::round(p_vec);
			});

		glm.set_function("V2RoundEven",
			[](glm::vec2 p_vec) {
				return glm::roundEven(p_vec);
			});

		glm.set_function("V2Ceil",
			[](glm::vec2 p_vec) {
				return glm::ceil(p_vec);
			});

		glm.set_function("V2Fract",
			[](glm::vec2 p_vec) {
				return glm::fract(p_vec);
			});

		glm.set_function("V2Mod",
			[](glm::vec2 p_vec1, glm::vec2 p_vec2) {
				return glm::mod(p_vec1, p_vec2);
			});

		glm.set_function("V2ModF",
			[](glm::vec2 p_vec1, float p_float) {
				return glm::mod(p_vec1, p_float);
			});

		glm.set_function("V2Min",
			[](glm::vec2 p_vec1, glm::vec2 p_vec2) {
				return glm::min(p_vec1, p_vec2);
			});

		glm.set_function("V2Max",
			[](glm::vec2 p_vec1, glm::vec2 p_vec2) {
				return glm::max(p_vec1, p_vec2);
			});

		glm.set_function("V2Clamp",
			[](glm::vec2 p_vec, glm::vec2 p_min, glm::vec2 p_max) {
				return glm::clamp(p_vec, p_min, p_max);
			});

		glm.set_function("V2AddF",
			[](glm::vec2 p_vec1, float p_float) {
				return p_vec1 + p_float;
			});

		glm.set_function("V2SubF",
			[](glm::vec2 p_vec1, float p_float) {
				return p_vec1 - p_float;
			});

		glm.set_function("V2MulF",
			[](glm::vec2 p_vec1, float p_float) {
				return p_vec1 * p_float;
			});

		glm.set_function("V2DivF",
			[](glm::vec2 p_vec1, float p_float) {
				return p_vec1 / p_float;
			});

		glm.set_function("V2Add",
			[](glm::vec2 p_vec1, glm::vec2 p_vec2) {
				return p_vec1 + p_vec2;
			});

		glm.set_function("V2Sub",
			[](glm::vec2 p_vec1, glm::vec2 p_vec2) {
				return p_vec1 - p_vec2;
			});

		glm.set_function("V2Mul",
			[](glm::vec2 p_vec1, glm::vec2 p_vec2) {
				return p_vec1 * p_vec2;
			});

		glm.set_function("V2Div",
			[](glm::vec2 p_vec1, glm::vec2 p_vec2) {
				return p_vec1 / p_vec2;
			});

		glm.set_function("V2Dot",
			[](glm::vec2 p_vec1, glm::vec2 p_vec2) {
				return glm::dot(p_vec1, p_vec2);
			});

		glm.set_function("V2Normalize",
			[](glm::vec2 p_vec) {
				return glm::normalize(p_vec);
			});

		glm.set_function("V2Length",
			[](glm::vec2 p_vec) {
				return glm::length(p_vec);
			});

		glm.set_function("V2Distance",
			[](glm::vec2 p_vec1, glm::vec2 p_vec2) {
				return glm::distance(p_vec1, p_vec2);
			});

		glm.set_function("V2Lerp",
			[](glm::vec2 a, glm::vec2 b, float t) {
				return glm::mix(a, b, t);
			});

		glm.set_function("V2Reflect",
			[](glm::vec2 p_vec1, glm::vec2 p_vec2) {
				return glm::reflect(p_vec1, p_vec2);
			});

		glm.set_function("V2Refract",
			[](glm::vec2 p_vec1, glm::vec2 p_vec2, float p_float) {
				return glm::refract(p_vec1, p_vec2, p_float);
			});
	}

	// Vec3
	{
		glm.set_function("V3Abs",
			[](glm::vec3 p_vec) {
				return glm::abs(p_vec);
			});

		glm.set_function("V3Sign",
			[](glm::vec3 p_vec) {
				return glm::sign(p_vec);
			});

		glm.set_function("V3Floor",
			[](glm::vec3 p_vec) {
				return glm::floor(p_vec);
			});

		glm.set_function("V3Trunc",
			[](glm::vec3 p_vec) {
				return glm::trunc(p_vec);
			});

		glm.set_function("V3Round",
			[](glm::vec3 p_vec) {
				return glm::round(p_vec);
			});

		glm.set_function("V3RoundEven",
			[](glm::vec3 p_vec) {
				return glm::roundEven(p_vec);
			});

		glm.set_function("V3Ceil",
			[](glm::vec3 p_vec) {
				return glm::ceil(p_vec);
			});

		glm.set_function("V3Fract",
			[](glm::vec3 p_vec) {
				return glm::fract(p_vec);
			});

		glm.set_function("V3Mod",
			[](glm::vec3 p_vec1, glm::vec3 p_vec2) {
				return glm::mod(p_vec1, p_vec2);
			});

		glm.set_function("V3ModF",
			[](glm::vec3 p_vec1, float p_float) {
				return glm::mod(p_vec1, p_float);
			});

		glm.set_function("V3Min",
			[](glm::vec3 p_vec1, glm::vec3 p_vec2) {
				return glm::min(p_vec1, p_vec2);
			});

		glm.set_function("V3Max",
			[](glm::vec3 p_vec1, glm::vec3 p_vec2) {
				return glm::max(p_vec1, p_vec2);
			});

		glm.set_function("V3Clamp",
			[](glm::vec3 p_vec, glm::vec3 p_min, glm::vec3 p_max) {
				return glm::clamp(p_vec, p_min, p_max);
			});

		glm.set_function("V3AddF",
			[](glm::vec3 p_vec1, float p_float) {
				return p_vec1 + p_float;
			});

		glm.set_function("V3SubF",
			[](glm::vec3 p_vec1, float p_float) {
				return p_vec1 - p_float;
			});

		glm.set_function("V3MulF",
			[](glm::vec3 p_vec1, float p_float) {
				return p_vec1 * p_float;
			});

		glm.set_function("V3DivF",
			[](glm::vec3 p_vec1, float p_float) {
				return p_vec1 / p_float;
			});

		glm.set_function("V3Add",
			[](glm::vec3 p_vec1, glm::vec3 p_vec2) {
				return p_vec1 + p_vec2;
			});

		glm.set_function("V3Sub",
			[](glm::vec3 p_vec1, glm::vec3 p_vec2) {
				return p_vec1 - p_vec2;
			});

		glm.set_function("V3Mul",
			[](glm::vec3 p_vec1, glm::vec3 p_vec2) {
				return p_vec1 * p_vec2;
			});

		glm.set_function("V3Div",
			[](glm::vec3 p_vec1, glm::vec3 p_vec2) {
				return p_vec1 / p_vec2;
			});

		glm.set_function("V3Dot",
			[](glm::vec3 p_vec1, glm::vec3 p_vec2) {
				return glm::dot(p_vec1, p_vec2);
			});

		glm.set_function("V3Cross",
			[](glm::vec3 p_vec1, glm::vec3 p_vec2) {
				return glm::cross(p_vec1, p_vec2);
			});

		glm.set_function("V3Normalize",
			[](glm::vec3 p_vec) {
				return glm::normalize(p_vec);
			});

		glm.set_function("V3Length",
			[](glm::vec3 p_vec) {
				return glm::length(p_vec);
			});

		glm.set_function("V3Distance",
			[](glm::vec3 p_vec1, glm::vec3 p_vec2) {
				return glm::distance(p_vec1, p_vec2);
			});

		glm.set_function("V3Lerp",
			[](glm::vec3 a, glm::vec3 b, float t) {
				return glm::mix(a, b, t);
			});

		glm.set_function("V3Reflect",
			[](glm::vec3 p_vec1, glm::vec3 p_vec2) {
				return glm::reflect(p_vec1, p_vec2);
			});

		glm.set_function("V3Refract",
			[](glm::vec3 p_vec1, glm::vec3 p_vec2, float p_float) {
				return glm::refract(p_vec1, p_vec2, p_float);
			});
	}

	// Vec4
	{
		glm.set_function("V4Abs",
			[](glm::vec4 p_vec) {
				return glm::abs(p_vec);
			});

		glm.set_function("V4Sign",
			[](glm::vec4 p_vec) {
				return glm::sign(p_vec);
			});

		glm.set_function("V4Floor",
			[](glm::vec4 p_vec) {
				return glm::floor(p_vec);
			});

		glm.set_function("V4Trunc",
			[](glm::vec4 p_vec) {
				return glm::trunc(p_vec);
			});

		glm.set_function("V4Round",
			[](glm::vec4 p_vec) {
				return glm::round(p_vec);
			});

		glm.set_function("V4RoundEven",
			[](glm::vec4 p_vec) {
				return glm::roundEven(p_vec);
			});

		glm.set_function("V4Ceil",
			[](glm::vec4 p_vec) {
				return glm::ceil(p_vec);
			});

		glm.set_function("V4Fract",
			[](glm::vec4 p_vec) {
				return glm::fract(p_vec);
			});

		glm.set_function("V4Mod",
			[](glm::vec4 p_vec1, glm::vec4 p_vec2) {
				return glm::mod(p_vec1, p_vec2);
			});

		glm.set_function("V4ModF",
			[](glm::vec4 p_vec1, float p_float) {
				return glm::mod(p_vec1, p_float);
			});

		glm.set_function("V4Min",
			[](glm::vec4 p_vec1, glm::vec4 p_vec2) {
				return glm::min(p_vec1, p_vec2);
			});

		glm.set_function("V4Max",
			[](glm::vec4 p_vec1, glm::vec4 p_vec2) {
				return glm::max(p_vec1, p_vec2);
			});

		glm.set_function("V4Clamp",
			[](glm::vec4 p_vec, glm::vec4 p_min, glm::vec4 p_max) {
				return glm::clamp(p_vec, p_min, p_max);
			});

		glm.set_function("V4AddF",
			[](glm::vec4 p_vec1, float p_float) {
				return p_vec1 + p_float;
			});

		glm.set_function("V4SubF",
			[](glm::vec4 p_vec1, float p_float) {
				return p_vec1 - p_float;
			});

		glm.set_function("V4MulF",
			[](glm::vec4 p_vec1, float p_float) {
				return p_vec1 * p_float;
			});

		glm.set_function("V4DivF",
			[](glm::vec4 p_vec1, float p_float) {
				return p_vec1 / p_float;
			});

		glm.set_function("V4Add",
			[](glm::vec4 p_vec1, glm::vec4 p_vec2) {
				return p_vec1 + p_vec2;
			});

		glm.set_function("V4Sub",
			[](glm::vec4 p_vec1, glm::vec4 p_vec2) {
				return p_vec1 - p_vec2;
			});

		glm.set_function("V4Mul",
			[](glm::vec4 p_vec1, glm::vec4 p_vec2) {
				return p_vec1 * p_vec2;
			});

		glm.set_function("V4Div",
			[](glm::vec4 p_vec1, glm::vec4 p_vec2) {
				return p_vec1 / p_vec2;
			});

		glm.set_function("V4Dot",
			[](glm::vec4 p_vec1, glm::vec4 p_vec2) {
				return glm::dot(p_vec1, p_vec2);
			});

		glm.set_function("V4Normalize",
			[](glm::vec4 p_vec) {
				return glm::normalize(p_vec);
			});

		glm.set_function("V4Length",
			[](glm::vec4 p_vec) {
				return glm::length(p_vec);
			});

		glm.set_function("V4Distance",
			[](glm::vec4 p_vec1, glm::vec4 p_vec2) {
				return glm::distance(p_vec1, p_vec2);
			});

		glm.set_function("V4Lerp",
			[](glm::vec4 a, glm::vec4 b, float t) {
				return glm::mix(a, b, t);
			});

		glm.set_function("V4Reflect",
			[](glm::vec4 p_vec1, glm::vec4 p_vec2) {
				return glm::reflect(p_vec1, p_vec2);
			});

		glm.set_function("V4Refract",
			[](glm::vec4 p_vec1, glm::vec4 p_vec2, float p_float) {
				return glm::refract(p_vec1, p_vec2, p_float);
			});
	}

	// Quat
	{
		glm.set_function("Angle",
			[](glm::quat p_quat) {
				return glm::angle(p_quat);
			});

		glm.set_function("Axis",
			[](glm::quat p_quat) {
				return glm::axis(p_quat);
			});

		glm.set_function("AngleAxis",
			[](glm::vec3 p_axis, float p_angle) {
				return glm::angleAxis(p_angle, p_axis);
			});
	}

	// Mat3
	{
		glm.set_function("M3MulF",
			[](glm::mat3 p_mat, float p_float) {
				return p_mat * p_float;
			});

		glm.set_function("M3Mul",
			[](glm::mat3 p_mat1, glm::mat3 p_mat2) {
				return p_mat1 * p_mat2;
			});

		glm.set_function("M3MulV",
			[](glm::mat3 p_mat, glm::vec3 p_vec) {
				return p_mat * p_vec;
			});

		glm.set_function("M3Transpose",
			[](glm::mat3 p_mat) {
				return glm::transpose(p_mat);
			});

		glm.set_function("M3Inverse",
			[](glm::mat3 p_mat) {
				return glm::inverse(p_mat);
			});

		glm.set_function("M3Determinant",
			[](glm::mat3 p_mat) {
				return glm::determinant(p_mat);
			});
	}

	// Mat4
	{
		glm.set_function("M4MulF",
			[](glm::mat4 p_mat, float p_float) {
				return p_mat * p_float;
			});

		glm.set_function("M4Mul",
			[](glm::mat4 p_mat1, glm::mat4 p_mat2) {
				return p_mat1 * p_mat2;
			});

		glm.set_function("M4MulV",
			[](glm::mat4 p_mat, glm::vec4 p_vec) {
				return p_mat * p_vec;
			});

		glm.set_function("M4Transpose",
			[](glm::mat4 p_mat) {
				return glm::transpose(p_mat);
			});

		glm.set_function("M4Inverse",
			[](glm::mat4 p_mat) {
				return glm::inverse(p_mat);
			});

		glm.set_function("M4Determinant",
			[](glm::mat4 p_mat) {
				return glm::determinant(p_mat);
			});
	}


	LuaClassDefinition glmDef;
	glmDef.name = "GLM";

	glmDef.functions =
	{
		{
			"V2Abs",
			"Vec2",
			true,
			{
				{ "vec", "Vec2" }
			}
		},
		{
			"V2Sign",
			"Vec2",
			true,
			{
				{ "vec", "Vec2" }
			}
		},
		{
			"V2Floor",
			"Vec2",
			true,
			{
				{ "vec", "Vec2" }
			}
		},
		{
			"V2Trunc",
			"Vec2",
			true,
			{
				{ "vec", "Vec2" }
			}
		},
		{
			"V2Round",
			"Vec2",
			true,
			{
				{ "vec", "Vec2" }
			}
		},
		{
			"V2RoundEven",
			"Vec2",
			true,
			{
				{ "vec", "Vec2" }
			}
		},
		{
			"V2Ceil",
			"Vec2",
			true,
			{
				{ "vec", "Vec2" }
			}
		},
		{
			"V2Fract",
			"Vec2",
			true,
			{
				{ "vec", "Vec2" }
			}
		},
		{
			"V2Mod",
			"Vec2",
			true,
			{
				{ "vec1", "Vec2" },
				{ "vec2", "Vec2" }
			}
		},
		{
			"V2ModF",
			"Vec2",
			true,
			{
				{ "vec", "Vec2" },
				{ "float", "number" }
			}
		},
		{
			"V2Min",
			"Vec2",
			true,
			{
				{ "vec1", "Vec2" },
				{ "vec2", "Vec2" }
			}
		},
		{
			"V2Max",
			"Vec2",
			true,
			{
				{ "vec1", "Vec2" },
				{ "vec2", "Vec2" }
			}
		},
		{
			"V2Clamp",
			"Vec2",
			true,
			{
				{ "vec", "Vec2" },
				{ "min", "Vec2" },
				{ "max", "Vec2" }
			}
		},
		{
			"V2AddF",
			"Vec2",
			true,
			{
				{ "vec", "Vec2" },
				{ "float", "number" }
			}
		},
		{
			"V2SubF",
			"Vec2",
			true,
			{
				{ "vec", "Vec2" },
				{ "float", "number" }
			}
		},
		{
			"V2MulF",
			"Vec2",
			true,
			{
				{ "vec", "Vec2" },
				{ "float", "number" }
			}
		},
		{
			"V2DivF",
			"Vec2",
			true,
			{
				{ "vec", "Vec2" },
				{ "float", "number" }
			}
		},
		{
			"V2Add",
			"Vec2",
			true,
			{
				{ "vec1", "Vec2" },
				{ "vec2", "Vec2" }
			}
		},
		{
			"V2Sub",
			"Vec2",
			true,
			{
				{ "vec1", "Vec2" },
				{ "vec2", "Vec2" }
			}
		},
		{
			"V2Mul",
			"Vec2",
			true,
			{
				{ "vec1", "Vec2" },
				{ "vec2", "Vec2" }
			}
		},
		{
			"V2Div",
			"Vec2",
			true,
			{
				{ "vec1", "Vec2" },
				{ "vec2", "Vec2" }
			}
		},
		{
			"V2Dot",
			"number",
			true,
			{
				{ "vec1", "Vec2" },
				{ "vec2", "Vec2" }
			}
		},
		{
			"V2Normalize",
			"Vec2",
			true,
			{
				{ "vec", "Vec2" }
			}
		},
		{
			"V2Length",
			"number",
			true,
			{
				{ "vec", "Vec2" }
			}
		},
		{
			"V2Distance",
			"number",
			true,
			{
				{ "vec1", "Vec2" },
				{ "vec2", "Vec2" }
			}
		},
		{
			"V2Lerp",
			"Vec2",
			true,
			{
				{ "a", "Vec2" },
				{ "b", "Vec2" },
				{ "t", "number" }
			}
		},
		{
			"V2Reflect",
			"Vec2",
			true,
			{
				{ "vec1", "Vec2" },
				{ "vec2", "Vec2" }
			}
		},
		{
			"V2Refract",
			"Vec2",
			true,
			{
				{ "vec1", "Vec2" },
				{ "vec2", "Vec2" },
				{ "float", "number" }
			}
		},

		{
			"V3Abs",
			"Vec3",
			true,
			{
				{ "vec", "Vec3" }
			}
		},
		{
			"V3Sign",
			"Vec3",
			true,
			{
				{ "vec", "Vec3" }
			}
		},
		{
			"V3Floor",
			"Vec3",
			true,
			{
				{ "vec", "Vec3" }
			}
		},
		{
			"V3Trunc",
			"Vec3",
			true,
			{
				{ "vec", "Vec3" }
			}
		},
		{
			"V3Round",
			"Vec3",
			true,
			{
				{ "vec", "Vec3" }
			}
		},
		{
			"V3RoundEven",
			"Vec3",
			true,
			{
				{ "vec", "Vec3" }
			}
		},
		{
			"V3Ceil",
			"Vec3",
			true,
			{
				{ "vec", "Vec3" }
			}
		},
		{
			"V3Fract",
			"Vec3",
			true,
			{
				{ "vec", "Vec3" }
			}
		},
		{
			"V3Mod",
			"Vec3",
			true,
			{
				{ "vec1", "Vec3" },
				{ "vec2", "Vec3" }
			}
		},
		{
			"V3ModF",
			"Vec3",
			true,
			{
				{ "vec", "Vec3" },
				{ "float", "number" }
			}
		},
		{
			"V3Min",
			"Vec3",
			true,
			{
				{ "vec1", "Vec3" },
				{ "vec2", "Vec3" }
			}
		},
		{
			"V3Max",
			"Vec3",
			true,
			{
				{ "vec1", "Vec3" },
				{ "vec2", "Vec3" }
			}
		},
		{
			"V3Clamp",
			"Vec3",
			true,
			{
				{ "vec", "Vec3" },
				{ "min", "Vec3" },
				{ "max", "Vec3" }
			}
		},
		{
			"V3AddF",
			"Vec3",
			true,
			{
				{ "vec", "Vec3" },
				{ "float", "number" }
			}
		},
		{
			"V3SubF",
			"Vec3",
			true,
			{
				{ "vec", "Vec3" },
				{ "float", "number" }
			}
		},
		{
			"V3MulF",
			"Vec3",
			true,
			{
				{ "vec", "Vec3" },
				{ "float", "number" }
			}
		},
		{
			"V3DivF",
			"Vec3",
			true,
			{
				{ "vec", "Vec3" },
				{ "float", "number" }
			}
		},
		{
			"V3Add",
			"Vec3",
			true,
			{
				{ "vec1", "Vec3" },
				{ "vec2", "Vec3" }
			}
		},
		{
			"V3Sub",
			"Vec3",
			true,
			{
				{ "vec1", "Vec3" },
				{ "vec2", "Vec3" }
			}
		},
		{
			"V3Mul",
			"Vec3",
			true,
			{
				{ "vec1", "Vec3" },
				{ "vec2", "Vec3" }
			}
		},
		{
			"V3Div",
			"Vec3",
			true,
			{
				{ "vec1", "Vec3" },
				{ "vec2", "Vec3" }
			}
		},
		{
			"V3Dot",
			"number",
			true,
			{
				{ "vec1", "Vec3" },
				{ "vec2", "Vec3" }
			}
		},
		{
			"V3Cross",
			"Vec3",
			true,
			{
				{ "vec1", "Vec3" },
				{ "vec2", "Vec3" }
			}
		},
		{
			"V3Normalize",
			"Vec3",
			true,
			{
				{ "vec", "Vec3" }
			}
		},
		{
			"V3Length",
			"number",
			true,
			{
				{ "vec", "Vec3" }
			}
		},
		{
			"V3Distance",
			"number",
			true,
			{
				{ "vec1", "Vec3" },
				{ "vec2", "Vec3" }
			}
		},
		{
			"V3Lerp",
			"Vec3",
			true,
			{
				{ "a", "Vec3" },
				{ "b", "Vec3" },
				{ "t", "number" }
			}
		},
		{
			"V3Reflect",
			"Vec3",
			true,
			{
				{ "vec1", "Vec3" },
				{ "vec2", "Vec3" }
			}
		},
		{
			"V3Refract",
			"Vec3",
			true,
			{
				{ "vec1", "Vec3" },
				{ "vec2", "Vec3" },
				{ "float", "number" }
			}
		},

		{
			"V4Abs",
			"Vec4",
			true,
			{
				{ "vec", "Vec4" }
			}
		},
		{
			"V4Sign",
			"Vec4",
			true,
			{
				{ "vec", "Vec4" }
			}
		},
		{
			"V4Floor",
			"Vec4",
			true,
			{
				{ "vec", "Vec4" }
			}
		},
		{
			"V4Trunc",
			"Vec4",
			true,
			{
				{ "vec", "Vec4" }
			}
		},
		{
			"V4Round",
			"Vec4",
			true,
			{
				{ "vec", "Vec4" }
			}
		},
		{
			"V4RoundEven",
			"Vec4",
			true,
			{
				{ "vec", "Vec4" }
			}
		},
		{
			"V4Ceil",
			"Vec4",
			true,
			{
				{ "vec", "Vec4" }
			}
		},
		{
			"V4Fract",
			"Vec4",
			true,
			{
				{ "vec", "Vec4" }
			}
		},
		{
			"V4Mod",
			"Vec4",
			true,
			{
				{ "vec1", "Vec4" },
				{ "vec2", "Vec4" }
			}
		},
		{
			"V4ModF",
			"Vec4",
			true,
			{
				{ "vec", "Vec4" },
				{ "float", "number" }
			}
		},
		{
			"V4Min",
			"Vec4",
			true,
			{
				{ "vec1", "Vec4" },
				{ "vec2", "Vec4" }
			}
		},
		{
			"V4Max",
			"Vec4",
			true,
			{
				{ "vec1", "Vec4" },
				{ "vec2", "Vec4" }
			}
		},
		{
			"V4Clamp",
			"Vec4",
			true,
			{
				{ "vec", "Vec4" },
				{ "min", "Vec4" },
				{ "max", "Vec4" }
			}
		},
		{
			"V4AddF",
			"Vec4",
			true,
			{
				{ "vec", "Vec4" },
				{ "float", "number" }
			}
		},
		{
			"V4SubF",
			"Vec4",
			true,
			{
				{ "vec", "Vec4" },
				{ "float", "number" }
			}
		},
		{
			"V4MulF",
			"Vec4",
			true,
			{
				{ "vec", "Vec4" },
				{ "float", "number" }
			}
		},
		{
			"V4DivF",
			"Vec4",
			true,
			{
				{ "vec", "Vec4" },
				{ "float", "number" }
			}
		},
		{
			"V4Add",
			"Vec4",
			true,
			{
				{ "vec1", "Vec4" },
				{ "vec2", "Vec4" }
			}
		},
		{
			"V4Sub",
			"Vec4",
			true,
			{
				{ "vec1", "Vec4" },
				{ "vec2", "Vec4" }
			}
		},
		{
			"V4Mul",
			"Vec4",
			true,
			{
				{ "vec1", "Vec4" },
				{ "vec2", "Vec4" }
			}
		},
		{
			"V4Div",
			"Vec4",
			true,
			{
				{ "vec1", "Vec4" },
				{ "vec2", "Vec4" }
			}
		},
		{
			"V4Dot",
			"number",
			true,
			{
				{ "vec1", "Vec4" },
				{ "vec2", "Vec4" }
			}
		},
		{
			"V4Cross",
			"Vec4",
			true,
			{
				{ "vec1", "Vec4" },
				{ "vec2", "Vec4" }
			}
		},
		{
			"V4Normalize",
			"Vec4",
			true,
			{
				{ "vec", "Vec4" }
			}
		},
		{
			"V4Length",
			"number",
			true,
			{
				{ "vec", "Vec4" }
			}
		},
		{
			"V4Distance",
			"number",
			true,
			{
				{ "vec1", "Vec4" },
				{ "vec2", "Vec4" }
			}
		},
		{
			"V4Lerp",
			"Vec4",
			true,
			{
				{ "a", "Vec4" },
				{ "b", "Vec4" },
				{ "t", "number" }
			}
		},
		{
			"V4Reflect",
			"Vec4",
			true,
			{
				{ "vec1", "Vec4" },
				{ "vec2", "Vec4" }
			}
		},
		{
			"V4Refract",
			"Vec4",
			true,
			{
				{ "vec1", "Vec4" },
				{ "vec2", "Vec4" },
				{ "float", "number" }
			}
		},

		{
			"Angle",
			"number",
			true,
			{
				{ "quat", "Quat" }
			}
		},
		{
			"Axis",
			"Vec3",
			true,
			{
				{ "quat", "Quat" }
			}
		},
		{
			"AngleAxis",
			"Quat",
			true,
			{
				{ "axis", "Vec3" },
				{ "angle", "number" }
			}
		},

		{
			"M3MulF",
			"Mat3",
			true,
			{
				{ "mat", "Mat3" },
				{ "float", "number" }
			}
		},
		{
			"M3Mul",
			"Mat3",
			true,
			{
				{ "mat1", "Mat3" },
				{ "mat2", "Mat3" }
			}
		},
		{
			"M3MulV",
			"Vec3",
			true,
			{
				{ "mat", "Mat3" },
				{ "vec", "Vec3" }
			}
		},
		{
			"M3Transpose",
			"Mat3",
			true,
			{
				{ "mat", "Mat3" }
			}
		},
		{
			"M3Inverse",
			"Mat3",
			true,
			{
				{ "mat", "Mat3" }
			}
		},
		{
			"M3Determinant",
			"number",
			true,
			{
				{ "mat", "Mat3" }
			}
		},

		{
			"M4MulF",
			"Mat4",
			true,
			{
				{ "mat", "Mat4" },
				{ "float", "number" }
			}
		},
		{
			"M4Mul",
			"Mat4",
			true,
			{
				{ "mat1", "Mat4" },
				{ "mat2", "Mat4" }
			}
		},
		{
			"M4MulV",
			"Vec4",
			true,
			{
				{ "mat", "Mat4" },
				{ "vec", "Vec4" }
			}
		},
		{
			"M4Transpose",
			"Mat4",
			true,
			{
				{ "mat", "Mat4" }
			}
		},
		{
			"M4Inverse",
			"Mat4",
			true,
			{
				{ "mat", "Mat4" }
			}
		},
		{
			"M4Determinant",
			"number",
			true,
			{
				{ "mat", "Mat4" }
			}
		},
	};

	LuaBindings::m_luaClassDefinitions.push_back(glmDef);
}

void LuaBindings::RegisterInput(sol::state_view p_luaState)
{
	ZoneScoped;
	
	p_luaState.new_enum<Key>(
		"Key",
		{
			{"A", Key::KeyA},
			{"B", Key::KeyB},
			{"C", Key::KeyC},
			{"D", Key::KeyD},
			{"E", Key::KeyE},
			{"F", Key::KeyF},
			{"G", Key::KeyG},
			{"H", Key::KeyH},
			{"I", Key::KeyI},
			{"J", Key::KeyJ},
			{"K", Key::KeyK},
			{"L", Key::KeyL},
			{"M", Key::KeyM},
			{"N", Key::KeyN},
			{"O", Key::KeyO},
			{"P", Key::KeyP},
			{"Q", Key::KeyQ},
			{"R", Key::KeyR},
			{"S", Key::KeyS},
			{"T", Key::KeyT},
			{"U", Key::KeyU},
			{"V", Key::KeyV},
			{"W", Key::KeyW},
			{"X", Key::KeyX},
			{"Y", Key::KeyY},
			{"Z", Key::KeyZ},

			{"1", Key::Key1},
			{"2", Key::Key2},
			{"3", Key::Key3},
			{"4", Key::Key4},
			{"5", Key::Key5},
			{"6", Key::Key6},
			{"7", Key::Key7},
			{"8", Key::Key8},
			{"9", Key::Key9},
			{"0", Key::Key0},

			{"Enter", Key::KeyEnter},
			{"Escape", Key::KeyEscape},
			{"Backspace", Key::KeyBackspace},
			{"Tab", Key::KeyTab},
			{"Space", Key::KeySpace},
			{"LCTRL", Key::KeyLCTRL},
			{"RCTRL", Key::KeyRCTRL},
			{"SHIFT", Key::KeyShift},

			{"F1", Key::KeyF1},
			{"F2", Key::KeyF2},
			{"F3", Key::KeyF3},
			{"F4", Key::KeyF4},
			{"F5", Key::KeyF5},
			{"F6", Key::KeyF6},
			{"F7", Key::KeyF7},
			{"F8", Key::KeyF8},
			{"F9", Key::KeyF9},
			{"F10", Key::KeyF10},
			{"F11", Key::KeyF11},
			{"F12", Key::KeyF12},

			{"ArrowUp", Key::KeyArrowUP},
			{"ArrowRight", Key::KeyArrowRight},
			{"ArrowLeft", Key::KeyArrowLeft},
			{"ArrowDown", Key::KeyArrowDown}

		}
	);

	p_luaState.new_enum<Mouse>(
		"Mouse",
		{
			{"LMB", Mouse::LMB},
			{"RMB", Mouse::RMB}
		}
	);

	p_luaState.new_usertype<GameInput>(
		"Input",
		"KeyPressed", &GameInput::KeyPressed,
		"KeyHeld", &GameInput::KeyHeld,
		"KeyReleased", &GameInput::KeyReleased,
		"KeyToggle", &GameInput::KeyToggle,
		"MousePressed", &GameInput::MousePressed,
		"MouseHeld", &GameInput::MouseHeld,
		"MouseReleased", &GameInput::MouseReleased,
		"GetCursorX", &GameInput::GetCursorX,
		"GetCursorY", &GameInput::GetCursorY,
		"GetDeltaMouseX", &GameInput::GetDeltaMouseX,
		"GetDeltaMouseY", &GameInput::GetDeltaMouseY
	);

	LuaClassDefinition keyDef;
	keyDef.name = "Key";

	keyDef.properties =
	{
		{ "A", "Key" },
		{ "B", "Key" },
		{ "C", "Key" },
		{ "D", "Key" },
		{ "E", "Key" },
		{ "F", "Key" },
		{ "G", "Key" },
		{ "H", "Key" },
		{ "I", "Key" },
		{ "J", "Key" },
		{ "K", "Key" },
		{ "L", "Key" },
		{ "M", "Key" },
		{ "N", "Key" },
		{ "O", "Key" },
		{ "P", "Key" },
		{ "Q", "Key" },
		{ "R", "Key" },
		{ "S", "Key" },
		{ "T", "Key" },
		{ "U", "Key" },
		{ "V", "Key" },
		{ "W", "Key" },
		{ "X", "Key" },
		{ "Y", "Key" },
		{ "Z", "Key" },

		{ "Num1", "Key" },
		{ "Num2", "Key" },
		{ "Num3", "Key" },
		{ "Num4", "Key" },
		{ "Num5", "Key" },
		{ "Num6", "Key" },
		{ "Num7", "Key" },
		{ "Num8", "Key" },
		{ "Num9", "Key" },
		{ "Num0", "Key" },

		{ "Enter", "Key" },
		{ "Escape", "Key" },
		{ "Backspace", "Key" },
		{ "Tab", "Key" },
		{ "Space", "Key" },

		{ "F1", "Key" },
		{ "F2", "Key" },
		{ "F3", "Key" },
		{ "F4", "Key" },
		{ "F5", "Key" },
		{ "F6", "Key" },
		{ "F7", "Key" },
		{ "F8", "Key" },
		{ "F9", "Key" },
		{ "F10", "Key" },
		{ "F11", "Key" },
		{ "F12", "Key" },

		{ "ArrowUp", "Key" },
		{ "ArrowRight", "Key" },
		{ "ArrowLeft", "Key" },
		{ "ArrowDown", "Key" }
	};

	LuaBindings::m_luaClassDefinitions.push_back(keyDef);

	LuaClassDefinition mouseDef;
	mouseDef.name = "Mouse";

	mouseDef.properties =
	{
		{ "LMB", "Mouse" },
		{ "RMB", "Mouse" }
	};

	LuaBindings::m_luaClassDefinitions.push_back(mouseDef);

	LuaClassDefinition inputDef;
	inputDef.name = "Input";

	inputDef.functions =
	{
		{ "KeyPressed", "boolean", {{ "key", "Key" }} },
		{ "KeyHeld", "boolean", {{ "key", "Key" }} },
		{ "KeyReleased", "boolean", {{ "key", "Key" }} },
		{ "KeyToggle", "boolean", {{ "key", "Key" }} },

		{ "MousePressed", "boolean", {{ "button", "Mouse" }} },
		{ "MouseHeld", "boolean", {{ "button", "Mouse" }} },
		{ "MouseReleased", "boolean", {{ "button", "Mouse" }} },

		{ "GetCursorX", "number", {} },
		{ "GetCursorY", "number", {} },
		{ "GetDeltaMouseX", "number", {} },
		{ "GetDeltaMouseY", "number", {} }
	};

	LuaBindings::m_luaClassDefinitions.push_back(inputDef);
}

void LuaBindings::RegisterImGui(sol::state_view p_luaState)
{
	ZoneScoped;

	// Enums
	{
		p_luaState.new_enum<ImGuiWindowFlags>(
			"ImGuiWindowFlags",
			{
				{"None", ImGuiWindowFlags_None },
				{"NoTitleBar", ImGuiWindowFlags_NoTitleBar },
				{"NoResize", ImGuiWindowFlags_NoResize },
				{"NoMove", ImGuiWindowFlags_NoMove },
				{"NoScrollbar", ImGuiWindowFlags_NoScrollbar },
				{"NoScrollWithMouse", ImGuiWindowFlags_NoScrollWithMouse },
				{"NoCollapse", ImGuiWindowFlags_NoCollapse },
				{"AlwaysAutoResize", ImGuiWindowFlags_AlwaysAutoResize },
				{"NoBackground", ImGuiWindowFlags_NoBackground },
				{"NoSavedSettings", ImGuiWindowFlags_NoSavedSettings },
				{"NoMouseInputs", ImGuiWindowFlags_NoMouseInputs },
				{"MenuBar", ImGuiWindowFlags_MenuBar },
				{"HorizontalScrollbar", ImGuiWindowFlags_HorizontalScrollbar },
				{"NoFocusOnAppearing", ImGuiWindowFlags_NoFocusOnAppearing },
				{"NoBringToFrontOnFocus", ImGuiWindowFlags_NoBringToFrontOnFocus },
				{"AlwaysVerticalScrollbar", ImGuiWindowFlags_AlwaysVerticalScrollbar },
				{"AlwaysHorizontalScrollbar", ImGuiWindowFlags_AlwaysHorizontalScrollbar },
				{"NoNavInputs", ImGuiWindowFlags_NoNavInputs },
				{"NoNavFocus", ImGuiWindowFlags_NoNavFocus },
				{"UnsavedDocument", ImGuiWindowFlags_UnsavedDocument },
				{"NoDocking", ImGuiWindowFlags_NoDocking },
				{"NoNav", ImGuiWindowFlags_NoNav },
				{"NoDecoration", ImGuiWindowFlags_NoDecoration },
				{"NoInputs", ImGuiWindowFlags_NoInputs }
			}
		);

		LuaEnumDefinition imGuiWindowFlags;
		imGuiWindowFlags.name = "ImGuiWindowFlags";

		imGuiWindowFlags.flags =
		{
			{"None", ImGuiWindowFlags_None },
			{"NoTitleBar", ImGuiWindowFlags_NoTitleBar },
			{"NoResize", ImGuiWindowFlags_NoResize },
			{"NoMove", ImGuiWindowFlags_NoMove },
			{"NoScrollbar", ImGuiWindowFlags_NoScrollbar },
			{"NoScrollWithMouse", ImGuiWindowFlags_NoScrollWithMouse },
			{"NoCollapse", ImGuiWindowFlags_NoCollapse },
			{"AlwaysAutoResize", ImGuiWindowFlags_AlwaysAutoResize },
			{"NoBackground", ImGuiWindowFlags_NoBackground },
			{"NoSavedSettings", ImGuiWindowFlags_NoSavedSettings },
			{"NoMouseInputs", ImGuiWindowFlags_NoMouseInputs },
			{"MenuBar", ImGuiWindowFlags_MenuBar },
			{"HorizontalScrollbar", ImGuiWindowFlags_HorizontalScrollbar },
			{"NoFocusOnAppearing", ImGuiWindowFlags_NoFocusOnAppearing },
			{"NoBringToFrontOnFocus", ImGuiWindowFlags_NoBringToFrontOnFocus },
			{"AlwaysVerticalScrollbar", ImGuiWindowFlags_AlwaysVerticalScrollbar },
			{"AlwaysHorizontalScrollbar", ImGuiWindowFlags_AlwaysHorizontalScrollbar },
			{"NoNavInputs", ImGuiWindowFlags_NoNavInputs },
			{"NoNavFocus", ImGuiWindowFlags_NoNavFocus },
			{"UnsavedDocument", ImGuiWindowFlags_UnsavedDocument },
			{"NoDocking", ImGuiWindowFlags_NoDocking },
			{"NoNav", ImGuiWindowFlags_NoNav },
			{"NoDecoration", ImGuiWindowFlags_NoDecoration },
			{"NoInputs", ImGuiWindowFlags_NoInputs }
		};

		LuaBindings::m_luaEnumDefinitions.push_back(imGuiWindowFlags);


		p_luaState.new_enum<ImGuiChildFlags>(
			"ImGuiChildFlags",
			{
				{"None", ImGuiChildFlags_None },
				{"Borders", ImGuiChildFlags_Borders },
				{"AlwaysUseWindowPadding", ImGuiChildFlags_AlwaysUseWindowPadding },
				{"ResizeX", ImGuiChildFlags_ResizeX },
				{"ResizeY", ImGuiChildFlags_ResizeY },
				{"AutoResizeX", ImGuiChildFlags_AutoResizeX },
				{"AutoResizeY", ImGuiChildFlags_AutoResizeY },
				{"AlwaysAutoResize", ImGuiChildFlags_AlwaysAutoResize },
				{"FrameStyle", ImGuiChildFlags_FrameStyle },
				{"NavFlattened", ImGuiChildFlags_NavFlattened }
			}
		);

		LuaEnumDefinition imGuiChildFlags;
		imGuiChildFlags.name = "ImGuiChildFlags";

		imGuiChildFlags.flags =
		{
			{"None", ImGuiChildFlags_None },
			{"Borders", ImGuiChildFlags_Borders },
			{"AlwaysUseWindowPadding", ImGuiChildFlags_AlwaysUseWindowPadding },
			{"ResizeX", ImGuiChildFlags_ResizeX },
			{"ResizeY", ImGuiChildFlags_ResizeY },
			{"AutoResizeX", ImGuiChildFlags_AutoResizeX },
			{"AutoResizeY", ImGuiChildFlags_AutoResizeY },
			{"AlwaysAutoResize", ImGuiChildFlags_AlwaysAutoResize },
			{"FrameStyle", ImGuiChildFlags_FrameStyle },
			{"NavFlattened", ImGuiChildFlags_NavFlattened }
		};

		LuaBindings::m_luaEnumDefinitions.push_back(imGuiChildFlags);
	}


	// Functions

	{
		sol::table imgui = p_luaState.create_table("ImGui");

		imgui.set_function(
			"Begin",
			[](const std::string &p_name,
				sol::optional<ImGuiWindowFlags> p_flags)
			{
				return ImGui::Begin(p_name.c_str(), nullptr, p_flags.value_or(static_cast<ImGuiWindowFlags>(0)));
			}
		);

		imgui.set_function(
			"End",
			[]()
			{
				ImGui::End();
			}
		);

		imgui.set_function(
			"BeginChild",
			[](const std::string &p_name,
				sol::optional<ImVec2> p_size,
				sol::optional<ImGuiChildFlags> p_childFlags,
				sol::optional<ImGuiWindowFlags> p_windowFlags)
			{
				ImVec2 size = p_size.value_or(ImVec2(0.0f, 0.0f));
				ImGuiChildFlags childFlags = p_childFlags.value_or(static_cast<ImGuiChildFlags>(0));
				ImGuiWindowFlags windowFlags = p_windowFlags.value_or(static_cast<ImGuiWindowFlags>(0));
				return ImGui::BeginChild(p_name.c_str(), size, true, childFlags | windowFlags);
			}
		);

		imgui.set_function(
			"EndChild",
			[]()
			{
				ImGui::EndChild();
			}
		);

		imgui.set_function(
			"Text",
			[](const std::string &p_text)
			{
				ImGui::Text(p_text.c_str());
			}
		);

		imgui.set_function("TextUnformatted",
			[](const std::string &p_text)
			{
				ImGui::TextUnformatted(p_text.c_str());
			}
		);

		imgui.set_function("InputText",
			[](const std::string &p_label, std::string p_text)
			{
				bool changed = ImGui::InputText(
					p_label.c_str(),
					&p_text
				);

				return std::make_tuple(std::move(p_text), changed);
			}
		);

		imgui.set_function(
			"Separator",
			[]()
			{
				ImGui::Separator();
			}
		);

		imgui.set_function(
			"SeparatorText",
			[](const std::string &p_text)
			{
				ImGui::SeparatorText(p_text.c_str());
			}
		);

		imgui.set_function(
			"Spacing",
			[]()
			{
				ImGui::Spacing();
			}
		);

		imgui.set_function(
			"SameLine",
			[](sol::optional<float> p_offset,
				sol::optional<float> p_spacing)
			{
				float offset = p_offset.value_or(0.0f);
				float spacing = p_spacing.value_or(-1.0f);
				ImGui::SameLine(offset, spacing);
			}
		);

		imgui.set_function(
			"Button",
			[](const std::string &p_label,
				sol::optional<ImVec2> p_size)
			{
				ImVec2 size = p_size.value_or(ImVec2(0.0f, 0.0f));
				return ImGui::Button(p_label.c_str(), size);
			}
		);

		imgui.set_function(
			"Checkbox",
			[](const std::string &p_label, bool p_value)
			{
				bool value = p_value;

				const bool changed = ImGui::Checkbox(p_label.c_str(), &value);

				return std::make_tuple(changed, value);
			}
		);

		imgui.set_function(
			"DragFloat",
			[](const std::string &p_label,
				float p_value,
				sol::optional<float> p_speed,
				sol::optional<float> p_min,
				sol::optional<float> p_max)
			{
				float value = p_value;
				float speed = p_speed.value_or(1.0f);
				float min = p_min.value_or(0.0f);
				float max = p_max.value_or(0.0f);

				const bool changed = ImGui::DragFloat(
					p_label.c_str(),
					&value,
					speed,
					min,
					max
				);
				return std::make_tuple(changed, value);
			}
		);

		imgui.set_function(
			"DragFloat2",
			[](const std::string &p_label,
				glm::vec2 p_value,
				sol::optional<float> p_speed,
				sol::optional<float> p_min,
				sol::optional<float> p_max)
			{
				glm::vec2 value = p_value;
				float speed = p_speed.value_or(1.0f);
				float min = p_min.value_or(0.0f);
				float max = p_max.value_or(0.0f);

				const bool changed = ImGui::DragFloat2(
					p_label.c_str(),
					&value.x,
					speed,
					min,
					max
				);
				return std::make_tuple(changed, value);
			}
		);

		imgui.set_function(
			"DragFloat3",
			[](const std::string &p_label,
				glm::vec3 p_value,
				sol::optional<float> p_speed,
				sol::optional<float> p_min,
				sol::optional<float> p_max
				)
			{
				glm::vec3 value = p_value;
				float speed = p_speed.value_or(1.0f);
				float min = p_min.value_or(0.0f);
				float max = p_max.value_or(0.0f);

				const bool changed = ImGui::DragFloat3(
					p_label.c_str(),
					&value.x,
					speed,
					min,
					max
				);

				return std::make_tuple(changed, value);
			}
		);

		imgui.set_function(
			"DragFloat4",
			[](const std::string &p_label,
				glm::vec4 p_value,
				sol::optional<float> p_speed,
				sol::optional<float> p_min,
				sol::optional<float> p_max)
			{
				glm::vec4 value = p_value;
				float speed = p_speed.value_or(1.0f);
				float min = p_min.value_or(0.0f);
				float max = p_max.value_or(0.0f);

				const bool changed = ImGui::DragFloat4(
					p_label.c_str(),
					&value.x,
					speed,
					min,
					max
				);
				return std::make_tuple(changed, value);
			}
		);

		imgui.set_function(
			"DragInt",
			[](const std::string &p_label, int p_value, float p_speed, int p_min, int p_max)
			{
				int value = p_value;

				const bool changed = ImGui::DragInt(
					p_label.c_str(),
					&value,
					p_speed,
					p_min,
					p_max
				);
				return std::make_tuple(changed, value);
			}
		);

		imgui.set_function(
			"DragInt2",
			[](const std::string &p_label,
				glm::i32vec2 p_value,
				sol::optional<float> p_speed,
				sol::optional<int> p_min,
				sol::optional<int> p_max)
			{
				glm::i32vec2 value = p_value;
				float speed = p_speed.value_or(1.0f);
				int min = p_min.value_or(0);
				int max = p_max.value_or(0);

				const bool changed = ImGui::DragInt2(
					p_label.c_str(),
					&value.x,
					speed,
					min,
					max
				);
				return std::make_tuple(changed, value);
			}
		);

		imgui.set_function(
			"DragInt3",
			[](const std::string &p_label,
				glm::i32vec3 p_value,
				sol::optional<float> p_speed,
				sol::optional<int> p_min,
				sol::optional<int> p_max)
			{
				glm::i32vec3 value = p_value;
				float speed = p_speed.value_or(1.0f);
				int min = p_min.value_or(0);
				int max = p_max.value_or(0);

				const bool changed = ImGui::DragInt3(
					p_label.c_str(),
					&value.x,
					speed,
					min,
					max
				);
				return std::make_tuple(changed, value);
			}
		);

		imgui.set_function(
			"DragInt4",
			[](const std::string &p_label,
				glm::i32vec4 p_value,
				sol::optional<float> p_speed,
				sol::optional<int> p_min,
				sol::optional<int> p_max)
			{
				glm::i32vec4 value = p_value;
				float speed = p_speed.value_or(1.0f);
				int min = p_min.value_or(0);
				int max = p_max.value_or(0);

				const bool changed = ImGui::DragInt4(
					p_label.c_str(),
					&value.x,
					speed,
					min,
					max
				);
				return std::make_tuple(changed, value);
			}
		);

		imgui.set_function(
			"SliderFloat",
			[](const std::string &p_label, float p_value, float p_min, float p_max)
			{
				float value = p_value;

				const bool changed = ImGui::SliderFloat(
					p_label.c_str(),
					&value,
					p_min,
					p_max
				);

				return std::make_tuple(changed, value);
			}
		);

		imgui.set_function(
			"SliderInt",
			[](const std::string &p_label, int p_value, int p_min, int p_max)
			{
				int value = p_value;

				const bool changed = ImGui::SliderInt(
					p_label.c_str(),
					&value,
					p_min,
					p_max
				);

				return std::make_tuple(changed, value);
			}
		);

		imgui.set_function(
			"InputFloat",
			[](const std::string &p_label, float p_value)
			{
				float value = p_value;

				const bool changed = ImGui::InputFloat(
					p_label.c_str(),
					&value
				);

				return std::make_tuple(changed, value);
			}
		);

		imgui.set_function(
			"InputInt",
			[](const std::string &p_label, int p_value)
			{
				int value = p_value;

				const bool changed = ImGui::InputInt(
					p_label.c_str(),
					&value
				);

				return std::make_tuple(changed, value);
			}
		);

		imgui.set_function(
			"InputText",
			[](const std::string &p_label, const std::string &p_value)
			{
				std::vector<char> buffer(p_value.size() + 256, '\0');

				std::copy(p_value.begin(), p_value.end(), buffer.begin());

				const bool changed = ImGui::InputText(p_label.c_str(), buffer.data(), buffer.size());

				return std::make_tuple(changed, std::string(buffer.data()));
			}
		);

		imgui.set_function(
			"CollapsingHeader",
			[](const std::string &p_label)
			{
				return ImGui::CollapsingHeader(p_label.c_str());
			}
		);

		imgui.set_function(
			"Combo",
			[](const std::string &p_label, int p_selectedIndex, sol::table p_options)
			{
				const int optionCount = static_cast<int>(p_options.size());

				if (optionCount == 0)
				{
					return std::make_tuple(p_selectedIndex != 0, 0);
				}

				int selectedIndex = std::clamp(p_selectedIndex, 1, optionCount);

				bool changed = selectedIndex != p_selectedIndex;

				const std::string preview = p_options.get<std::string>(selectedIndex);

				if (ImGui::BeginCombo(p_label.c_str(), preview.c_str()))
				{
					for (int index = 1; index <= optionCount; ++index)
					{
						const std::string option = p_options.get<std::string>(index);

						const bool isSelected = index == selectedIndex;

						ImGui::PushID(index);

						if (ImGui::Selectable(option.c_str(), isSelected))
						{
							selectedIndex = index;
							changed = true;
						}

						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}

						ImGui::PopID();
					}

					ImGui::EndCombo();
				}

				return std::make_tuple(changed, selectedIndex);
			}
		);

		imgui.set_function(
			"Dummy",
			[](ImVec2 p_size)
			{
				ImGui::Dummy(p_size);
			}
		);
	}
}

void LuaBindings::RegisterTracy(sol::state_view p_luaState)
{
	tracy::LuaRegister(p_luaState.lua_state());

	LuaClassDefinition tracyDef;
	tracyDef.name = "tracy";

	tracyDef.functions =
	{
		{
			"ZoneBegin",
			"nil",
			true,
			{ }
		},
		{
			"ZoneBeginN",
			"nil",
			true,
			{
				{ "name", "string" }
			}
		},
		{
			"ZoneEnd",
			"nil",
			true,
			{ }
		},
		{
			"ZoneText",
			"nil",
			true,
			{
				{ "text", "string" }
			}
		},
		{
			"ZoneName",
			"nil",
			true,
			{
				{ "text", "string" }
			}
		},
		{
			"Message",
			"nil",
			true,
			{
				{ "text", "string" }
			}
		},
	};

	LuaBindings::m_luaClassDefinitions.push_back(tracyDef);
}

void LuaBindings::RegisterHealthComponent(sol::state_view p_luaState)
{
	p_luaState.new_usertype<Scene::HealthComponent>(
		"HealthComponent",
		"GetHealth", &Scene::HealthComponent::GetHealth,
		"GetMaxHealth", &Scene::HealthComponent::GetMaxHealth,
		"TakeDamage", &Scene::HealthComponent::TakeDamage,
		"Heal", &Scene::HealthComponent::Heal,
		"SetHealth", &Scene::HealthComponent::SetHealth,
		"SetMaxHealth", &Scene::HealthComponent::SetMaxHealth
	);

	LuaClassDefinition classDef;
	classDef.name = "HealthComponent";

	classDef.functions =
	{
		{ "GetHealth", "float", {} },
		{ "GetMaxHealth", "float", {} },
		{ "TakeDamage", "void", { { "amount", "number" } } },
		{ "Heal", "void", { { "amount", "number" } } },
		{ "SetHealth", "void", { { "amount", "number" } } },
		{ "SetMaxHealth", "void", { { "amount", "number" } } }
	};

	LuaBindings::m_luaClassDefinitions.push_back(classDef);
}

void LuaBindings::RegisterPlayerComponent(sol::state_view p_luaState)
{
	p_luaState.new_usertype<Scene::PlayerComponent>(
		"PlayerComponent",
		"GetPlayerBehaviour", &Scene::PlayerComponent::GetPlayerBehaviourLua
	);

	LuaClassDefinition classDef;
	classDef.name = "PlayerComponent";

	classDef.functions =
	{
		{ "GetPlayerBehaviour", "PlayerBehaviour", {} },
	};

	LuaBindings::m_luaClassDefinitions.push_back(classDef);
}

void LuaBindings::RegisterPlayerBehaviour(sol::state_view p_luaState)
{
	p_luaState.new_usertype<Scene::PlayerBehaviour>(
		"PlayerBehaviour",
		"GetMaxHealth", &Scene::PlayerBehaviour::GetMaxHealth,
		"GetHeatResistance", &Scene::PlayerBehaviour::GetHeatResistance,
		"GetHeatLevel", &Scene::PlayerBehaviour::GetHeatLevel,
		"IncreaseHeatLevel", &Scene::PlayerBehaviour::IncreaseHeatLevel,
		"SetHeatLevel", &Scene::PlayerBehaviour::SetHeatLevel
	);

	LuaClassDefinition classDef;
	classDef.name = "PlayerBehaviour";

	classDef.functions =
	{
		{ "GetMaxHealth", "float", {} },
		{ "GetHeatResistance", "float", {} },
		{ "GetHeatLevel", "float", {} },
		{ "IncreaseHeatLevel", "void", { { "amount", "number" } } },
		{ "SetHeatLevel", "HeatResistance", { { "amount", "number" } } },
	};

	LuaBindings::m_luaClassDefinitions.push_back(classDef);
}