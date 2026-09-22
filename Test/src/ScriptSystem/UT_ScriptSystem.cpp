#include <gtest/gtest.h>
#include <ScriptSystem/ScriptSystem.hpp>
#include <string>

TEST(ScriptSystem, LoadScript)
{
	ScriptSystem system;
	EXPECT_TRUE(system.LoadScript("testScript.lua"));
}

TEST(ScriptSystem, UnloadScript)
{
	ScriptSystem system;
	system.LoadScript("testScript.lua");
	EXPECT_TRUE(system.UnloadScript("testScript.lua"));
}

TEST(ScriptSystem, ActivateScript)
{
	ScriptSystem system;
	TestNode testNode;
	system.CreateScript(&testNode, "testScript.lua");
	system.ActivateScript(&testNode);
	//Will activate the update loop for the node
}

TEST(ScriptSystem, DeactivateScript)
{
}
