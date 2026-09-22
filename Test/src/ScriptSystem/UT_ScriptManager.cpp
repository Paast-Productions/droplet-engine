#include <gtest/gtest.h>
#include <ScriptSystem/ScriptManager.hpp>
#include <ScriptSystem/LuaBindings.hpp>

TEST(ScriptManager, CreateScript)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	ScriptManager manager(stateHandler);
	TestNode testNode;

	ScriptInstance* instance = manager.CreateScript(&testNode, "testScript.lua");

	EXPECT_NE(instance, nullptr);
}

TEST(ScriptManager, DestroyScript)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	ScriptManager manager(stateHandler);
	TestNode testNode;

	ScriptInstance* instance = manager.CreateScript(&testNode, "testScript.lua");
	manager.DestroyScript(instance);

	EXPECT_EQ(instance, nullptr);
}

TEST(ScriptManager, LoadScript)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	ScriptManager manager(stateHandler);
	TestNode testNode;

	EXPECT_TRUE(manager.LoadScript("testScript.lua"));
	EXPECT_TRUE(manager.LoadScript("bruh.lua"));
	EXPECT_TRUE(manager.LoadScript("testScript2.lua"));
}

TEST(ScriptManager, UnloadScript)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	ScriptManager manager(stateHandler);
	TestNode testNode;

	manager.LoadScript("testScript.lua");
	manager.LoadScript("bruh.lua");
	manager.LoadScript("testScript2.lua");

	EXPECT_TRUE(manager.UnloadScript("testScript.lua"));
	EXPECT_TRUE(manager.UnloadScript("bruh.lua"));
	EXPECT_TRUE(manager.UnloadScript("testScript2.lua"));
}

TEST(ScriptManager, IsLoaded)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	ScriptManager manager(stateHandler);
	TestNode testNode;

	manager.LoadScript("testScript.lua");
	manager.LoadScript("bruh.lua");
	manager.LoadScript("testScript2.lua");

	EXPECT_TRUE(manager.IsLoaded("testScript.lua"));
	EXPECT_TRUE(manager.IsLoaded("bruh.lua"));
	EXPECT_TRUE(manager.IsLoaded("testScript2.lua"));
}

TEST(ScriptManager, ReloadScript)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	ScriptManager manager(stateHandler);
	TestNode testNode;

	manager.LoadScript("testScript.lua");
	manager.LoadScript("bruh.lua");
	manager.LoadScript("testScript2.lua");

	EXPECT_TRUE(manager.ReloadScript("testScript.lua"));
	EXPECT_TRUE(manager.ReloadScript("bruh.lua"));
	EXPECT_TRUE(manager.ReloadScript("testScript2.lua"));
}

TEST(ScriptManager, GetLoadedScript)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	ScriptManager manager(stateHandler);
	TestNode testNode;

	manager.LoadScript("testScript.lua");
	manager.LoadScript("bruh.lua");
	manager.LoadScript("testScript2.lua");

	sol::load_result* result1 = manager.GetLoadedScript("testScript.lua");
	sol::load_result* result2 = manager.GetLoadedScript("bruh.lua");
	sol::load_result* result3 = manager.GetLoadedScript("testScript2.lua");

	EXPECT_TRUE(result1->valid());
	EXPECT_TRUE(result2->valid());
	EXPECT_TRUE(result3->valid());
}

TEST(ScriptManager, ActivateScript)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	ScriptManager manager(stateHandler);
	TestNode testNode;

	manager.LoadScript("bruh.lua");
	manager.ActivateScript(&testNode);

	auto result = manager.Call(&testNode, "timesTwo", 1);
	int timesTwo = manager.Call(&testNode, "timesTwo", 1);

	EXPECT_TRUE(result.valid());
	EXPECT_EQ(timesTwo, 2);
}

TEST(ScriptManager, DeactivateScript)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	ScriptManager manager(stateHandler);
	TestNode testNode;

	manager.LoadScript("bruh.lua");
	manager.ActivateScript(&testNode);

	int timesTwo = manager.Call(&testNode, "timesTwo", 1);
	EXPECT_EQ(timesTwo, 2);

	manager.DeactivateScript(&testNode);

	auto result = manager.Call(&testNode, "timesTwo", 1);
	EXPECT_FALSE(result.valid());
}
