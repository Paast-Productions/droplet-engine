#include "LuaBindings.hpp"
#include "TestNode.hpp"
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
	RegisterTestNode(p_luaState);
    RegisterNode(p_luaState);
    RegisterTransform(p_luaState);
    RegisterGLM(p_luaState);

	std::filesystem::path scriptDirectory = std::filesystem::current_path()
		/ ".." / ".." / ".." / "src" / "TestScripts"; // Not sure if this should be hardcoded like this :)

    std::filesystem::path path = scriptDirectory / "ScriptSystem.d.lua";
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

void Script::LuaBindings::RegisterTestNode(sol::state_view p_luaState)
{
	p_luaState.new_usertype<TestNode>(
		"TestNode",
		"Set_position", &TestNode::setPosition,
		"Get_x", &TestNode::getX,
		"Get_y", &TestNode::getY,
		"Get_z", &TestNode::getZ,
		"Print_message", & TestNode::printMessage 
	);

	LuaClassDefinition testNode;
    testNode.functions =
    {
        {
            "Set_position",
            "void",
            {
                { "x", "number" },
                { "y", "number" },
                { "z", "number" }
            }
        },
        {
            "Get_x",
            "number",
            {}
        },
        {
            "Get_y",
            "number",
            {}
        },
        {
            "Get_z",
            "number",
            {}
        }
    };

    m_luaClassDefinitions.push_back(testNode);
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

    //LuaClassDefinition transform;
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
}

