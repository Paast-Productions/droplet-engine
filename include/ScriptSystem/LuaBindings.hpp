#pragma once
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>
#include "LuaStateHandler.hpp"

class LuaBindings
{
public:
	static void RegisterBindings(sol::state_view p_luaState);

private:
	static void RegisterTestNode(sol::state_view p_luaState);
	//Implement all after added in the engine
};