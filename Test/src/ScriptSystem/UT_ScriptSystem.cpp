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

TEST(ScriptSystem, DestroyScript)
{
	ScriptSystem system;
	system.SetScriptPath("../src/TestScripts");
	ScriptComponent firstComponent("");
	ScriptComponent secondComponent("");
	ScriptComponent thirdComponent("");

	system.CreateComponentScript(&firstComponent, "testScript.lua");
	system.CreateComponentScript(&secondComponent, "testScript.lua");
	system.CreateComponentScript(&thirdComponent, "testScript2.lua");

	system.ActivateComponentScript(&firstComponent);
	system.ActivateComponentScript(&secondComponent);
	system.ActivateComponentScript(&thirdComponent);

	system.DetachAllInstancesToScript("testScript.lua");

	EXPECT_FALSE(system.Call(&firstComponent, "test").valid());
	EXPECT_FALSE(system.Call(&secondComponent, "test").valid());

	EXPECT_TRUE(system.Call(&thirdComponent, "test").valid());

	EXPECT_NO_THROW(system.Update(0.016f));

	EXPECT_NO_THROW(system.DetachAllInstancesToScript("testScript.lua"));
}

TEST(ScriptSystem, ActivateScript)
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

TEST(ScriptSystem, DeactivateScript)
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
