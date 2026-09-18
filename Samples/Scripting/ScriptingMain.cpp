#include <print>
#include "ScriptSystem/ScriptSystem.hpp"

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	TestNode testNode;
    ScriptSystem scriptSystem;
    scriptSystem.Initialize();
	if (scriptSystem.CreateScript(&testNode, "testScript.lua") == nullptr)
	{
		std::print("Failed to create script instance\n");
	}

    std::print("Hello World\n");

    return 0;
} 