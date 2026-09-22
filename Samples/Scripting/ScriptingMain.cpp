#include <print>
#include "ScriptSystem/ScriptSystem.hpp"
#include "ScriptSystem/TestNode.hpp"

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	ScriptSystem scriptSystem;

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

    while (true)
    {
        scriptSystem.Update(1.0);
        std::cout << testNode.getX() << '\n';
    }

    return 0;
} 