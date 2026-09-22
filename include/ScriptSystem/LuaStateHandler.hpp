#pragma once
#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

class LuaStateHandler
{
public:
	LuaStateHandler();
	~LuaStateHandler() = default;

	sol::state& GetState();

private:
	sol::state m_luaState;
};