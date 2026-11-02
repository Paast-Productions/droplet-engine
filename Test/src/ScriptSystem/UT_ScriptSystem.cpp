#include <gtest/gtest.h>
#include <ScriptSystem/ScriptSystem.hpp>
#include <string>
#include <SceneSystem/Components/ScriptComponent.hpp>
 
 using namespace Droplet::Script;
 using namespace Droplet::Scene;

TEST(ScriptSystem, LoadScript)
{
	ScriptSystem system;
	system.SetScriptPath("../src/Scripts");
	EXPECT_NO_THROW(system.LoadScript("testScript.lua"));
}

TEST(ScriptSystem, UnloadScript)
{
	ScriptSystem system;
	system.SetScriptPath("../src/Scripts");
	system.LoadScript("testScript.lua");
	EXPECT_NO_THROW(system.UnloadScript("testScript.lua"));
}

TEST(ScriptSystem, CreateComponentScript)
{
	ScriptSystem system;
	system.SetScriptPath("../src/Scripts");
	ScriptComponent component;
	EXPECT_NO_THROW(system.CreateComponentScript(&component, "testScript.lua"));
}
