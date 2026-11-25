#include "ComponentsBindings.hpp"
#include <SceneSystem/Components/CameraComponent.hpp>
#include <SceneSystem/Components/MeshComponent.hpp>
#include <SceneSystem/Components/PhysicsComponent.hpp>
#include <SceneSystem/Components/HealthComponent.hpp>
#include <SceneSystem/Components/DamageComponent.hpp>
#include <SceneSystem/Components/PlayerComponent.hpp>
#include <SceneSystem/Behaviours/PlayerBehaviour.hpp>
#include <SceneSystem/Components/AudioEmitterComponent.hpp>
#include <SceneSystem/Components/AudioPlayerComponent.hpp>
#include <SceneSystem/Components/AudioListenerComponent.hpp>
#include <SceneSystem/Components/UIComponent.hpp>
#include <LuaBindings.hpp>
#include <Item.hpp>
#include <ItemRegistry.hpp>

using namespace Droplet::Scene;
using namespace Droplet::Script;

void Bindings::RegisterComponents(sol::state_view p_luaState)
{
	RegisterCameraComponent(p_luaState);
	RegisterHealthComponent(p_luaState);
	RegisterPlayerComponent(p_luaState);
	RegisterPlayerBehaviour(p_luaState);
	RegisterDamageComponent(p_luaState);
    RegisterPhysicsComponent(p_luaState);
    RegisterAudioEmitterComponent(p_luaState);
}

