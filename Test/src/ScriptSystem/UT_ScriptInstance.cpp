#include <gtest/gtest.h>
#include <ScriptSystem/ScriptInstance.hpp>
#include <sol/sol.hpp>

TEST(ScriptInstance, onStart)
{
	//Lua state initialization
	sol::state luaState;
	luaState.open_libraries(
		sol::lib::base,
		sol::lib::math,
		sol::lib::string,
		sol::lib::table
	);

	sol::load_result loadResult = luaState.load_file("testScript.lua");
}
