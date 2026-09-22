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
    
    scriptSystem.Start();
    scriptSystem.Call(&guptaNode, "backflip");
    auto var = scriptSystem.Call(&guptaNode, "fishflip", 5, 2,3);

    scriptSystem.ActivateScript(&testNode);
    for (int i = 0; i < 3; ++i)
    {
        scriptSystem.Update(1.0f);
    }
    scriptSystem.DeactivateScript(&testNode);
    std::cout << testNode.getX() << '\n';
    std::cout << guptaNode.getX() << '\n';

    scriptSystem.DeactivateScript(&testNode);
    std::cout << testNode.getX() << '\n';
    return 0;
} 