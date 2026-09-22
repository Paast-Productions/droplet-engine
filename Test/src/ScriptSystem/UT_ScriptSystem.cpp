#include <gtest/gtest.h>
#include <ScriptSystem/ScriptSystem.hpp>
#include <string>

TEST(ScriptSystem, LoadScript)
{
	std::string scriptPath = "testScript.lua";
	ScriptSystem scriptSystem;
	EXPECT_TRUE(scriptSystem.LoadScript(scriptPath));
}

TEST(ScriptSystem, UnloadScript)
{
	std::string scriptPath = "testScript.lua";
	ScriptSystem scriptSystem;
	scriptSystem.LoadScript(scriptPath);
	EXPECT_TRUE(scriptSystem.UnloadScript(scriptPath));
}

TEST(ScriptSystem, CreateScript_not_yet_made)
{
	//Ändra på testnode så att den är en component
	//Skapa ett skript och kolla om ScriptInstance är okej.
	EXPECT_TRUE(false);
}

TEST(ScriptSystem, DestroyScript_not_yet_made)
{
	//Kan endast implementeras efter att CreateScript är klar
	//Skapa en scriptinstance med hjälp av CreateScript
	//Se till att ha koll på ScriptInstance
	//Kör DestroyScript och kolla om ScriptInstance har försvunnit
	EXPECT_TRUE(false);
}
