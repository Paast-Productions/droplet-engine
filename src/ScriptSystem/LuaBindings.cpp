#include "LuaBindings.hpp"
#include "TestNode.hpp"

void LuaBindings::RegisterBindings(sol::state_view p_luaState)
{
	RegisterTestNode(p_luaState);
}

void LuaBindings::RegisterTestNode([[maybe_unused]] sol::state_view p_luaState)
{
	//p_luaState.new_usertype<TestNode>(
	//	"TestNode",
	//	"set_position", & TestNode::setPosition,
	//	"get_x", & TestNode::getX,
	//	"get_y", & TestNode::getY,
	//	"get_z", & TestNode::getZ,
	//	"print_message", & TestNode::printMessage
	//);
}
