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

TEST(ScriptSystem, CreateScript)
{
	ScriptSystem system;
	system.SetScriptPath("../src/TestScripts");
	ScriptComponent component("");
	EXPECT_NO_THROW(system.CreateScript(&component, "testScript.lua"));
}

TEST(ScriptSystem, DestroyScript)
{
	ScriptSystem system;
	system.SetScriptPath("../src/TestScripts");
	ScriptComponent firstComponent("");
	ScriptComponent secondComponent("");
	ScriptComponent thirdComponent("");

	system.CreateScript(&firstComponent, "testScript.lua");
	system.CreateScript(&secondComponent, "testScript.lua");
	system.CreateScript(&thirdComponent, "testScript2.lua");

	system.ActivateScript(&firstComponent);
	system.ActivateScript(&secondComponent);
	system.ActivateScript(&thirdComponent);

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
	
	system.CreateScript(&component, "testScript3.lua");
	system.ActivateScript(&component);
	
	system.Update(1);
	float result = system.Call(&component, "GetDT");

	EXPECT_EQ(result, 1);
}

TEST(ScriptSystem, DeactivateScript)
{
	ScriptSystem system;
	system.SetScriptPath("../src/TestScripts");
	ScriptComponent component("");

	system.CreateScript(&component, "testScript3.lua");
	system.ActivateScript(&component);
	system.Update(1);
	system.DeactivateScript(&component);
	system.Update(1);
	auto result = system.Call(&component, "GetDT");

	EXPECT_FALSE(result.valid());
}
