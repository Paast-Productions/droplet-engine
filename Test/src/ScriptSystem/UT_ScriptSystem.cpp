#include <gtest/gtest.h>
#include <ScriptSystem/ScriptSystem.hpp>

TEST(ScriptSystem, LoadScript)
{
	ScriptSystem scriptSystem;
	EXPECT_TRUE(scriptSystem.LoadScript("testScript.lua"));
}