void Bindings::RegisterCameraComponent(sol::state_view p_luaState)
{
	p_luaState.new_usertype<Common::Camera>(
		"Camera",
		"Move", &Common::Camera::Move,
		"Rotate", &Common::Camera::Rotate,
		"GetPosition", &Common::Camera::GetPosition,
		"GetForward", &Common::Camera::GetForward,
		"GetRight", &Common::Camera::GetRight,
		"GetUp", &Common::Camera::GetUp,
		"GetViewMatrix", &Common::Camera::GetViewMatrix,
		"GetProjectionMatrix", &Common::Camera::GetProjectionMatrix,
		"GetFOV", &Common::Camera::GetFOV,
		"GetAspect", &Common::Camera::GetAspect,
		"GetPlanes", &Common::Camera::GetPlanes,
		"SetPosition", &Common::Camera::SetPosition,
		"SetFOV", &Common::Camera::SetFOV
	);

	p_luaState.new_usertype<Scene::CameraComponent>(
		"CameraComponent",
		"GetCamera",
        [](Scene::CameraComponent &p_component) -> Common::Camera &
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

		"GetTarget", &Scene::CameraComponent::GetTarget,
		"SetTarget", &Scene::CameraComponent::SetTarget
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

void Bindings::RegisterMeshComponent(sol::state_view p_luaState)
{
	p_luaState.new_usertype<Scene::MeshComponent>(
		"MeshComponent",
		"SetMeshGUID", &Scene::MeshComponent::SetMeshGUID,
		"GetMeshGUID", &Scene::MeshComponent::GetMeshGUID,
		"SetTextureGUID", &Scene::MeshComponent::SetTextureGUID,
		"GetTextureGUID", &Scene::MeshComponent::GetTextureGUID,
		"SetVSGUID", &Scene::MeshComponent::SetVSGUID,
		"GetVSGUID", &Scene::MeshComponent::GetVSGUID,
		"SetTCSGUID", &Scene::MeshComponent::SetTCSGUID,
		"GetTCSGUID", &Scene::MeshComponent::GetTCSGUID,
		"SetTESGUID", &Scene::MeshComponent::SetTESGUID,
        "GetTESGUID", &Scene::MeshComponent::GetTESGUID,
		"SetGSGUID", &Scene::MeshComponent::SetGSGUID,
		"GetGSGUID", &Scene::MeshComponent::GetGSGUID,
		"HasBoundsOverride", &Scene::MeshComponent::HasBoundsOverride,
		"GetsBounds", &Scene::MeshComponent::GetBounds
	);

    LuaClassDefinition classDef;
    classDef.name = "MeshComponent";

    classDef.functions = {
        {"GetTypeName", "string", {}},

        {"SetMeshGUID", "void", {{"guid", "GUID"}}},
        {"GetMeshGUID", "GUID", {}},

        {"SetTextureGUID", "void", {{"guid", "GUID"}}},
        {"GetTextureGUID", "GUID", {}},

        {"SetVSGUID", "void", {{"guid", "GUID"}}},
        {"GetVSGUID", "GUID", {}},

        {"SetTCSGUID", "void", {{"guid", "GUID"}}},
        {"GetTCSGUID", "GUID", {}},

        {"SetTESGUID", "void", {{"guid", "GUID"}}},
        {"GetTESGUID", "GUID", {}},

        {"SetGSGUID", "void", {{"guid", "GUID"}}},
        {"GetGSGUID", "GUID", {}},

        {"SetFSGUID", "void", {{"guid", "GUID"}}},
        {"GetFSGUID", "GUID", {}},

        {"HasBoundsOverride", "boolean", {}},
        {"GetBounds", "NodeBounds", {}}
    };

    LuaBindings::PushLuaClassDefinition(classDef);
}

void Bindings::RegisterPhysicsComponent(sol::state_view p_luaState)
{
	p_luaState.new_enum<PhysicsComponent::ShapeType>(
		"ShapeType",
		{
            {"None", PhysicsComponent::ShapeType::None},
            {"Box", PhysicsComponent::ShapeType::Box},
            {"Sphere",  PhysicsComponent::ShapeType::Sphere},
            {"Capsule", PhysicsComponent::ShapeType::Capsule},
            {"Mesh", PhysicsComponent::ShapeType::Mesh},
            {"ConvexHull", PhysicsComponent::ShapeType::ConvexHull},
            {"Compound", PhysicsComponent::ShapeType::Compound},
            {"Player", PhysicsComponent::ShapeType::Player}  
		}
	);

    p_luaState.new_usertype<PhysicsComponent>(
        "PhysicsComponent",

        "GetTypeName", &PhysicsComponent::GetTypeName,

        // Collider configuration
        "SetBoxCollider",
        [](PhysicsComponent &p_component,
            const glm::vec3 &p_halfExtents,
            const glm::vec3 &p_offset)
        {
            p_component.SetBoxCollider(
                p_halfExtents,
                GLMtoJPH(p_offset)
            );
        },

        "SetSphereCollider",
        [](PhysicsComponent &p_component,
            float p_radius,
            const glm::vec3 &p_offset)
        {
            p_component.SetSphereCollider(
                p_radius,
                GLMtoJPH(p_offset)
            );
        },

        "SetCapsuleCollider",
        [](PhysicsComponent &p_component,
            float p_halfHeight,
            float p_radius,
            const glm::vec3 &p_offset)
        {
            p_component.SetCapsuleCollider(
                p_halfHeight,
                p_radius,
                GLMtoJPH(p_offset)
            );
        },

        "SetMeshCollider",
        [](PhysicsComponent &p_component,
            GUID p_meshGuid,
            const glm::vec3 &p_offset)
        {
            p_component.SetMeshCollider(
                p_meshGuid,
                GLMtoJPH(p_offset)
            );
        },

        "SetConvexHullCollider",
        [](PhysicsComponent &p_component,
            GUID p_meshGuid,
            float p_edgeRadius,
            const glm::vec3 &p_offset)
        {
            p_component.SetConvexHullCollider(
                p_meshGuid,
                p_edgeRadius,
                GLMtoJPH(p_offset)
            );
        },

        "SetPlayerCollider",
        [](PhysicsComponent &p_component,
            float p_halfHeight,
            float p_radius,
            const glm::vec3 &p_offset)
        {
            p_component.SetPlayerCollider(
                p_halfHeight,
                p_radius,
                GLMtoJPH(p_offset)
            );
        },

        // Body configuration
        "SetMotionType", &PhysicsComponent::SetMotionType,
        "SetObjectLayer", &PhysicsComponent::SetObjectLayer,
        "SetActive", &PhysicsComponent::SetActive,
        "RemoveCollider", &PhysicsComponent::RemoveCollider,
        
        "SetVelocity", &PhysicsComponent::SetVelocity,
        "GetVelocity", &PhysicsComponent::GetVelocity
    );


    LuaClassDefinition classDef;
    classDef.name = "PhysicsComponent";

    classDef.functions = {
        {"GetTypeName", "string", {}},

        {"SetBoxCollider", "void", {
            {"halfExtents", "Vec3"},
            {"offset", "Vec3"}
        }},

        {"SetSphereCollider", "void", {
            {"radius", "number"},
            {"offset", "Vec3"}
        }},

        {"SetCapsuleCollider", "void", {
            {"halfHeight", "number"},
            {"radius", "number"},
            {"offset", "Vec3"}
        }},

        {"SetMeshCollider", "void", {
            {"meshGuid", "GUID"},
            {"offset", "Vec3"}
        }},

        {"SetConvexHullCollider", "void", {
            {"meshGuid", "GUID"},
            {"edgeRadius", "number"},
            {"offset", "Vec3"}
        }},

        {"SetPlayerCollider", "void", {
            {"halfHeight", "number"},
            {"radius", "number"},
            {"offset", "Vec3"}
        }},

        {"SetMotionType", "void", {
            {"motionType", "MotionType"}
        }},

        {"SetObjectLayer", "void", {
            {"objectLayer", "Layers"}
        }},

        {"SetActive", "void", {
            {"active", "boolean"}
        }},

        {"RemoveCollider", "void", {}},
                
        {"SetVelocity", "void", {
            {"velocity", "Vec3"}
        }},
                
        {"GetVelocity", "Vec3", {}}
    };

	LuaBindings::PushLuaClassDefinition(classDef);
}

void Bindings::RegisterHealthComponent(sol::state_view p_luaState)
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

void Bindings::RegisterDamageComponent(sol::state_view p_luaState)
{
	p_luaState.new_usertype<Scene::DamageComponent>(
		"DamageComponent",
		"GetDamage", &Scene::DamageComponent::GetDamageLua
	);

    LuaClassDefinition classDef;
    classDef.name = "DamageComponent";

    classDef.functions =
    {
        { "GetDamage", "float", {} },
    };

    LuaBindings::PushLuaClassDefinition(classDef);
}

void Bindings::RegisterPlayerComponent(sol::state_view p_luaState)
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


void Bindings::RegisterPlayerBehaviour(sol::state_view p_luaState)
{
    // Item identifier.
    p_luaState.new_usertype<Game::ItemId>(
        "ItemId",
        sol::no_constructor,
        "IsValid", &Game::ItemId::IsValid,
        "value", &Game::ItemId::value
    );

    // Inventory type.
    p_luaState.new_enum(
        "InventoryType",
        "Player", Scene::InventoryType::Player,
        "Drone", Scene::InventoryType::Drone
    );

    // Item registry.
    p_luaState.new_usertype<Game::ItemRegistry>(
        "ItemRegistry",
        sol::no_constructor,
        "Find", [](const Game::ItemRegistry &p_registry, const std::string& p_name)
        {
            return p_registry.Find(p_name);
        }
    );
	
	p_luaState.new_usertype<Scene::PlayerBehaviour>(
		"PlayerBehaviour",
		"GetMaxHealth", &Scene::PlayerBehaviour::GetMaxHealth,
		
        "GetHeatResistance", &Scene::PlayerBehaviour::GetHeatResistance,
		"SetHeatResistance", & Scene::PlayerBehaviour::SetHeatResistance,
		"GetHeatLevel", &Scene::PlayerBehaviour::GetHeatLevel,
		"IncreaseHeatLevel", &Scene::PlayerBehaviour::IncreaseHeatLevel,
		"SetHeatLevel", &Scene::PlayerBehaviour::SetHeatLevel,
		
        "GetFreezeResistance", &Scene::PlayerBehaviour::GetFreezeResistance,
        "GetFreezeLevel", &Scene::PlayerBehaviour::GetFreezeLevel,
        "SetFreezeResistance", & Scene::PlayerBehaviour::SetFreezeResistance,
        "IncreaseFreezeLevel", &Scene::PlayerBehaviour::IncreaseFreezeLevel,
        "SetFreezeLevel", &Scene::PlayerBehaviour::SetFreezeLevel,
        "SetFreezeTimer", &Scene::PlayerBehaviour::SetFreezeTimer,
        "GetFreezeTimer", &Scene::PlayerBehaviour::GetFreezeTimer,

        "GetPressureResistance", &Scene::PlayerBehaviour::GetPressureResistance,
        "SetPressureResistance", &Scene::PlayerBehaviour::SetPressureResistance,
        "GetPressureLevel", &Scene::PlayerBehaviour::GetPressureLevel,
        "SetPressureLevel", &Scene::PlayerBehaviour::SetPressureLevel,
        "IncreasePressureLevel", &Scene::PlayerBehaviour::IncreasePressureLevel,

        "GetItemRegistry", &Scene::PlayerBehaviour::GetItemRegistry,
        "AddItem", &Scene::PlayerBehaviour::AddItem,
        "RemoveItem", &Scene::PlayerBehaviour::RemoveItem,
        "CountItem", &Scene::PlayerBehaviour::CountItem,
        "GetUniqueID", & Scene::PlayerBehaviour::GetUniqueID,

        "SetDroneInventorySize", &Scene::PlayerBehaviour::SetDroneInventorySize,
        "GetDroneInventorySize", &Scene::PlayerBehaviour::GetDroneInventorySize,

		"SetDroneMaxSpeed", &Scene::PlayerBehaviour::SetDroneMaxSpeed,
        "GetDroneMaxSpeed", &Scene::PlayerBehaviour::GetDroneMaxSpeed,
        "SetDroneMaxRange", &Scene::PlayerBehaviour::SetDroneMaxRange,
        "GetDroneMaxRange", &Scene::PlayerBehaviour::GetDroneMaxRange

	);

    // PlayerBehaviour API metadata.
    LuaClassDefinition classDef;
    classDef.name = "PlayerBehaviour";
    classDef.functions =
    {
        { "GetMaxHealth", "float", {} },
        { "GetHeatResistance", "float", {} },
        { "GetHeatLevel", "float", {} },
        { "IncreaseHeatLevel", "void", { { "amount", "number" } } },
        { "SetHeatLevel", "void", { { "heat", "number" } } },
        { "SetHeatResistance", "void", { { "resistance", "number" } } },
        { "GetFreezeResistance", "float", {} },
        { "GetFreezeLevel", "float", {} },
        { "IncreaseFreezeLevel", "void", { { "amount", "number" } } },
        { "SetFreezeLevel", "FreezeResistance", { { "amount", "number" } } },
        { "SetFreezeTimer", "Freezetimer", {{"amount", "number" } } },
        { "GetFreezeTimer", "float", {} },
		{ "GetItemRegistry", "ItemRegistry", {} },
        { "AddItem", "number", {{ "inventory", "InventoryType" },{ "itemId", "ItemId" },{ "count", "number" }} },
        { "RemoveItem", "boolean", {{ "inventory", "InventoryType" },{ "itemId", "ItemId" },{ "count", "number" }} },
        { "CountItem", "number", {{ "inventory", "InventoryType" },{ "itemId", "ItemId" }} },
		{ "GetPressureResistance", "float", {} },
        { "SetPressureResistance", "void", { { "amount", "number" } } },
        { "GetPressureLevel", "float", {} },
        { "SetPressureLevel", "void", { { "amount", "number" } } },
        { "IncreasePressureLevel", "void", { { "amount", "number" } } },
        { "GetRadiationResistance", "float", {} },
        { "SetRadiationResistance", "void", { { "amount", "number" } } },
        { "GetRadiationLevel", "float", {} },
        { "SetRadiationLevel", "void", { { "amount", "number" } } },
		{ "IncreaseRadiationLevel", "void", { { "amount", "number" } } },
		{ "SetDroneInventorySize", "void", { { "amount", "number" } } },
		{ "GetDroneInventorySize", "float", {} },
		{ "SetDroneMaxSpeed", "void", { { "amount", "number" } } },
        { "GetDroneMaxSpeed", "float", {} },
		{ "SetDroneMaxRange", "void", { { "amount", "number" } } },
        { "GetDroneMaxRange", "float", {} },
		
	};

	LuaBindings::PushLuaClassDefinition(classDef);

    // ItemRegistry API metadata.
    LuaClassDefinition registryDef;
    registryDef.name = "ItemRegistry";

    registryDef.functions =
    {
        { "Find", "ItemId", { { "name", "string" } } }
    };

    LuaBindings::PushLuaClassDefinition(registryDef);
}

void Bindings::RegisterAudioEmitterComponent(sol::state_view p_luaState)
{
    p_luaState.new_enum<dalia::AttenuationCurve>(
        "AttenuationCurve",
        {
            {"InverseSquare", dalia::AttenuationCurve::InverseSquare},
            {"Linear", dalia::AttenuationCurve::Linear},
            {"Quadratic", dalia::AttenuationCurve::Quadratic}
        }
    );

    p_luaState.new_usertype<AudioEmitterComponent>(
        "AudioEmitterComponent",
        "Play", &AudioEmitterComponent::Play,
        "Pause", &AudioEmitterComponent::Pause,
        "Resume", &AudioEmitterComponent::Resume,
        "Stop", &AudioEmitterComponent::Stop,
        "IsPlaying", &AudioEmitterComponent::IsPlaying,
        "SetStartTimeSeconds", &AudioEmitterComponent::SetStartTimeSeconds,
        "SetVolumeDb", &AudioEmitterComponent::SetVolumeDb,
        "SetPitch", &AudioEmitterComponent::SetPitch,
        "SetLooping", &AudioEmitterComponent::SetLooping,
        "SetMinMaxDistance", &AudioEmitterComponent::SetMinMaxDistance,
        "SetAttenuationCurve", &AudioEmitterComponent::SetAttenuationCurve,
        "GetStartTimeSeconds", &AudioEmitterComponent::GetStartTimeSeconds,
        "GetVolumeDb", &AudioEmitterComponent::GetVolumeDb,
        "GetPitch", &AudioEmitterComponent::GetPitch,
        "IsLooping", &AudioEmitterComponent::IsLooping,
        "GetMinDistance", &AudioEmitterComponent::GetMinDistance,
        "GetMaxDistance", &AudioEmitterComponent::GetMaxDistance,
        "GetAttenuationCurve", &AudioEmitterComponent::GetAttenuationCurve
    );



    LuaClassDefinition classDef;
    classDef.name = "AudioEmitterComponent";

    classDef.functions =
    {
        {"Play", "void", {}},
        {"Pause", "void", {}},
        {"Resume", "void", {}},
        {"Stop", "void", {}},
        {"IsPlaying", "bool", {}},
        {"SetStartTimeSeconds", "void", {{"startTime","double"}}},
        {"SetVolumeDb", "void", {{"volume", "float"}}},
        {"SetPitch", "void", {{"pitch", "float"}} },
        {"SetLooping", "void", {{"isLooping", "bool"}}},
        {"SetMinMaxDistance", "void",{} },
        {"SetAttenuationCurve", "void", {{"daliaAttenuationcurve", "daliaattenuationCurve"}} },
        {"GetStartTimeSeconds", "double", {}},
        {"GetVolumeDb", "float", {}},
        {"GetPitch", "float", {}},
        {"IsLooping", "bool", {}},
        {"GetMinDistance", "float", {}},
        {"GetMaxDistance", "float", {}},
        {"GetAttenuationCurve", "daliaattenuationCurve",{}},
    };

    LuaBindings::PushLuaClassDefinition(classDef);
}

void Bindings::RegisterAudioPlayerComponent(sol::state_view p_luaState)
{
    p_luaState.new_usertype<AudioPlayerComponent>(
        "AudioPlayerComponent",
        "Play", &AudioPlayerComponent::Play,
        "Pause", &AudioPlayerComponent::Pause,
        "Resume", &AudioPlayerComponent::Resume,
        "Stop", &AudioPlayerComponent::Stop,
        "IsPlaying", &AudioPlayerComponent::IsPlaying,
        "SetStartTimeSeconds", &AudioPlayerComponent::SetStartTimeSeconds,
        "SetVolumeDb", &AudioPlayerComponent::SetVolumeDb,
        "SetStereoPan", &AudioPlayerComponent::SetStereoPan,
        "SetPitch", &AudioPlayerComponent::SetPitch,
        "SetLooping", &AudioPlayerComponent::SetLooping,
        "GetStartTimeSeconds", &AudioPlayerComponent::GetStartTimeSeconds,
        "GetVolumeDb", &AudioPlayerComponent::GetVolumeDb,
        "GetPitch", &AudioPlayerComponent::GetPitch,
        "IsLooping", &AudioPlayerComponent::IsLooping,
        "GetStereoPan", &AudioPlayerComponent::GetStereoPan
    );

    LuaClassDefinition classDef;
    classDef.name = "AudioPlayerComponent";

    classDef.functions =
    {
        {"Play", "void", {}},
        {"Pause", "void", {}},
        {"Resume", "void", {}},
        {"Stop", "void", {}},
        {"IsPlaying", "bool", {}},
        {"SetStartTimeSeconds", "void", {{"Seconds", "float"}}},
        {"SetVolumeDb", "void", {{"volume", "float"}}},
        {"SetPitch", "void", {{"pitch", "float"}}},
        {"SetLooping", "void", {{"islooping", "bool"}}},
        {"GetStartTimeseconds", "float", {}},
        {"GetVolumeDb", "float", {}},
        {"GetPitch", "float",{}},
        {"IsLooping", "bool", {}},
        {"GetStereoPan", "float", {}},
    };

    LuaBindings::PushLuaClassDefinition(classDef);
}
