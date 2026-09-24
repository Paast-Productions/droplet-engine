#include "LuaBindings.hpp"
#include "Node.hpp"
#include "Transform.hpp"
#include <glm/glm.hpp>

#include <print>
#include <filesystem>

using namespace Droplet;

std::vector<Script::LuaGlobalFunctionDefinition> Script::LuaBindings::m_luaGlobalDefinitions;
std::vector<Script::LuaClassDefinition> Script::LuaBindings::m_luaClassDefinitions;

void Script::LuaBindings::RegisterBindings(sol::state_view p_luaState)
{
    RegisterGlobalFunctions();
    RegisterNode(p_luaState);
    RegisterTransform(p_luaState);
    RegisterGLM(p_luaState);

	std::filesystem::path scriptDirectory = std::filesystem::current_path()
		/ ".." / ".." / ".." / "src" / "TestScripts"; // Not sure if this should be hardcoded like this :)

    std::filesystem::path path = scriptDirectory / "LuaHelper.d.lua";
	LuaApiGenerator::Generate(path, m_luaGlobalDefinitions, m_luaClassDefinitions);
}

void Script::LuaBindings::RegisterGlobalFunctions()
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
        }
    };
}
 
void Script::LuaBindings::RegisterNode(sol::state_view p_luaState)
{
    p_luaState.new_usertype<Scene::Node>(
        "Node",
        "GetName", &Droplet::Scene::Node::GetName,
        "GetTransform", static_cast<Scene::Transform &(Scene::Node::*)()>(&Scene::Node::GetTransform)
    );
}

void Script::LuaBindings::RegisterTransform(sol::state_view p_luaState)
{
    p_luaState.new_enum<Scene::Transform::Space>(
        "TransformSpace",
        {
            {"Local", Scene::Transform::Space::Local},
            {"World", Scene::Transform::Space::World}
        }
    );

    p_luaState.new_usertype<Scene::Transform>(
        "Transform",
        "GetPosition", &Scene::Transform::GetPosition,
        "GetRotation", &Scene::Transform::GetRotation,
        "GetEuler", &Scene::Transform::GetEuler,
        "GetScale", &Scene::Transform::GetScale,
        "GetMatrix", &Scene::Transform::GetMatrix,
        "IsDirty", &Scene::Transform::IsDirty,
        "GetUp", &Scene::Transform::GetUp,
        "GetRight", &Scene::Transform::GetRight,
        "GetForward", &Scene::Transform::GetForward,
        "SetPosition", &Scene::Transform::SetPosition,
        "SetRotation", &Scene::Transform::SetRotation,
        "SetEuler", &Scene::Transform::SetEuler,
        "SetScale", &Scene::Transform::SetScale,
        "SetMatrix", &Scene::Transform::SetMatrix,
        "MakeDirty", &Scene::Transform::MakeDirty,
        "RecalculateMatrices", &Scene::Transform::RecalculateMatrices,
        "Move", &Scene::Transform::Move,
        "Rotate", &Scene::Transform::Rotate,
        "RotateEuler", &Scene::Transform::RotateEuler,
        "AddScale", &Scene::Transform::AddScale,
        "RotateAxis", &Scene::Transform::RotateAxis,
        "LookAt", &Scene::Transform::LookAt
    );

    LuaClassDefinition transform;
    transform.name = "Transform";

    transform.functions =
    {
        {
            "GetPosition",
            "Vec3",
            {}
        },
        {
            "GetRotation",
            "Quat",
            {}
        },
        {
            "GetEuler",
            "Vec3",
            {}
        },
        {
            "GetScale",
            "Vec3",
            {}
        },
        {
            "GetMatrix",
            "Mat4",
            {}
        },
        {
            "IsDirty",
            "boolean",
            {}
        },
        {
            "GetUp",
            "Vec3",
            {}
        },
        {
            "GetRight",
            "Vec3",
            {}
        },
        {
            "GetForward",
            "Vec3",
            {}
        },
        {
            "SetPosition",
            "void",
            {
                { "position", "Vec3" }
            }
        },
        {
            "SetRotation",
            "void",
            {
                { "rotation", "Quat" }
            }
        },
        {
            "SetEuler",
            "void",
            {
                { "euler", "Vec3" }
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
                { "matrix", "Mat4" }
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
                { "amount", "Vec3" }
            }
        },
        {
            "Rotate",
            "void",
            {
                { "rotation", "Quat" }
            }
        },
        {
            "RotateEuler",
            "void",
            {
                { "euler", "Vec3" }
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
                { "axis", "Vec3" },
                { "angle", "number" }
            }
        },
        {
            "LookAt",
            "void",
            {
                { "target", "Vec3" }
            }
        }
    };

    LuaBindings::m_luaClassDefinitions.push_back(transform);
}

void Script::LuaBindings::RegisterGLM(sol::state_view p_luaState)
{
    p_luaState.new_usertype<glm::vec3>(
        "Vec3",
        sol::constructors<glm::vec3(float, float, float)>(),
        "x", &glm::vec3::x,
        "y", &glm::vec3::y,
        "z", &glm::vec3::z
    );

    p_luaState.new_usertype<glm::quat>(
        "Quat",
        sol::constructors<glm::quat(float, float, float, float)>(),
        "w", &glm::quat::w,
        "x", &glm::quat::x,
        "y", &glm::quat::y,
        "z", &glm::quat::z
    );

    p_luaState.new_usertype<glm::mat4>(
        "Mat4",
        sol::constructors<glm::mat4(float)>()
    );
}

