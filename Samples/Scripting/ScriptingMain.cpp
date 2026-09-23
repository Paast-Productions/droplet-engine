#include <print>
#include "ScriptSystem/ScriptSystem.hpp"
#include "ScriptSystem/TestNode.hpp"

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    // Start the script system and tell it the path from your working directory to the script map
	ScriptSystem scriptSystem;
    scriptSystem.SetScriptPath("../../../src/TestScripts");

    // A scriptComponent is what talks to the script system
    TestNode scriptComponent;

    scriptSystem.CreateScript(&scriptComponent, "testScript.lua");

    scriptSystem.Start();
    scriptSystem.Update(1.0);
    TestNode guptaNode;


    return 0;
} 