#include <print>
#include "ScriptSystem/ScriptSystem.hpp"
#include "ScriptSystem/TestNode.hpp"

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	ScriptSystem scriptSystem;
    scriptSystem.SetScriptPath("../../../src/TestScripts");

    scriptSystem.Start();
    scriptSystem.Update(1.0);
    TestNode guptaNode;

    scriptSystem.CreateScript(&guptaNode, "guptaTestScript.lua");
    scriptSystem.Call(&guptaNode, "backflip");
    scriptSystem.ActivateScript(&guptaNode);
    scriptSystem.Call(&guptaNode, "backflip");


    return 0;
} 