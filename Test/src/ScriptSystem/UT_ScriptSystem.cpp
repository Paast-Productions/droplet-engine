#include <gtest/gtest.h>
#include <ScriptSystem/ScriptSystem.hpp>
#include <string>
#include <SceneSystem/Components/ScriptComponent.hpp>
 
 using namespace Droplet::Script;
 using namespace Droplet::Scene;

TEST(ScriptSystem, LoadScript)
{
	ScriptSystem system;
	system.SetScriptPath("../src/TestScripts");
	EXPECT_NO_THROW(system.LoadScript("testScript.lua"));
}

TEST(ScriptSystem, UnloadScript)
{
	ScriptSystem system;
	system.SetScriptPath("../src/TestScripts");
	system.LoadScript("testScript.lua");
	EXPECT_NO_THROW(system.UnloadScript("testScript.lua"));
}

TEST(ScriptSystem, CreateComponentScript)
{
	ScriptSystem system;
	system.SetScriptPath("../src/TestScripts");
	ScriptComponent component("");
	EXPECT_NO_THROW(system.CreateComponentScript(&component, "testScript.lua"));
}

TEST(ScriptSystem, ActivateComponentScript)
{
	ScriptSystem system;
	system.SetScriptPath("../src/TestScripts"); 
	ScriptComponent component("");
	
	system.CreateComponentScript(&component, "testScript3.lua");
	system.ActivateComponentScript(&component);
	
	system.Update(1);
	float result = system.Call(&component, "GetDT");

	EXPECT_EQ(result, 1);
}

TEST(ScriptSystem, DeactivateComponentScript)
{
	ScriptSystem system;
	system.SetScriptPath("../src/TestScripts");
	ScriptComponent component("");

	system.CreateComponentScript(&component, "testScript3.lua");
	system.ActivateComponentScript(&component);
	system.Update(1);
	system.DeactivateComponentScript(&component);
	system.Update(1);
	auto result = system.Call(&component, "GetDT");

	EXPECT_FALSE(result.valid());
}
