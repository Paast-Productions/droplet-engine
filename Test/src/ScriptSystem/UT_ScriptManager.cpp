#include <gtest/gtest.h>
#include <ScriptSystem/ScriptManager.hpp>
#include <ScriptSystem/LuaBindings.hpp>
#include <SceneSystem/Components/ScriptComponent.hpp>

using namespace Droplet;
using namespace Droplet::Script;

TEST(ScriptManager, SetScriptDirectory)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	ScriptManager manager(stateHandler);

	EXPECT_NO_THROW(manager.SetScriptDirectory("../src/Scripts"));
}

TEST(ScriptManager, CreateScript)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	ScriptManager manager(stateHandler);
	

	manager.SetScriptDirectory("../src/Scripts");
	Scene::ScriptComponent testComp("testScript.lua");
	EXPECT_NO_THROW(manager.CreateComponentScript(&testComp, "testScript.lua"));
	EXPECT_NO_THROW(manager.CreateComponentScript(&testComp, "testScript2.lua"));
	EXPECT_THROW(manager.CreateComponentScript(&testComp, "nonExisting.lua"), std::runtime_error);
}

TEST(ScriptManager, DetachAllInstancesToScript)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	ScriptManager manager(stateHandler);

	Scene::ScriptComponent firstComp("testScript.lua");
	Scene::ScriptComponent secondComp("testScript.lua");

	EXPECT_NO_THROW(manager.SetScriptDirectory("../src/Scripts"));

	manager.CreateComponentScript(&firstComp, "testScript.lua");
	manager.CreateComponentScript(&secondComp, "testScript.lua");

	manager.ActivateComponentScript(&firstComp);
	manager.ActivateComponentScript(&secondComp);

	manager.DetachAllInstancesToScript("testScript.lua");

	EXPECT_FALSE(manager.Call(&firstComp, "test").valid());
	EXPECT_FALSE(manager.Call(&secondComp, "test").valid());

	EXPECT_NO_THROW(manager.Update(0.016f));

	EXPECT_NO_THROW(manager.DetachAllInstancesToScript("testScript.lua"));
}

TEST(ScriptManager, LoadScript)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	ScriptManager manager(stateHandler);
	

	manager.SetScriptDirectory("../src/Scripts");

	EXPECT_NO_THROW(manager.LoadScript("testScript.lua"));
	EXPECT_NO_THROW(manager.LoadScript("testScript3.lua"));
	EXPECT_NO_THROW(manager.LoadScript("testScript2.lua"));
}

TEST(ScriptManager, UnloadScript)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	ScriptManager manager(stateHandler);
	

	manager.SetScriptDirectory("../src/Scripts");

	manager.LoadScript("testScript.lua");
	manager.LoadScript("testScript3.lua");
	manager.LoadScript("testScript2.lua");

	EXPECT_NO_THROW(manager.UnloadScript("testScript.lua"));
	EXPECT_NO_THROW(manager.UnloadScript("testScript3.lua"));
	EXPECT_NO_THROW(manager.UnloadScript("testScript2.lua"));
}

TEST(ScriptManager, IsLoaded)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	ScriptManager manager(stateHandler);
	

	manager.SetScriptDirectory("../src/Scripts");

	manager.LoadScript("testScript.lua");
	manager.LoadScript("testScript3.lua");
	manager.LoadScript("testScript2.lua");

	EXPECT_TRUE(manager.IsLoaded("testScript.lua"));
	EXPECT_TRUE(manager.IsLoaded("testScript3.lua"));
	EXPECT_TRUE(manager.IsLoaded("testScript2.lua"));

	EXPECT_FALSE(manager.IsLoaded("nonExisting"));
}

TEST(ScriptManager, ReloadScript)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	ScriptManager manager(stateHandler);
	

	manager.SetScriptDirectory("../src/Scripts");

	manager.LoadScript("testScript.lua");
	manager.LoadScript("testScript3.lua");
	manager.LoadScript("testScript2.lua");

	EXPECT_NO_THROW(manager.ReloadScript("testScript.lua"));
	EXPECT_NO_THROW(manager.ReloadScript("testScript3.lua"));
	EXPECT_NO_THROW(manager.ReloadScript("testScript2.lua"));
}

TEST(ScriptManager, GetLoadedScript)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	ScriptManager manager(stateHandler);
	

	manager.SetScriptDirectory("../src/Scripts");

	manager.LoadScript("testScript.lua");
	manager.LoadScript("testScript3.lua");
	manager.LoadScript("testScript2.lua");

	sol::load_result *result1 = manager.GetLoadedScript("testScript.lua");
	sol::load_result *result2 = manager.GetLoadedScript("testScript3.lua");
	sol::load_result *result3 = manager.GetLoadedScript("testScript2.lua");
	sol::load_result *failedResult = manager.GetLoadedScript("NonExisting.lua");
	
	EXPECT_TRUE(result1->valid());
	EXPECT_TRUE(result2->valid());
	EXPECT_TRUE(result3->valid());
	EXPECT_EQ(failedResult, nullptr);
}

TEST(ScriptManager, ActivateScript)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	ScriptManager manager(stateHandler);
	Droplet::Scene::ScriptComponent testComp("");

	manager.SetScriptDirectory("../src/Scripts");

	manager.CreateComponentScript(&testComp, "testScript3.lua");
	manager.ActivateComponentScript(&testComp);

	manager.Update(1);
	int timesTwo = manager.Call(&testComp, "TimesTwo", 1);

	EXPECT_EQ(timesTwo, 2);
}

TEST(ScriptManager, DeactivateScript)
{
	//Lua state initialization
	LuaStateHandler stateHandler;
	ScriptManager manager(stateHandler);
	Droplet::Scene::ScriptComponent testComp("");

	manager.SetScriptDirectory("../src/Scripts");

	manager.CreateComponentScript(&testComp, "testScript3.lua");
	manager.ActivateComponentScript(&testComp);

	int timesTwo = manager.Call(&testComp, "TimesTwo", 1);
	EXPECT_EQ(timesTwo, 2);

	manager.DeactivateComponentScript(&testComp);

	auto result = manager.Call(&testComp, "TimesTwo", 1);
	EXPECT_FALSE(result.valid());
}
