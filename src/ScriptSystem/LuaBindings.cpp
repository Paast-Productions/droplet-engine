#include "LuaBindings.hpp"
#include "TestNode.hpp"
#include "Node.hpp"
#include "Transform.hpp"

#include <print>
#include <filesystem>

using namespace Droplet::Scene;

std::vector<LuaGlobalFunctionDefinition> LuaBindings::m_luaGlobalDefinitions;
std::vector<LuaClassDefinition> LuaBindings::m_luaClassDefinitions;

void LuaBindings::RegisterBindings(sol::state_view p_luaState) 
{
    RegisterGlobalFunctions();
	RegisterTestNode(p_luaState);
    RegisterNode(p_luaState);
    RegisterTransform(p_luaState);

	std::filesystem::path scriptDirectory = std::filesystem::current_path()
		/ ".." / ".." / ".." / "src" / "TestScripts"; // Not sure if this should be hardcoded like this :)

    std::filesystem::path path = scriptDirectory / "ScriptSystem.d.lua";
	LuaApiGenerator::Generate(path, m_luaGlobalDefinitions, m_luaClassDefinitions);
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
        }
    };
}

void LuaBindings::RegisterTestNode(sol::state_view p_luaState)
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
 
void LuaBindings::RegisterNode(sol::state_view p_luaState)
{
    p_luaState.new_usertype<Node>(
        "Node",
        "GetName", &Node::GetName,
        "GetTransform", static_cast<Transform &(Node::*)()>(&Node::GetTransform)
    );
}

void LuaBindings::RegisterTransform(sol::state_view p_luaState)
{
    p_luaState.new_usertype<Transform>(
        "Transform",
        "GetPosition", &Transform::GetPosition,
        "GetRotation", &Transform::GetRotation,
        "GetEuler", &Transform::GetEuler,
        "GetScale", &Transform::GetScale,
        "GetMatrix", &Transform::GetMatrix,
        "IsDirty", &Transform::IsDirty,
        "GetUp", &Transform::GetUp,
        "GetRight", &Transform::GetRight,
        "GetForward", &Transform::GetForward,
        "SetPosition", &Transform::SetPosition,
        "SetRotation", &Transform::SetRotation,
        "SetEuler", &Transform::SetEuler,
        "SetScale", &Transform::SetScale,
        "SetMatrix", &Transform::SetMatrix,
        "MakeDirty", &Transform::MakeDirty,
        "RecalculateMatrices", &Transform::RecalculateMatrices,
        "Move", &Transform::Move,
        "Rotate", &Transform::Rotate,
        "RotateEuler", &Transform::RotateEuler,
        "AddScale", &Transform::AddScale,
        "RotateAxis", &Transform::RotateAxis,
        "LookAt", &Transform::LookAt
    );

    //LuaClassDefinition transform;
}

