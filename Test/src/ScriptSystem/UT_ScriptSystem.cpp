#include <gtest/gtest.h>
#include <ScriptSystem/ScriptSystem.hpp>
#include <string>

TEST(ScriptSystem, LoadScript)
{
	ScriptSystem system;
	system.SetScriptPath("../src/TestScripts");
	EXPECT_TRUE(system.LoadScript("testScript.lua"));
}

TEST(ScriptSystem, UnloadScript)
{
	ScriptSystem system;
	system.SetScriptPath("../src/TestScripts");
	system.LoadScript("testScript.lua");
	EXPECT_TRUE(system.UnloadScript("testScript.lua"));
}

TEST(ScriptSystem, CreateScript)
{
	ScriptSystem system;
	system.SetScriptPath("../src/TestScripts");
	TestNode testNode;

	bool result = system.CreateScript(&testNode, "testScript.lua");
	EXPECT_TRUE(result);
}

TEST(ScriptSystem, DestroyScript)
{
	ScriptSystem system;
	system.SetScriptPath("../src/TestScripts");
	TestNode testNode;

	system.CreateScript(&testNode, "testScript.lua");
	
	//Function DestroyScript should destroy instance not an entire script
	//system.DestroyScript(instance);

	EXPECT_EQ(&testNode, nullptr);
}

TEST(ScriptSystem, ActivateScript)
{
	ScriptSystem system;
	system.SetScriptPath("../src/TestScripts");
	TestNode testNode;

	system.CreateScript(&testNode, "bruh.lua");
	system.ActivateScript(&testNode);
	system.Update(1);
	float result = system.Call(&testNode, "getDT");

	EXPECT_EQ(result, 1);
}

TEST(ScriptSystem, DeactivateScript)
{
	ScriptSystem system;
	system.SetScriptPath("../src/TestScripts");
	TestNode testNode;

	system.CreateScript(&testNode, "bruh.lua");
	system.ActivateScript(&testNode);
	system.Update(1);
	system.DeactivateScript(&testNode);
	system.Update(1);
	float result = system.Call(&testNode, "getDT");

	EXPECT_EQ(result, 1);
}
