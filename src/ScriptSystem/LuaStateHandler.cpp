#include "LuaStateHandler.hpp"
#include <tracy/public/tracy/Tracy.hpp>

using namespace Droplet::Script;

LuaStateHandler::LuaStateHandler()
{
	ZoneScoped;

	m_luaState.open_libraries(
		sol::lib::base,
		sol::lib::math,
		sol::lib::string,
		sol::lib::table
	);
}

sol::state& LuaStateHandler::GetState()
{
	return m_luaState;
}