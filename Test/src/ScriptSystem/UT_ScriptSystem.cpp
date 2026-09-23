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

	ScriptInstance* instance = system.CreateScript(&testNode, "testScript.lua");
	EXPECT_NE(instance, nullptr);
}

TEST(ScriptSystem, DestroyScript)
{
	ScriptSystem system;
	system.SetScriptPath("../src/TestScripts");
	TestNode testNode;

	ScriptInstance* instance = system.CreateScript(&testNode, "testScript.lua");
	system.DestroyScript("testScript.lua");

	EXPECT_EQ(instance, nullptr);
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
	auto result = system.Call(&testNode, "getDT");

	EXPECT_FALSE(result.valid());
}
