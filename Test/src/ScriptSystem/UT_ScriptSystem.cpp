#include <gtest/gtest.h>
#include <ScriptSystem/ScriptSystem.hpp>
#include <string>

TEST(ScriptSystem, LoadScript)
{
	ScriptSystem scriptSystem;
	EXPECT_TRUE(scriptSystem.LoadScript("testScript.lua"));
}

TEST(ScriptSystem, UnloadScript)
{
	ScriptSystem scriptSystem;
	scriptSystem.LoadScript("testScript.lua");
	EXPECT_TRUE(scriptSystem.UnloadScript("testScript.lua"));
}

TEST(ScriptSystem, ActivateScript)
{
}

TEST(ScriptSystem, DeactivateScript)
{
}
