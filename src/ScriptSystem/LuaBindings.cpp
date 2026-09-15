#include "LuaBindings.hpp"

void LuaBindings::RegisterBindings(sol::state_view p_luaState)
{
	registerNode(p_luaState);
	registerInput(p_luaState);
	registerUI(p_luaState);
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