#include "LuaBindings.hpp"
#include "TestNode.hpp"
#include <print>
#include <filesystem>

std::vector<LuaClassDefinition> LuaBindings::m_luaClassDefinitions;

void LuaBindings::RegisterBindings(sol::state_view p_luaState) 
{
	RegisterTestNode(p_luaState);

	std::filesystem::path scriptDirectory = std::filesystem::current_path()
		/ ".." / ".." / ".." / "src" / "TestScripts"; // Not sure if this should be hardcoded like this :)

	LuaApiGenerator::Generate(scriptDirectory / "ScriptSystem.d.lua", m_luaClassDefinitions);
}

void LuaBindings::RegisterTestNode(sol::state_view p_luaState)
{
	p_luaState.new_usertype<TestNode>(
		"TestNode",
		"set_position", &TestNode::setPosition,
		"get_x", &TestNode::getX,
		"get_y", &TestNode::getY,
		"get_z", &TestNode::getZ,
		"print_message", & TestNode::printMessage 
	);

	LuaClassDefinition testNode;
    testNode.functions =
    {
        {
            "set_position",
            "void",
            {
                { "x", "number" },
                { "y", "number" },
                { "z", "number" }
            }
        },
        {
            "get_x",
            "number",
            {}
        },
        {
            "get_y",
            "number",
            {}
        },
        {
            "get_z",
            "number",
            {}
        }
    };

    m_luaClassDefinitions.push_back(testNode);
}
