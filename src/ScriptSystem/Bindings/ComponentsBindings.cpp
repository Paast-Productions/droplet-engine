#include "ComponentsBindings.hpp"
#include "SceneSystem/Components/CameraComponent.hpp"
#include "SceneSystem/Components/HealthComponent.hpp"
#include "SceneSystem/Components/PlayerComponent.hpp"
#include "SceneSystem/Behaviours/PlayerBehaviour.hpp"
#include <LuaBindings.hpp>


using namespace Droplet;

void Script::Bindings::RegisterComponents(sol::state_view p_luaState)
{
	RegisterCameraComponent(p_luaState);
	RegisterHealthComponent(p_luaState);
	RegisterPlayerComponent(p_luaState);
	RegisterPlayerBehaviour(p_luaState);
}

void Script::Bindings::RegisterCameraComponent(sol::state_view p_luaState)
{
	p_luaState.new_usertype<Graphics::Camera>(
		"Camera",
		"Move", &Graphics::Camera::Move,
		"Rotate", &Graphics::Camera::Rotate,
		"GetPosition", &Graphics::Camera::GetPosition,
		"GetForward", &Graphics::Camera::GetForward,
		"GetRight", &Graphics::Camera::GetRight,
		"GetUp", &Graphics::Camera::GetUp,
		"GetViewMatrix", &Graphics::Camera::GetViewMatrix,
		"GetProjectionMatrix", &Graphics::Camera::GetProjectionMatrix,
		"GetFOV", &Graphics::Camera::GetFOV,
		"GetAspect", &Graphics::Camera::GetAspect,
		"GetPlanes", &Graphics::Camera::GetPlanes,
		"SetPosition", &Graphics::Camera::SetPosition,
		"SetFOV", &Graphics::Camera::SetFOV
	);

	p_luaState.new_usertype<Scene::CameraComponent>(
		"CameraComponent",
		"GetCamera",
        [](Scene::CameraComponent &p_component) -> Graphics::Camera &
        {
            return p_component.GetCamera();
        },

        "GetFrustum",
        [](Scene::CameraComponent &p_component) -> Math::Frustum &
        {
            return p_component.GetFrustum();
        },

		"GetDrawLayerMask", &Scene::CameraComponent::GetDrawLayerMask,

		"GetPriority", &Scene::CameraComponent::GetPriority,
		"SetPriority", &Scene::CameraComponent::SetPriority,

		"GetResolution", &Scene::CameraComponent::GetResolution,
		"SetResolution", &Scene::CameraComponent::SetResolution,

		"IsAutoRenderEnabled", &Scene::CameraComponent::IsAutoRenderEnabled,
		"SetAutoRender", &Scene::CameraComponent::SetAutoRender,
		"TriggerRender", &Scene::CameraComponent::TriggerRender,

		"GetTargetIndex", &Scene::CameraComponent::GetTargetIndex,
		"SetTargetIndex", &Scene::CameraComponent::SetTargetIndex,

		"GetTextureIndex", &Scene::CameraComponent::GetTextureIndex,
		"SetTextureIndex", &Scene::CameraComponent::SetTextureIndex
	);

    LuaClassDefinition classDef;
    classDef.name = "Camera";

    classDef.functions =
    {
        {
            "Move",
            "void",
            {
                {"direction", "Vec3"},
                {"deltaTime", "number"}
            }
        },
        {
            "Rotate",
            "void",
            {
                {"xOffset", "number"},
                {"yOffset", "number"},
                {"up", "Vec3"},
                {"constrainPitch", "boolean"}
            }
        },
        {
            "GetPosition",
            "Vec3",
            {}
        },
        {
            "GetForward",
            "Vec3",
            {}
        },
        {
            "GetRight",
            "Vec3",
            {}
        },
        {
            "GetUp",
            "Vec3",
            {}
        },
        {
            "GetViewMatrix",
            "Mat4",
            {}
        },
        {
            "GetProjectionMatrix",
            "Mat4",
            {}
        },
        {
            "GetFOV",
            "number",
            {}
        },
        {
            "GetAspect",
            "number",
            {}
        },
        {
            "GetPlanes",
            "Vec2",
            {}
        },
        {
            "SetPosition",
            "void",
            {
                {"position", "Vec3"}
            }
        },
        {
            "SetFOV",
            "void",
            {
                {"fov", "number"}
            }
        },
        {
            "SetOrientationYawPitch",
            "void",
            {
                {"yaw", "number"},
                {"pitch", "number"}
            }
        },
        {
            "SetOrientationVectors",
            "void",
            {
                {"forward", "Vec3"},
                {"up", "Vec3"}
            }
        },
        {
            "SetOrientationQuaternion",
            "void",
            {
                {"orientation", "Quat"}
            }
        }
    };

    LuaBindings::PushLuaClassDefinition(classDef);

    classDef = {};
    classDef.name = "CameraComponent";

    classDef.functions =
    {
        {
            "GetCamera",
            "Camera",
            {}
        },
        {
            "GetDrawLayerMask",
            "DrawLayer",
            {}
        },
        {
            "GetPriority",
            "integer",
            {}
        },
        {
            "SetPriority",
            "void",
            {
                {"priority", "integer"}
            }
        },
        {
            "GetResolution",
            "Vec2i",
            {}
        },
        {
            "SetResolution",
            "void",
            {
                {"resolution", "Vec2i"}
            }
        },
        {
            "IsAutoRenderEnabled",
            "boolean",
            {}
        },
        {
            "SetAutoRender",
            "void",
            {
                {"enabled", "boolean"}
            }
        },
        {
            "TriggerRender",
            "void",
            {}
        },
        {
            "GetTargetIndex",
            "integer",
            {}
        },
        {
            "SetTargetIndex",
            "void",
            {
                {"renderTarget", "integer"}
            }
        },
        {
            "GetTextureIndex",
            "integer",
            {}
        },
        {
            "SetTextureIndex",
            "void",
            {
                {"textureIndex", "integer"}
            }
        },
        {
            "SetAspect",
            "void",
            {
                {"aspect", "number"}
            }
        },
        {
            "SetPlanes",
            "void",
            {
                {"nearPlane", "number"},
                {"farPlane", "number"}
            }
        },
        {
            "GetViewRay",
            "Ray",
            {
                {"viewPosition", "Vec2"}
            }
        },
        {
            "ViewRectToWorldTile",
            "Frustum",
            {
                {"minMax", "Vec4"}
            }
        }
    };

    LuaBindings::PushLuaClassDefinition(classDef);
}

void Script::Bindings::RegisterHealthComponent(sol::state_view p_luaState)
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

	LuaBindings::PushLuaClassDefinition(classDef);

}

void Script::Bindings::RegisterPlayerComponent(sol::state_view p_luaState)
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

	LuaBindings::PushLuaClassDefinition(classDef);
}

void Script::Bindings::RegisterPlayerBehaviour(sol::state_view p_luaState)
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

	LuaBindings::PushLuaClassDefinition(classDef);
}