#include "LuaBindings.hpp"
#include "TestNode.hpp"
#include <print>
#include <filesystem>

std::vector<LuaGlobalFunctionDefinition> LuaBindings::m_luaGlobalDefinitions;
std::vector<LuaClassDefinition> LuaBindings::m_luaClassDefinitions;

void LuaBindings::RegisterBindings(sol::state_view p_luaState) 
{
    RegisterGlobalFunctions(p_luaState);
	RegisterTestNode(p_luaState);

	std::filesystem::path scriptDirectory = std::filesystem::current_path()
		/ ".." / ".." / ".." / "src" / "TestScripts"; // Not sure if this should be hardcoded like this :)

    std::filesystem::path path = scriptDirectory / "ScriptSystem.d.lua";
	LuaApiGenerator::Generate(path, m_luaGlobalDefinitions, m_luaClassDefinitions);
}

void LuaBindings::RegisterGlobalFunctions([[maybe_unused]] sol::state_view p_luaState)
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

