#include "LuaBindings.hpp"
#include "TestNode.hpp"

void LuaBindings::RegisterBindings(sol::state_view p_luaState)
{
	registerTestNode(p_luaState);
	registerNode(p_luaState);
	registerInput(p_luaState);
	registerUI(p_luaState);
}

void LuaBindings::registerTestNode(sol::state_view p_luaState)
{
	p_luaState.new_usertype<TestNode>(
		"TestNode",
		"set_position", & TestNode::setPosition,
		"get_x", & TestNode::getX,
		"get_y", & TestNode::getY,
		"get_z", & TestNode::getZ,
		"print_message", & TestNode::printMessage
	);
}

void LuaBindings::registerNode(sol::state_view p_luaState)
{
	// When scene system is available
} 

void LuaBindings::registerInput(sol::state_view p_luaState)
{
	// When input system is available
}

void LuaBindings::registerUI(sol::state_view p_luaState)
{
	// When UI system is available
}