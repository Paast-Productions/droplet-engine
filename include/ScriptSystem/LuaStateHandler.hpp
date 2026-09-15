#pragma once

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