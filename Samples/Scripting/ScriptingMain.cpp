#include <print>
#include "ScriptSystem/ScriptSystem.hpp"

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	ScriptSystem scriptSystem;

	TestNode testNode;

    [[maybe_unused]] auto* script = scriptSystem.CreateScript(
        &testNode,
       "testScript.lua"
    ); 
    
    scriptSystem.Start();

    while(true)
	{
		scriptSystem.Update(1.0f);
	}

    std::cout << testNode.getX() << '\n';

    return 0;
} 