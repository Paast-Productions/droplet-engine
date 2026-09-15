#pragma once

#include <sol/sol.hpp>

class LuaBindings
{
public:
	static void RegisterBindings(sol::state_view p_luaState);

private:
	static void registerNode(sol::state_view p_luaState);
	static void registerInput(sol::state_view p_luaState);
	static void registerUI(sol::state_view p_luaState);

};