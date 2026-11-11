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
		sol::lib::table,
		sol::lib::package,
		sol::lib::jit,
		sol::lib::ffi
	);
	
	m_luaState.script("print(jit.version)");
	std::flush(std::cout);
}

Droplet::Script::LuaStateHandler::~LuaStateHandler()
{
	ZoneScoped;

	Shutdown();
}

void LuaStateHandler::Shutdown()
{
	if (m_isShutdown)
	{
		return;
	}

	m_isShutdown = true;

	m_luaState = sol::state{};
}

sol::state& LuaStateHandler::GetState()
{
	return m_luaState;
}