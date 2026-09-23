#include <print>
#include "ScriptSystem/ScriptSystem.hpp"
#include "ScriptSystem/TestNode.hpp"

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	ScriptSystem scriptSystem;

	TestNode testNode;
    [[maybe_unused]] auto* script = scriptSystem.CreateScript(
        &testNode,
       "testScript.lua"
    ); 

    scriptSystem.ActivateScript(&testNode);
    
    scriptSystem.Start();
    scriptSystem.Update(1.0);

    while (true)
    {
        scriptSystem.Update(1.0);
        //std::cout << testNode.getX() << '\n';
    }

    return 0;
} 