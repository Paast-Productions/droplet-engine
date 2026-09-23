#include <print>
#include "ScriptSystem/ScriptSystem.hpp"
#include "ScriptSystem/TestNode.hpp"

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	ScriptSystem scriptSystem;
    scriptSystem.SetScriptPath("../../../src/TestScripts");
	TestNode testNode;
    TestNode guptaNode;
    [[maybe_unused]] auto* script = scriptSystem.CreateScript(
        &testNode,
       "testScript.lua"
    ); 
    scriptSystem.CreateScript(&guptaNode, "guptaTestScript.lua");
    scriptSystem.Call(&guptaNode, "backflip");
    scriptSystem.ActivateScript(&guptaNode);
    scriptSystem.Call(&guptaNode, "backflip");

    while (true)
    {
        scriptSystem.Update(1.0);
        std::cout << testNode.getX() << '\n';
    }

    return 0;
} 