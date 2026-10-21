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
	Scene::ScriptComponent testComp;
	EXPECT_NO_THROW(manager.CreateComponentScript(&testComp, "testScript.lua"));
	EXPECT_NO_THROW(manager.CreateComponentScript(&testComp, "testScript2.lua"));
	EXPECT_THROW(manager.CreateComponentScript(&testComp, "nonExisting.lua"), std::runtime_error);
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

TEST(ScriptManager, GetLoadedScriptInfo)
{
	LuaStateHandler stateHandler;
	ScriptManager manager(stateHandler);

	manager.SetScriptDirectory("../src/Scripts");

	manager.LoadScript("testScript.lua");
	manager.LoadScript("testScript3.lua");
	manager.LoadScript("testScript2.lua");

	LoadedScript *result1 = manager.GetLoadedScriptInfo("testScript.lua");
	LoadedScript *result2 = manager.GetLoadedScriptInfo("testScript3.lua");
	LoadedScript *result3 =	manager.GetLoadedScriptInfo("testScript2.lua");
	LoadedScript *failedResult = manager.GetLoadedScriptInfo("NonExisting.lua");

	ASSERT_NE(result1, nullptr);
	ASSERT_NE(result2, nullptr);
	ASSERT_NE(result3, nullptr);

	EXPECT_FALSE(result1->scriptPath.empty());
	EXPECT_FALSE(result2->scriptPath.empty());
	EXPECT_FALSE(result3->scriptPath.empty());

	EXPECT_EQ(failedResult, nullptr);
}

